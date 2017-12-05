#ifndef SN_ALGDS_GRAPHICS_H
#define SN_ALGDS_GRAPHICS_H

namespace sn_AlgDS {
    template <typename T>
    struct Point2D {
        T x;
        T y;
        uint8_t label;
        Point2D() : x{0}, y{0}, label{0} {}
        Point2D(T x_, T y_, uint8_t, label_) : x{x_}, y{y_}, label{label_} {}
        Point2D(const Point2D<T>& rhs) : x(rhs.x), y(rhs.y), label{rhs.label} {}
        double xy2() {
            return (double)x * x + (double)y * y;
        }
        double dist2(const Point2D<T>& rhs) {
            double dx = x - rhs.x;
            double dy = y - rhs.y;
            return dx * dx + dy * dy;
        }
        double dist(const Point2D<T>& rhs) {
            return sqrt(dist2(rhs));
        }
        bool operator==(const Point2D<T>& rhs) {
            return (label == rhs.label_) && (x == rhs.x) && (y == rhs.y);
        }
        double operator-(const Point2D<T>& rhs) {
            return dist(rhs);
        }
        double operator<(const Point2D<T>& rhs) {
            if (x != rhs.x) {
                return x < rhs.x;
            }
            return y < rhs.y;
        }
    };

    template <typename T>
    struct Edge2D {
        bool is_bad = false;
        Point2D<T> p1;
        Point2D<T> p2;
        Edge2D(const Point2D<T>& p1_, const Point2D<T>& p2_) : p1{p1_}, p2{p2_} {}
        Edge2D(const Edge2D<T>& rhs) : p1{rhs.p1}, p2{rhs.p2} {}
        bool operator==(const Edge2D<T>& rhs) {
            return (p1 == rhs.p1 && p2 == rhs.p2) || (p1 == rhs.p2 && p2 == rhs.p1);
        }
        bool operator<(const Edge2D<T>& rhs) {
            if (p1 != rhs.p1) {
                return p1 < rhs.p1;
            }
            return p2 < rhs.p2;
        }
    };

    template <typename T>
    struct Triangle {
        bool is_bad = false;
        Point2D<T> p1;
        Point2D<T> p2;
        Point2D<T> p3;
        bool has_point(const Point2D<T>& v) {
            return p1 == v || p2 == v || p3 == v;
        }
        bool circum_circle_has_point(const Point2D<T>& v) {
            double ab = p1.xy2();
            double cd = p2.xy2();
            double ef = p3.xy2();
            
            double circum_x = (ab * (p3.y - p2.y) + cd * (p1.y - p3.y) + ef * (p2.y - p1.y)) / (p1.x * (p3.y - p2.y) + p2.x * (p1.y - p3.y) + p3.x * (p2.y - p1.y)) / 2.0;
			double circum_y = (ab * (p3.x - p2.x) + cd * (p1.x - p3.x) + ef * (p2.x - p1.x)) / (p1.y * (p3.x - p2.x) + p2.y * (p1.x - p3.x) + p3.y * (p2.x - p1.x)) / 2.0;
			double circum_radius = ((p1.x - circum_x) * (p1.x - circum_x)) + ((p1.y - circum_y) * (p1.y - circum_y));

			double dist = ((v.x - circum_x) * (v.x - circum_x)) + ((v.y - circum_y) * (v.y - circum_y));
			return dist <= circum_radius;
        }
        bool operator==(const Triangle<T> &rhs) {
            return	(p1 == rhs.p1 || p1 == rhs.p2 || p1 == rhs.p3) &&
                    (p2 == rhs.p1 || p2 == rhs.p2 || p2 == rhs.p3) && 
                    (p3 == rhs.p1 || p3 == rhs.p2 || p3 == rhs.p3);
        }
    };

    template <typename T>
    struct RefEdge2D {
        uint32_t p1;
        uint32_t p2;
    };

    template <typename T>
    void solve_delaunay(vector<Point2D<T>>& pts, T minX, T minY, T maxX, T maxY) {
        vector<uint8_t> spts;
        vector<vector<bool>> valid_edge;
        for (size_t i = 0; i < pts.size(); ++i) {
            spts[i] = i;
        }
        vector<Triangle<T>> tris;
        std::sort(spts.begin(), spts.end(), [&](const uint32_t a, const uint32_t b){
            return pts[a] < pts[b];
        });
        T dX = maxX - minX;
        T dY = maxY - minY;
        T dmax = std::max(dX, dY);
        double midX = (maxX + minX) / 2;
        double midY = (maxY + minY) / 2;
        Point2D<T> p1(midX - 20 * dmax, midY - dmax, 51);
        Point2D<T> p2(midX, midY + 20 * dmax, 52);
        Point2D<T> p3(midX + 20 * dmax, midY - dmax, 53);
        tris.push_back(Triangle<T>{p1, p2, p3});
        for (size_t i = 0; i < spts.size(); ++i) {
            set<RefEdge2D> polygon;
            bool on_vertex = false;
            for (size_t j = 0; j < tris.size(); ++i) {
                if (tris[j].circum_circle_has_point(pts[spts[i])]) {
                    tris[j].is_bad = true;
                    polygon.push_back(Edge2D{tris[j].p1.label, tris[j].p2});
                    polygon.push_back(Edge2D{tris[j].p2.label, tris[j].p3});
                    polygon.push_back(Edge2D{tris[j].p3.label, tris[j].p1});
                }
            }
            tris.erase(
                remove_if(tris.begin(), tris.end(), [](const Triangle<T>& t){
                    return t.is_bad;
                }),
                tris.end()
            );
            for (auto& eg : polygon) {
                tris.push_back(Triangle<T>{pts[eg.p1], eg.p2, pts[spts[i]]});
            }
        }
    }
}


#endif