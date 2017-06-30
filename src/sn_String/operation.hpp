#ifndef SN_STRING_OPERATION_H
#define SN_STRING_OPERATION_H

#include "../sn_CommonHeader.h"


namespace sn_String {
    std::string trim(const std::string& str, char delim = ' ') {
        std::string::size_type pos = str.find_first_not_of(delim);
        if (pos == std::string::npos)
            return str;
        std::string::size_type pos2 = str.find_last_not_of(delim);
        if (pos2 != std::string::npos)
            return str.substr(pos, pos2 - pos + 1);
        return str.substr(pos);
    }

    struct split_option {
			enum empty_t { empty_remain, empty_discard };
		};
		
    template<typename T>
    std::vector<T> split(const T& str, const T& delimiter = "\t", split_option::empty_t empty_option = split_option::empty_remain) {
        std::vector<T> v;
        std::size_t current;
        std::size_t next = -1;
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
        } while (next != std::string::npos);
        return v;
    }

    namespace detail {
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
    }

    template <typename ...Args>
    std::string format(std::string base_string, Args... args) {
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
        detail::string_formatter_impl(base_string, token_pos, format_list, params_copy);
        return base_string;
    }
}





#endif