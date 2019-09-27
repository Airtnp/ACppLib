#ifndef SN_TEST_ALG_H
#define SN_TEST_ALG_H

#include "sn_CommonHeader_test.h"
#include "../src/sn_Alg.hpp"


namespace sn_Alg_test {
	using namespace std;

	template <typename T>
	std::ostream& operator<<(std::ostream& out, const vector<T> vec) {
		for (const auto v : vec) {
			out << v;
			out << " ";
		}
		out << endl;
		return out;
	}

	void graph_test() {
		std::vector<std::pair<int, int>> test_edges = {
			make_pair(3, 4),
			make_pair(4, 9),
			make_pair(8, 0),
			make_pair(2, 3),
			make_pair(5, 6),
			make_pair(2, 9),
			make_pair(5, 9),
			make_pair(7, 3),
			make_pair(4, 8),
			make_pair(5, 6),
			make_pair(0, 2),
			make_pair(6, 1)
		};
		cout << graph::connectivity::weighted_quick_union(test_edges, 10);

	}

	void sn_alg_test() {
		//graph_test();
		//cout << sn_Alg::number_theory::prime::linear_prime_sieve(100);
		
	}
	
	void gcd_alg_compare() {
		const size_t s = 100000;
		vector<int> a{};
		a.reserve(s);
		vector<int> b{};
		b.reserve(s);
		vector<int> vc1{};
		vc1.reserve(s);
		vector<int> vc2{};
		vc2.reserve(s);
		clock_t t1, t2, t3;
		unsigned long long c1, c2, c3;
		for (int i = 0; i < s; ++i) {
			a.push_back(rand());
			b.push_back(rand());
		}
		c1 = misc::get_clock_cycle();
		t1 = clock();
		for (int i = 0; i < s; ++i) {
			vc1.push_back(number::gcd::gcd(a[i], b[i]));
		}
		c2 = misc::get_clock_cycle();
		t2 = clock();
		for (int i = 0; i < s; ++i) {
			vc2.push_back(number::gcd::gcd(a[i], b[i]));
		}
		c3 = misc::get_clock_cycle();
		t3 = clock();
		cout << c2 - c1 << endl;
		cout << c3 - c2 << endl;
		cout << t2 - t1 << endl;
		cout << t3 - t2 << endl;
	}
}
#endif