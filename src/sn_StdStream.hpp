#ifndef SN_STDSTREAM_H
#define SN_STDSTREAM_H

#include "sn_CommonHeader.h"

namespace sn_StdStream {
	namespace manipulator {
		template <class CharT, class Traits>
		std::basic_ostream<CharT, Traits>& nl(std::basic_ostream<CharT, Traits>& os) {
			os.put('\n');
			return os;
		}
	}

	namespace stlio {
		//ref : http://stackoverflow.com/a/6245777/273767
		template<class Ch, class Tr, class Tuple, std::size_t... Is>
		void print_tuple_impl(std::basic_ostream<Ch, Tr>& os,
			const Tuple & t,
			std::index_sequence<Is...>)
		{
			using swallow = int[]; // guarantees left to right order
			(void)swallow {
				0, (void(os << (Is == 0 ? "" : ", ") << std::get<Is>(t)), 0)...
			};
		}

		template<class Ch, class Tr, class... Args>
		decltype(auto) operator<<(std::basic_ostream<Ch, Tr>& os,
			const std::tuple<Args...>& t)
		{
			os << "(";
			print_tuple_impl(os, t, std::index_sequence_for<Args...>{}); //can use fold expression easily, ref: http://en.cppreference.com/w/cpp/language/fold
			return os << ")";
		}

		template <typename Ch, typename Tr, typename T, typename U>
		std::basic_ostream<Ch, Tr>& operator<<(std::basic_ostream<Ch, Tr>& out, const std::pair<T, U>& p) {
			return out << '(' << p.first << ", " << p.second << ')';
		}

		template <typename Ch, typename Tr, typename T, std::size_t N>
		std::basic_ostream<Ch, Tr>& operator<<(std::basic_ostream<Ch, Tr>& out, const std::array<T, N>& v) {
			out << '[';
			for (auto i = 0; i < N; ++i) {
				out << v[i];
				auto j = i;
				if (++j != N)
					out << ',';
			}
			return out << ']';
		}


		template <typename Ch, typename Tr, typename T>
		std::basic_ostream<Ch, Tr>& operator<<(std::basic_ostream<Ch, Tr>& out, const std::vector<T>& v) {
			out << '[';
			for (auto i = v.begin(); i != v.end(); ++i) {
				out << *i;
				auto j = i;
				if (++j != v.end())
					out << ',';
			}
			return out << ']';
		}

		template <typename Ch, typename Tr, typename T>
		std::basic_ostream<Ch, Tr>& operator<<(std::basic_ostream<Ch, Tr>& out, const std::set<T>& v) {
			out << '[';
			for (auto i = v.begin(); i != v.end(); ++i) {
				out << *i;
				auto j = i;
				if (++j != v.end())
					out << ',';
			}
			return out << ']';
		}

		template <typename Ch, typename Tr, typename T>
		std::basic_ostream<Ch, Tr>& operator<<(std::basic_ostream<Ch, Tr>& out, const std::unordered_set<T>& v) {
			out << '[';
			for (auto i = v.begin(); i != v.end(); ++i) {
				out << *i;
				auto j = i;
				if (++j != v.end())
					out << ',';
			}
			return out << ']';
		}

		template <typename Ch, typename Tr, typename T, typename U>
		std::basic_ostream<Ch, Tr>& operator<<(std::basic_ostream<Ch, Tr>& out, const std::map<T, U>& m) {
			out << '{';
			for (auto i = m.begin(); i != m.end(); ++i) {
				out << i->first << " = " << i->second;
				auto j = i;
				if (++j != m.end())
					out << ',';
			}
			return out << '}';
		}

		template <typename Ch, typename Tr, typename T, typename U>
		std::basic_ostream<Ch, Tr>& operator<<(std::basic_ostream<Ch, Tr>& out, const std::unordered_map<T, U>& m) {
			out << '{';
			for (auto i = m.begin(); i != m.end(); ++i) {
				out << i->first << " = " << i->second;
				auto j = i;
				if (++j != m.end())
					out << ',';
			}
			return out << '}';
		}

		template <typename Ch, typename Tr, typename T, typename U>
		std::basic_ostream<Ch, Tr>& operator<<(std::basic_ostream<Ch, Tr>& out, const std::multimap<T, U>& m) {
			out << '{';
			for (auto i = m.begin(); i != m.end(); ++i) {
				out << i->first << " = " << i->second;
				auto j = i;
				if (++j != m.end())
					out << ',';
			}
			return out << '}';
		}

		template <typename Ch, typename Tr, typename T, typename U>
		std::basic_ostream<Ch, Tr>& operator<<(std::basic_ostream<Ch, Tr>& out, const std::unordered_multimap<T, U>& m) {
			out << '{';
			for (auto i = m.begin(); i != m.end(); ++i) {
				out << i->first << " = " << i->second;
				auto j = i;
				if (++j != m.end())
					out << ',';
			}
			return out << '}';
		}

	}


}

namespace std {
	using sn_StdStream::manipulator::nl;
	using namespace sn_StdStream::stlio;
}

#endif