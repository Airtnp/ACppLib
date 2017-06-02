#ifndef SN_STRING_H
#define SN_STRING_H

#include "sn_CommonHeader.h"
#include "sn_Assist.hpp"

//TODO: add string view
//TODO: add UTF-8/UTF-16/UCS-2/ANSI/WIDE support
//TODO: add bytestr
//TODO: add async support
namespace sn_String {
	namespace formatter {
		
		std::array<int, 4> find_string_token(const std::string& base_string, std::string::size_type& pos, std::string::size_type& prev_len, const std::string& arg) {
			int pprev_len = prev_len;
			int first = base_string.find('{', pos);
			int second = base_string.find('}', first);
			std::string numeric = base_string.substr(first + 1, second - first - 1);
			if (numeric.find_first_not_of("0123456789") == numeric.npos) {
				pos = second;
				std::array<int, 4> res{ first, second, pprev_len, atoi(numeric.c_str()) };
				prev_len = arg.length();
				return res;
			}
			else {
				++pos;
				return find_string_token(base_string, pos, prev_len, arg);
			}
		}

		template <typename T, std::size_t N, std::size_t ...I>
		void string_formatter_impl(std::string& base_string, const std::vector<std::array<int, 4>>& v, std::index_sequence<I...>, std::array<T, N> arr) {
			std::initializer_list<int>{ (base_string.replace(std::get<0>(v[I]) + std::get<2>(v[I]) - 3 - static_cast<int>(floor(I == 0 ? 0 : log10(I))), std::get<1>(v[I]) - std::get<0>(v[I]) + 1, arr[I]), 0)...};
		}

		template <typename ...Args>
		std::string string_formatter(std::string base_string, Args... args) {
			auto format_list = std::make_index_sequence<sizeof...(args)>();
			std::vector<std::array<int, 4>> token_pos{};
			std::string::size_type current_pos = 0;
			std::string::size_type previous_length = 3;
			std::array<std::string, sizeof...(args)> params{ args... };
			std::initializer_list<int>{(token_pos.push_back(find_string_token(base_string, current_pos, previous_length, args)), 0)...};
			std::array<std::size_t, sizeof...(args)> idx;
			for (std::size_t i = 0; i < idx.size(); ++i) {
				idx[i] = i;
			}
			std::sort(idx.begin(), idx.end(), [&token_pos](std::size_t a, std::size_t b) {
				return std::get<3>(token_pos[a]) < std::get<3>(token_pos[b]);
			});
			std::array<std::string, sizeof...(args)> params_copy;
			for (std::size_t i = 0; i < idx.size(); ++i) {
				params_copy[i] = params[idx[i]];
				if (i == 0)
					std::get<2>(token_pos[i]) = 3;
				else
					std::get<2>(token_pos[i]) = params_copy[i - 1].length();
			}
			string_formatter_impl(base_string, token_pos, format_list, params_copy);
			return base_string;
		}
	}
	namespace splitter {
				using std::vector;
		using std::size_t;
		struct split_option {
			enum empty_t { empty_remain, empty_discard };
		};
		
		template<typename T>
		vector<T> string_split(const T& str, const T& delimiter = "\t", split_option::empty_t empty_option = split_option::empty_remain) {
			vector<T> v;
			size_t current;
			size_t next = -1;
			do {
				if (empty_option == split_option::empty_discard) {
					next = str.find_first_not_of(delimiter, next + 1);
					if (next == string::npos)
						break;
					next -= 1;
				}
				current = next + 1;
				next = str.find_first_of(delimiter, current);
				v.push_back(str.substr(current, next - current));
			} while (next != string::npos);
			return v;
		}

	}

	namespace trimmer {
		 std::string trim(const std::string& str, char delim = ' ') {
			std::string::size_type pos = str.find_first_not_of(delim);
			if (pos == std::string::npos)
				return str;
			std::string::size_type pos2 = str.find_last_not_of(delim);
			if (pos2 != std::string::npos)
				return str.substr(pos, pos2 - pos + 1);
			return str.substr(pos);
		}
	}

	namespace wide_conv {
		using std::string;
		using std::wstring;
		using std::stringstream;
		using std::setw;
		using std::setfill;
		string wstring_to_utf8(const wstring& str) {
			std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
			return myconv.to_bytes(str);
		}

		wstring gbk_to_wstring(const string& gbk_str) {
			string gbk_locale_name = ".936";
			std::wstring_convert<std::codecvt_byname<wchar_t, char, mbstate_t>> conv(new std::codecvt_byname<wchar_t, char, mbstate_t>(gbk_locale_name));
			return conv.from_bytes(gbk_str);
		}

		//for outputing chinese rows, we use wcout(.imbue(local("chs"))) << utf8_to_wstring(content)
		wstring utf8_to_wstring(const string& str) {
			std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
			return myconv.from_bytes(str);
		}

		wstring string_to_wstring(const string& str) {
			try {
				std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
				wstring wstr = conv.from_bytes(str);
				return wstr;
			}
			catch (std::range_error&)
			{
				wstring wstr = gbk_to_wstring(str);
				return wstr;
			}
		}

