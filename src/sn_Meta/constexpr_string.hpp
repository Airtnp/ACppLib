#ifndef SN_META_CONSTEXPR_STRING_H
#define SN_META_CONSTEXPR_STRING_H

#include "../sn_CommonHeader.h"

namespace sn_Meta {

    namespace constexpr_string {
		// ref : https://github.com/mclow/string_view/blob/master/include/experimental/string_view
		// another_impl(not const, support append): http://stackoverflow.com/a/15912824
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
				// Laji VS2015 constexpr ?
				// pos > size() ? throw std::out_of_range("string_view:substr") : ((m_sz - pos > len) ? (return string_view(data() + pos, len)) : (return string_view(data() + pos, m_sz - pos)));
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