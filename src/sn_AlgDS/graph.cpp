#ifndef SN_ALGDS_GRAPH_H
#define SN_ALGDS_GRAPH_H

#include <bits/stdc++.h>
#include "basic_ds.cpp"
using namespace std;

namespace graph {
    
    namespace connectivity {

        //from <<algorithm in C>> Ch.1
        vector<int> weighted_quick_union(const vector<pair<int, int>>& edges, int vertex_num) {
            vector<int> res;
            vector<int> sz;
            res.resize(vertex_num);
            sz.resize(vertex_num);
            for (int i = 0; i < vertex_num; ++i) {
                res[i] = i;
                sz[i] = 1;
            }
            for (const auto& x : edges) {
                int v1 = x.first;
                int v2 = x.second;
                int p1, p2;
                for (p1 = v1; p1 != res[p1]; p1 = res[p1]);
                for (p2 = v2; p2 != res[p2]; p2 = res[p1]);
                if (p1 == p2)
                    continue;
                if (sz[p1] < sz[p2]) {
                    res[p1] = p2;
                    sz[p2] += sz[p1];
                }
                else {
                    res[p2] = p1;
                    sz[p1] += sz[p2];
                }
            }
            return res;
        }

        vector<int> path_compression_union(const vector<pair<int, int>>& edges, int vertex_num) {
            vector<int> res;
            vector<int> sz;
            res.resize(vertex_num);
            sz.resize(vertex_num);
            for (int i = 0; i < vertex_num; ++i) {
                res[i] = i;
                sz[i] = 1;
            }
            for (const auto& x : edges) {
                int v1 = x.first;
                int v2 = x.second;
                int p1, p2;
                for (p1 = v1; p1 != res[p1]; p1 = res[p1])
                    res[p1] = res[res[p1]];
                for (p2 = v2; p2 != res[p2]; p2 = res[p1])
                    res[p2] = res[res[p2]];
                if (p1 == p2)
                    continue;
                if (sz[p1] < sz[p2]) {
                    res[p1] = p2;
                    sz[p2] += sz[p1];
                }
                else {
                    res[p2] = p1;
                    sz[p1] += sz[p2];
                }
            }
            return res;
        }
    }

    namespace search {
        using basic::graph;
        using basic::graph_node;
        using basic::bitvec;

        //connected graph
        template <typename T, typename E>
        vector<graph_node<T>> graph_dfs(const graph<T, E>& gh) {
            vector<graph_node<T>> v_dfs;
            bitvec visited(gh.size);
            graph_dfs_helper(gh, 0, visited, v_dfs);
            return v_dfs;
        }

        template <typename T, typename E>
        void graph_dfs_helper(const graph<T, E>& gh, int index, bitvec& visited, vector<graph_node<T>> v_dfs) {
            if (visited[index])
                return;
            visited[index] = true;
            auto v_adj = gh.get_adj(index);
            for (const auto& v : v_adj) {
                visited[v.first] = true;
                v_dfs.push_back(v);
                graph_dfs_helper(gh, v.first, visited, v_dfs);
            }
        }

        using sn_DS::basic::queue;

        template <typename T, typename E>
        vector<graph_node<T>> graph_bfs(const graph<T, E>& gh) {
            vector<graph_node<T>> v_bfs;
            bitvec visited(gh.size);
            queue<graph_node<T>> q_bfs(gh.size);
            q_bfs.push(gh[0][0]);
            do {
                auto front = q_bfs.front();
                q_bfs.pop();
                if (!visited[front.first]) {
                    visited[front.first] = true;
                    v_bfs.push_back(front);
                    auto v_adj = gh.get_adj(front.first);
                    for (const auto& v : v_adj) {
                        if (!visited[v.first])
                            q_bfs.push(v);
                    }

                }
            } while (!q_bfs.empty());
            
            return v_bfs;
        }

    }

    namespace distance {
        using sn_DS::basic::graph;
        using sn_DS::basic::graph_node;
        using sn_DS::basic::bitvec;
        
        template <typename E>
        vector<int> dijkstra(const graph<int, E>& gh, const int& start_index) {
            vector<int> v_dis(gh.size, INT_MAX);
            priority_queue<int, vector<int>, [&v_dis](int a, int b) { return v_dis[a] < v_dis[b]; }> p;
            bitvec visit(gh.size);
            p.push(start_index);
            v_dis[start_index] = 0;
            while (!p.empty()) {
                auto u = p.top();
                p.pop();
                visit[u] = 0;
                auto u_adj = gh.get_adj(u);
                for (const auto& v : u_adj) {
                    auto weight = v.second;
                    auto index = v.first;
                    if (v_dis[index] > v_dis[u] + weight) {
                        v_dis[index] = v_dis[u] + weight;
                        if (!visit[index]) {
                            visit[v] = 1;
                            p.push(v);
                        }
                    }
                }
            }
            return v_dis;
        }

        //Bellman-Ford in queue-optim, high in sparse graph better return optional
        template <typename E>
        vector<int> bf_min_distance(const graph<int, E>& gh, const int& start_index) {
            vector<int> v_dis(gh.size, INT_MAX);
            v_dis[start_index] = 0;
            queue<int> q_dis;
            bitvec in_queue(gh.size);
            vector<int> in_queue_sum(0);

            q_dis.push(start_index);
            in_queue[start_index] = 1;
            in_queue_sum[start_index] += 1;
            while (!q_dis.empty()) {
                auto front = q_dis.front();
                q_dis.pop();
                in_queue[front] = 0;
                auto f_adj = gh.get_adj(front);
                for (const auto& v : f_adj) {
                    auto weight = v.second;
                    auto index = v.first;
                    if (weight + v_dis[front] < v_dis[index]) {
                        v_dis[index] = weight + v_dis[front];
                        if (!in_queue[index]) {
                            q_dis.push(index);
                            in_queue[index] = 1;
                            in_queue_sum[index] += 1;
                            if (in_queue_sum[index] > gh.size)
                                return NULL;
                        }
                    }
                    if (v_dis[start_index] < 0)
                        return NULL;
                }
            }
            return v_dis;

        }

        template <typename T, typename E>
        vector<vector<T>> floyd(const graph<T, E>& gh, T INF = 65536) {
            std::size_t n = gh.size;
            std::vector<std::vector<T>> dis(n, std::vector<T>(n, INF));
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < n; ++j)
                    for (int k = 0; k < n; ++k)
                        dis[i][j] = std::min(dis[i][j], dis[i][k] + dis[k][j]);
            return dis;
        }


    }

}


#endif