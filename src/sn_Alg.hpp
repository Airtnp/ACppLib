#ifndef SN_ALG_H
#define SN_ALG_H

#include "sn_CommonHeader.h"
#include "sn_DS.hpp"

/*
TODO: (easy)
dij/a* ±³°üdp prim kruskal floyd
*/

namespace sn_Alg {
	using namespace std;
	
	namespace misc {
		//ref: https://en.wikipedia.org/wiki/Fast_inverse_square_root
		float q_rsqrt(float x) {
			float x2 = x * 0.5F;
			int i = *(int*) & x;
			i = 0x5f3759df - (i >> 1);    
			x = *(float*) & i;
			x = x * (1.5F - (x2 * x * x)); 
			return x;
		}

		unsigned long long get_clock_cycle() {
			unsigned long long ret;
#if defined (__GNUC__)
			__asm__ __volatile__("rdtsc\n\t": "=A" (ret) : );
#else
			ret = __rdtsc();
			/*For another option : untested, better specify bits
			unsigned long ret_p;
			unsigned long ret_l;
			__asm {
				rdtsc;
				out edx, ret_p
				out eax, ret_l
			}
			ret = ret_p << 32 + ret_l
			*/
#endif
			return ret;
		}

		inline int inc_mod(int x, int v, int m) {
			x += v;
			return x >= m ? x - m : x;
		}

		inline int dec_mod(int x, int v, int m) {
			x -= v;
			return x >= m ? x - m : x;
		}

		inline void read_int(int& x) {
			//or use fread(input, 1 << 31, stdin)
			char c = getchar(); x = 0;
			while (c < '0' || c > '9')
				c = getchar();
			while (c <= '9' && c >= '0') {
				x = x * 10 + c - 48;
				c = getchar();
			}
		}

		inline void write_int(int x) {
			//or use fwrite(output, 1, strlen(output), stdout)
			int cnt = 0;
			char c[15];
			while (x) {
				c[++cnt] = (x % 10) + 48;
				x /= 10;
			}
			while (cnt)
				putchar(c[cnt--]);
			putchar('\n');
		}

	}

	namespace binary {

		//For some other algs, ref: http://graphics.stanford.edu/~seander/bithacks.html
		//For linux bitops, ref: https://code.woboq.org/linux/linux/arch/x86/include/asm/bitops.h.html
		inline int lowbit(const int& x) {
			return x & (-x);  //or ((x-1) xor x) and x)
		}

		inline int remove_lowbit(const int& x) {
			return x & (x - 1);  //or x ^= lowbit(x)
		}

		//__builtin_cnz
		inline int count_zeros(int x) {
			int count;
			while (x) {
				count++;
				x ^= lowbit(x);
			}
			return count;
		}

		//For detail, see http://www.matrix67.com/blog/archives/264 
		//divide and conquer 11001100 -> 11001100 -> 00110011 -> 00001100
		//For __builtin_popcount, get popcount table, see http://www.xuebuyuan.com/828691.html
		inline unsigned long long count_bits(unsigned long long x) {
			x = (x & 0x5555555555555555LL) + ((x & 0xAAAAAAAAAAAAAAAALL) >> 1);
			x = (x & 0x3333333333333333LL) + ((x & 0xCCCCCCCCCCCCCCCCLL) >> 2);
			x = (x & 0x0F0F0F0F0F0F0F0FLL) + ((x & 0xF0F0F0F0F0F0F0F0LL) >> 4);
			x = (x & 0x00FF00FF00FF00FFLL) + ((x & 0xFF00FF00FF00FF00LL) >> 8);
			x = (x & 0x0000FFFF0000FFFFLL) + ((x & 0xFFFF0000FFFF0000LL) >> 16);
			x = (x & 0x00000000FFFFFFFFLL) + ((x & 0xFFFFFFFF00000000LL) >> 32);
			return x;
		}

		//divide cand conquer 11001100 -> 11001100 -> 00110011 -> 00110011
		//For table-lookup, see http://stackoverflow.com/questions/2602823/in-c-c-whats-the-simplest-way-to-reverse-the-order-of-bits-in-a-byte
		inline long long reverse_bits(unsigned long long x) {
			x = ( (x >> 1)  & 0x5555555555555555LL) + ( (x << 1)  & 0xAAAAAAAAAAAAAAAALL);
			x = ( (x >> 2)  & 0x3333333333333333LL) + ( (x << 2)  & 0xCCCCCCCCCCCCCCCCLL);
			x = ( (x >> 4)  & 0x0F0F0F0F0F0F0F0FLL) + ( (x << 4)  & 0xF0F0F0F0F0F0F0F0LL);
			x = ( (x >> 8)  & 0x00FF00FF00FF00FFLL) + ( (x << 8)  & 0xFF00FF00FF00FF00LL);
			x = ( (x >> 16) & 0x0000FFFF0000FFFFLL) + ( (x << 16) & 0xFFFF0000FFFF0000LL);
			x = ( (x >> 32) & 0x00000000FFFFFFFFLL) + ( (x << 32) & 0xFFFFFFFF00000000LL);
			return x;
		}

	}

