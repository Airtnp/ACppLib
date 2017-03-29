#ifndef SN_FILESYSTEM_H
#define SN_FILESYSTEM_H

#ifndef _CRT_SECURE_NO_WARNING
#define _CRT_SECURE_NO_WARNING
#endif

#include "sn_CommonHeader.h"

//ref: Boost.filesystem
//TODO: add directory based
namespace sn_FileSystem{

	namespace path_traits {

		using codecvt_type = std::codecvt<wchar_t, char, std::mbstate_t>;

		template <class T>
		struct is_pathable { static const bool value = false; };

		template<> struct is_pathable<char*> { static const bool value = true; };
		template<> struct is_pathable<const char*> { static const bool value = true; };
		template<> struct is_pathable<wchar_t*> { static const bool value = true; };
		template<> struct is_pathable<const wchar_t*> { static const bool value = true; };
		template<> struct is_pathable<std::string> { static const bool value = true; };
		template<> struct is_pathable<std::wstring> { static const bool value = true; };
		template<> struct is_pathable<std::vector<char> > { static const bool value = true; };
		template<> struct is_pathable<std::vector<wchar_t> > { static const bool value = true; };
		template<> struct is_pathable<std::list<char> > { static const bool value = true; };
		template<> struct is_pathable<std::list<wchar_t> > { static const bool value = true; };

		template <class T> 
		inline bool empty(T* const& c_str)
		{
			return !*c_str;
		}

		template <typename T, size_t N> 
		inline bool empty(T(&x)[N])
		{
			return !x[0];
		}

		inline void convert(const char* from, const char* from_end, std::string & to)
		{
			to.append(from, from_end);
		}

		inline void convert(const char* from, std::string & to)
		{
			to += from;
		}

		inline void convert(const wchar_t* from, const wchar_t* from_end, std::wstring & to)
		{
			to.append(from, from_end);
		}

		inline void convert(const wchar_t* from, std::wstring & to)
		{
			to += from;
		}

		template <class U> inline
			void dispatch(const std::string& c, U& to)
		{
			if (c.size())
				convert(&*c.begin(), &*c.begin() + c.size(), to);
		}

		template <class U> 
		inline void dispatch(const std::wstring& c, U& to)
		{
			if (c.size())
				convert(&*c.begin(), &*c.begin() + c.size(), to);
		}
		template <class U> 
		inline void dispatch(const std::vector<char>& c, U& to)
		{
			if (c.size())
				convert(&*c.begin(), &*c.begin() + c.size(), to);
		}
		template <class U> 
		inline void dispatch(const std::vector<wchar_t>& c, U& to)
		{
			if (c.size())
				convert(&*c.begin(), &*c.begin() + c.size(), to);
		}

	}
	class FilePath {
	public:
#if defined(_WIN32)
		using value_type = wchar_t;
		static const wchar_t m_delimiter = L'\\';
		static const wchar_t m_separator = L'\\';
		static const wchar_t m_dot = L'.';
#else
		using value_type = char;
		static const wchar_t m_delimiter = '/';
		static const wchar_t m_separator = '/';
		static const wchar_t m_dot = '.';
#endif
		using string_type = std::basic_string<value_type>;
		using codecvt_type = std::codecvt<wchar_t, char, std::mbstate_t>;

		FilePath() noexcept {}
		FilePath(const FilePath& rhs) : m_path(rhs.m_path) {}
		FilePath(FilePath&& rhs) : m_path(std::move(rhs.m_path)) {}

		FilePath& operator=(const FilePath& rhs) {
			m_path = rhs.m_path;
			return *this;
		}

		FilePath& operator=(FilePath&& rhs) {
			m_path = std::move(rhs.m_path);
			return *this;
		}

		template <typename T>
		FilePath(const T& source,
			std::enable_if_t<path_traits::is_pathable<std::decay_t<T>>::value, T>* = 0) {
			path_traits::dispatch(source, m_path);
		}

		FilePath(const value_type* s) : m_path(s) {}
		FilePath(value_type* s) : m_path(s) {}
		FilePath(const string_type& s) : m_path(s) {}
		FilePath(string_type& s) : m_path(s) {}

		template <typename It>
		FilePath(It begin, It end) {
			if (begin != end) {
				std::basic_string<typename std::iterator_traits<It>::value_type> seq(begin, end);
				path_traits::convert(seq.c_str(), seq.c_str() + seq.size(), m_path);
			}
		}

		template <typename T>
		std::enable_if_t<path_traits::is_pathable<std::decay_t<T>>::value, FilePath&> operator=(const T& source) {
			m_path.clear();
			path_traits::dispatch(source, m_path);
			return *this;
		}

		FilePath& operator=(const value_type* ptr)  // required in case ptr overlaps *this
		{
			m_path = ptr;
			return *this;
		}
		FilePath& operator=(value_type* ptr)  // required in case ptr overlaps *this
		{
			m_path = ptr;
			return *this;
		}
		FilePath& operator=(const string_type& s) { 
			m_path = s;
			return *this; 
		}
		FilePath& operator=(string_type& s) { 
			m_path = s;
			return *this;
		}

		template <typename T>
		typename std::enable_if<path_traits::is_pathable<std::decay_t<T>>::value, FilePath&>::type operator+=(const T& source) {
			path_traits::dispatch(source, m_path);
		}

		FilePath& operator+=(const FilePath& p) { m_path += p.m_path; return *this; }
		FilePath& operator+=(const value_type* ptr) { m_path += ptr; return *this; }
		FilePath& operator+=(value_type* ptr) { m_path += ptr; return *this; }
		FilePath& operator+=(const string_type& s) { m_path += s; return *this; }
		FilePath& operator+=(string_type& s) { m_path += s; return *this; }
		FilePath& operator+=(value_type c) { m_path += c; return *this; }

		template <typename T>
		std::enable_if_t<std::is_integral<T>::value, FilePath&> operator+= (T c) {
			T tmp[2];
			tmp[0] = c;
			tmp[1] = 0;
			path_traits::dispatch(tmp, m_path);
			return *this;
		}

		const string_type&  native() const noexcept { return m_path; }
		const value_type*   c_str() const noexcept { return m_path.c_str(); }
		string_type::size_type size() const noexcept { return m_path.size(); }


	private:
		string_type m_path;

	};
}




#endif