		string wstring_to_string(const wstring& wstr) {
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
			string str = conv.to_bytes(wstr);
			return str;
		}

		//for chinese fields, we use hex(field_name) = wstring_to_hex(L'example')
		string wstring_to_hex(const wstring& str) {
			string u8str = wstring_to_utf8(str);
			stringstream ss;
			ss << std::hex;
			for (unsigned char c : u8str)
				ss << setw(2) << setfill('0') << static_cast<int>(c);
			string hexstr = ss.str();
			hexstr = "\'" + hexstr + "\'";
			return hexstr;
		}

		//to chinese name, use wstring_to_unhex('example');
		string wstring_to_unhex(const wstring& str) {
			string unhexstr = "unhex(" + wstring_to_hex(str) + ")";
			return unhexstr;
		}

		string string_to_hex(const string& str) {
			wstring wstr = string_to_wstring(str);
			return wstring_to_hex(wstr);
		}

		string string_to_unhex(const string& str) {
			wstring wstr = string_to_wstring(str);
			return wstring_to_unhex(wstr);
		}

		void hex_print(const std::string& s) {
			std::cout << std::hex << std::setfill('0');
			for(unsigned char c : s)
				std::cout << std::setw(2) << static_cast<int>(c) << ' ';
			std::cout << std::dec << '\n';
		}
	}
	namespace misc {
		using std::to_string;
		using std::string;
		string to_string(string input) {
			return input;
		}

		string to_string(char* input) {
			return static_cast<string>(input);
		}

	}
	
	namespace cstring {
		class CStringHelper {
		public: //unsigned  = unsigned int
			CStringHelper(const char* str, unsigned int len) : str_(str), len_(len) {
				assert(strlen(str_) == len_);
			}
			const char* str_;
			const unsigned int len_;
		};

		template <typename CharT>
		using basic_zstring = CharT*;
		using zstring = basic_zstring<char>;
		using czstring = basic_zstring<const char>;
		using wzstring = basic_zstring<wchar_t>;
		using cwzstring = basic_zstring<const wchar_t>;

	}

	namespace constexpr_string {
		//ref : https://github.com/mclow/string_view/blob/master/include/experimental/string_view
		//another_impl(not const, support append): http://stackoverflow.com/a/15912824
		class string_view {
		private:
			const char* m_str;
			const std::size_t m_sz;
		public:
			static constexpr const std::size_t npos = std::size_t(-1);
			using value_type = const char*;
			using const_pointer = const char* const;
			using const_iterator = const_pointer;
			using size_type = std::size_t;
			using difference_type = std::ptrdiff_t;

			constexpr string_view(const char* str, const std::size_t len): m_str(str), m_sz(len) {}
			
			template <std::size_t N>
			constexpr string_view(const char(&a)[N]):
				m_str(a), m_sz(N) {}
			
			
			
			constexpr const char operator[](std::size_t n) {
				return n < m_sz ? m_str[n] : throw std::out_of_range("string_view:ctor");
			}
			constexpr string_view(const string_view& rhs) = default;
			string_view& operator=(const string_view& rhs) = default;

			constexpr const_iterator begin() const noexcept {
				return cbegin();
			}

			constexpr const_iterator end() const noexcept {
				return cend();
			}

			constexpr const_iterator cbegin() const noexcept {
				return m_str;
			}

			constexpr const_iterator cend() const noexcept {
				return m_str + m_sz;
			}


			constexpr std::size_t size() const {
				return m_sz;
			}

			constexpr std::size_t length() const {
				return m_sz;
			}
			constexpr const char* data() const {
				return m_str;
			}

			constexpr string_view substr(const std::size_t pos, const std::size_t len = npos) const {
				return string_view(data() + pos, len);
				// Laji VS2015 ?
				//pos > size() ? throw std::out_of_range("string_view:substr") : ((m_sz - pos > len) ? (return string_view(data() + pos, len)) : (return string_view(data() + pos, m_sz - pos)));
			}

			// VS2015 doesn't support fully Cpp14-constexpr
			/*
			constexpr int compare(string_view rhs) const noexcept {
				std::size_t len = std::min(size(), rhs.size());
				for (std::ptrdiff_t i = 0; i < len; ++i)
					if (m_str[i] != rhs[i])
						return 0;
				
			}*/


		};

		// TODO: write a formatter
		template<std::size_t ...str>
		struct string {
			static constexpr const char chars[sizeof...(str)+1] = { str..., '\0' };
		};

		template <std::size_t ...I1, std::size_t ...I2>
		string<I1..., I2...> operator+(string<I1...>, string<I2...>) {
			return {};
		}

		template <std::size_t ...I>
		constexpr string<I...> make_string_impl(const char* str, std::index_sequence<I...>) {
			return string<str[I]...>{};
		}

		template <std::size_t N>
		constexpr auto make_string(const char(&str)[N]) {
			return make_string_impl(str, std::make_index_sequence<N - 1>{});
		}

		template <char ...str>
		constexpr string<str...> operator "" _sn() {
			return string<str...>{};
		}

		inline string_view operator "" _snsv(const char* str, std::size_t len) {
			return string_view(str, len);
		}

	}

}




#endif