	namespace graph {

		namespace connectivity {

			//from <<algorithm in C>> Ch.1
			vector<int> weighted_quick_union(const vector<pair<int, int>>& edges, int vertex_num) {
				vector<int> res;
				vector<int> sz;
				res.reserve(vertex_num);
				sz.reserve(vertex_num);
				for (int i = 0; i < vertex_num; ++i) {
					res.push_back(i);
					sz.push_back(1);
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
				res.reserve(vertex_num);
				sz.reserve(vertex_num);
				for (int i = 0; i < vertex_num; ++i) {
					res.push_back(i);
					sz.push_back(1);
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
			using sn_DS::basic::graph;
			using sn_DS::basic::graph_node;
			using sn_DS::basic::bitvec;

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

	namespace search {

		template <typename C, typename T>
		int bisearch(C sorted_arr, T value, int l, int r) {
			while (r >= l) {
				int mid = (l + r) / 2;
				if (value == sorted_arr[mid])
					return mid;
				if (value < sorted_arr[mid])
					r = mid - 1;
				else
					l = mid + 1;
			}
			return -1;
		}

		//T support .next(), return loop length
		template <typename T>
		int floyd_judge_loop(const T& head_node, const T& end_node) {
			auto p1 = head_node;
			auto p2 = head_node;
			bool has_loop = false;
			while (p1 != end_node && p2 != end_node) {
				p1 = p1.next();
				p2 = p2.next().next();
				if (p1 == p2) { //The meeting node is the loop's first node, since the exceeded length L, loop length C, C | L
					has_loop = true;
					break;
				}
			}
			if (!has_loop)
				return -1;

			int loop_length = 0;
			do {
				loop_length++;
				p1 = p1.next();
			} while (p1 != p2);

			return loop_length;
		}

		//Max subarray sum
		int kadane(int arr[], int n) {
			int max_so_far = 0;
			int max_ending_here = 0;
			for (int i = 0; i < n; ++i) {
				max_ending_here = max_ending_here + arr[i];
				max_ending_here = std::max(max_ending_here, 0);
				max_so_far = std::max(max_so_far, max_ending_here);
			}
			return max_so_far;
		}

		//Max subarray product
		int kadane_product(int arr[], int n) {
			int max_so_far = 0;
			int max_ending_here = 0;
			int min_ending_here = 0;
			for (int i = 0; i < n; ++i) {
				int temp = max_ending_here;
				max_ending_here = std::max(arr[i], std::max(arr[i] * max_ending_here, arr[i]*min_ending_here));
				min_ending_here = std::min(arr[i], std::max(arr[i] * temp, arr[i] * min_ending_here));
				max_so_far = std::max(max_so_far, max_ending_here);
			}
			return max_so_far;
		}


	}

	namespace sort {

	}

	namespace dynamic_programming {
		namespace knapsack {
			//if T support </> we can apply to n-dimension
			template <typename T>
			T recursive_1d_knapsack(T capacity, const vector<T>& size, const vector<T>& value) {
				int sz = value.size();
				unordered_map<T, T> mt{};
				return recursive_1d_knapsack_helper(capacity, 0, size, value, mt);
			}

			template<typename T>
			T recursive_1d_knapsack_helper(T capacity, int index, const vector<T>& size, const vector<T>& value, unordered_map<T, T>& mt) {
				auto p = mt.find(capacity);
				if (p != mt.end())
					return p->second;
				if (capacity < 0)
					return -item[0];
				if (index == value.size())
					return 0;
				res1 = recursive_1d_knapsack_helper(capacity, index + 1, size, value, mt);
				res2 = recursive_1d_knapsack_helper(capacity - size[index], index + 1, size, value, mt) + value[index];
				T res = res1 > res2 ? res1 : res2;
				mt.insert({ capacity, res });
				return res;
			}

		}
	}

	namespace number_theory {

		namespace prime {

			constexpr size_t PRIME_LIMIT = 10000;

			bool is_prime(unsigned int n) {
				for (size_t i = 2; i < floor(sqrt(n)); ++i) {
					if (i % n)
						return false;
				}
				return true;
			}

			vector<unsigned int> linear_prime_sieve(unsigned int n) {
				bitset<PRIME_LIMIT> sieve_arr(0);
				vector<unsigned int> prime;
				for (size_t i = 2; i < n; ++i) {
					if (!sieve_arr[i])
						prime.push_back(i);
					for (size_t j = 0; i * prime[j] < n; ++j) {
						sieve_arr[i * prime[j]] = 1;
						if (!(i % prime[j]))
							break;
					}
				}
				return prime;
			}

			/*
			for detail: https://www.zhihu.com/question/29580448
						http://cstheory.stackexchange.com/questions/5578/can-merlin-convince-arthur-about-a-certain-sum
			*/
			unsigned int fast_prime_sum(unsigned int n) {
				unsigned int sqrt_n = floor(sqrt(n));
				unsigned int arr_index[PRIME_LIMIT * 10];  //contain the value of n/1, n/2, ... n / sqrt(n) , 1, ..., sqrt(n)
				unsigned int sum_prime[PRIME_LIMIT]; //contain the sum of first n primes
				for (unsigned int k = 0; k < sqrt_n + 1; ++k) {
					arr_index[k] = k;
					sum_prime[k] = (k * (k + 1)) / 2 - 1;
				}
				for (unsigned int k = sqrt_n + 1; k < 2 * sqrt_n + 1; ++k) {
					arr_index[k] = n / (2 * sqrt_n + 1 - k);
					sum_prime[k] = (arr_index[k] * (arr_index[k] + 1)) / 2 - 1;
				}
				for (unsigned int p = 2; p < sqrt_n + 1; ++p) {
					if (sum_prime[p] > sum_prime[p - 1]) {  //condition: p is a prime
						unsigned int sum_pre = sum_prime[p - 1];
						unsigned int square_p = p * p;
						for (unsigned int q = 2 * sqrt_n; q > 1; --q) {
							if (arr_index[q] < square_p)
								break;
							unsigned int a = arr_index[q];
							unsigned int b = a / p;
							a = a > sqrt_n ? 2 * sqrt_n + 1 - n / a : a; //map arr_index to indices of sum_prime
							b = b > sqrt_n ? 2 * sqrt_n + 1 - n / b : b;
							sum_prime[a] -= p * (sum_prime[b] - sum_pre);
						}
					}
				}
				return sum_prime[2 * sqrt_n];
			}

		}

		namespace gcd {
			int gcd(int a, int b) {
				return b ? gcd(b, a % b) : a;
			}

			//ref: https://code.woboq.org/linux/linux/lib/gcd.c.html
			unsigned long linux_gcd(unsigned long a, unsigned long b) {
#if defined(AVAILABLE_FFS)
				unsigned long r = a | b;
				if (!a || !b)
					return r;
				b >>= __ffs(b);
				if (b == 1)
					return r & -r;
				for (;;) {
					a >>= __ffs(a);
					if (a == 1)
						return r & -r;
					if (a == b)
						return a << __ffs(r);
					if (a < b)
						swap(a, b);
					a -= b;
				}
#else
				unsigned long r = a | b;
				if (!a || !b)
					return r;

				//get lowest bit of r
				r = sn_Alg::binary::lowbit(r);

				while (!(b & r))
					b >>= 1;
				if (b == r)
					return r;

				for (;;) {
					while (!(a & r))
						a >>= 1;
					if (a == r)
						return r;
					if (a == b)
						return a;
					if (a < b)
						swap(a, b);
					a -= b;
					a >>= 1;
					if (a & r)
						a += b;
					a >>= 1;
				}
#endif
			}


			//return gcd(a, b) ax + by = gcd(a, b)
			int exgcd(int a, int b, int&x, int& y) {
				if (!b) {
					x = 1;
					y = 0;
					return a;
				}
				int r = exgcd(b, a % b, x, y);
				int t = x;
				x = y;
				y = t - a / b * y;
				return r;
			}

			//ax \\equiv b (mod n)
			int solve_module_equation(int a, int b, int n) {
				int x = 0, y = 0;
				int d = exgcd(a, n, x, y);
				if (b % d == 0) {
					return x * b / d;
				}
				return -1;
			}

			//ax \\equiv 1 (mod n)
			int solve_reverse_element(int a, int n) {
				return solve_module_equation(a, 1, n);
			}

		}

	}

}


#endif