#ifndef SN_LINQ_H
#define SN_LINQ_H

#include "sn_CommonHeader.h"
#include "sn_Assist.hpp"
#include "sn_Macro.hpp"

// Simple implement: just use pipeline in sn_Function
// R = ChainHead (beginning of chain) | where<a, b> (template function restrict)... | ....
// from(v) (vector or some container) | R (functor) 
// ref: https://github.com/vczh/vczh_toys/blob/master/CppLinq/CppLinq/linq.h
namespace sn_LINQ {
	namespace linq {
		using sn_Assist::sn_function_traits::function_traits;

		template <typename T, typename V = void>
		struct T_iterator_type {
			using type = decltype(std::addressof(std::declval<T&>()));
		};
		template <typename T>
		struct T_iterator_type<T, std::void_t<typename T::iterator>> {
			using type = typename T::iterator;
		};

		template <typename TIt>
		struct D_iterator_type {
			using type = typename std::remove_reference<decltype(*std::declval<TIt>())>::type;
		};

		template <typename TIt>
		using deference_type = typename D_iterator_type<TIt>::type;

		template <typename T>
		using iterator_type = typename T_iterator_type<T>::type;

		class LinqException : public std::exception {
		public:
			std::string m_message;
			LinqException(const std::string& message)
				: m_message(message) {}
		};

		template <typename F>
		using result_type = typename function_traits<F>::result_type;

		template <typename T, typename U>
		struct zip_pair : public sn_Assist::sn_functional_base::less_than<zip_pair<T, U>> {
			T m_first;
			U m_second;
			zip_pair() {}
			zip_pair(const T& first, const T& second)
				: m_first(first), m_second(second) {}
			template <typename X, typename Y>
			zip_pair(const zip_pair<X, Y>& rhs)
				: m_first(rhs.m_first), m_second(rhs.m_second) {}
			static int compare(const zip_pair& a, const zip_pair& b) {
				if (a.m_first != a.m_second)
					return a.m_first < a.m_second ? -1 : 1;
				if (b.m_first != b.m_second)
					return b.m_first < b.m_second ? -1 : 1;
				return 0;
			}
			bool operator==(const zip_pair& rhs) const {
				return m_first == rhs.m_first && m_second == rhs.m_second;
			}
			bool operator<(const zip_pair& rhs) const {
				return compare(*this, rhs) < 0;
			}
		};

		template <typename T, typename U, typename W>
		using join_pair = zip_pair<T, zip_pair<U, W>>;
	}

	namespace linq_iterator {

		template <typename T>
		class linq_iterator_base {
			using TSelf = T;
		public:
			TSelf& operator++() {
				++(static_cast<T*>(this)->m_iterator);
				return *(static_cast<T*>(this))
			}
			TSelf operator++(int) {
				TSelf t = *(static_cast<T*>(this));
				++(static_cast<T*>(this)->m_iterator);
				return t;
			}
			bool operator==(const TSelf& rhs) const {
				return (static_cast<T*>(this)->m_iterator) == rhs.m_iterator;
			}
			bool operator!=(const TSelf& rhs) const {
				return (static_cast<T*>(this)->m_iterator) != rhs.m_iterator;
			}
		};

		template <typename T>
		class hide_type_iterator {
		private:
			using TSelf = hide_type_iterator<T>;
			class iterator_interface {
				using TSelf = iterator_interface;
			public:
				virtual std::shared_ptr<TSelf> next() = 0;
				virtual T deref() = 0;
				virtual bool equals(const std::shared_ptr<TSelf>& it) = 0;
			};

			template <typename TIt>
			class iterator_implement : public iterator_interface {
				using TSelf = iterator_implement<TIt>;
				TIt m_iterator;
			public:
				iterator_implement(const TIt& iterator)
					: m_iterator(iterator) {}
				std::shared_ptr<iterator_interface> next() override {
					TIt t = m_iterator;
					++t;
					return std::make_shared<TSelf>(t);
				}

				T deref() override {
					return *m_iterator;
				}

				bool equals(const std::shared_ptr<iterator_interface>& it) override {
					auto impl = std::dynamic_pointer_cast<TSelf>(it);
					return impl && (m_iterator == impl->m_iterator);
				}
			};
			std::shared_ptr<iterator_interface> m_iterator;  //pimpl
		public:
			template <typename TIt>
			hide_type_iterator(const TIt& iterator)
				: m_iterator(std::make_shared<iterator_implement<TIt>>(iterator)) {}
			TSelf& operator++() {
				m_iterator = m_iterator->next();
				return *this;
			}
			TSelf operator++(int) {
				TSelf t = *this;
				m_iterator = m_iterator->next();
				return t;
			}
			T operator*() const {
				return m_iterator->deref();
			}
			bool operator==(const TSelf& it) const {
				return m_iterator->equals(it.m_iterator);
			}
			bool operator!=(const TSelf& it) const {
				return !m_iterator->equals(it.m_iterator);
			}
		};

		template <typename T>
		class storage_iterator {
			using TSelf = storage_iterator<T>;
			std::shared_ptr<std::vector<T>> m_values;
			typename std::vector<T>::iterator m_iterator;
		public:
			storage_iterator(const std::shared_ptr<std::vector<T>>& values,
				const std::vector<T>::iterator& iter)
				: m_values(values), m_iterator(iter) {}
			TSelf& operator++() {
				++m_iterator;
				return *this;
			}
			TSelf operator++(int) {
				TSelf t = *this;
				++m_iterator;
				return *this;
			}
			T operator*() const {
				return *m_iterator;
			}
			bool operator==(const TSelf& rhs) const {
				return m_iterator == rhs.m_iterator;
			}
			bool operator!=(const TSelf& rhs) const {
				return m_iterator != rhs.m_iterator;
			}
		};

		template <typename T>
		class empty_iterator {
			using TSelf = empty_iterator<T>;
		public:
			empty_iterator() {}
			TSelf& operator++() {
				return *this;
			}

			TSelf operator++(int) {
				return *this;
			}

			T operator*() const {
				throw linq::LinqException("Error: Get a value from an empty collection.");
			}

			bool operator==(const TSelf& it) const {
				return true;
			}

			bool operator!=(const TSelf& it) const {
				return false;
			}
		};

		template <typename TIt, typename TFunc>
		class select_iterator {
			using TSelf = select_iterator<TIt, TFunc>;
			TIt m_iterator;
			TFunc m_func;
		public:
			select_iterator(const TIt& iterator, const TFunc& f)
				: m_iterator(iterator), m_func(f) {}
			TSelf& operator++() {
				++m_iterator;
				return *this;
			}
			TSelf operator++(int) {
				TSelf t = *this;
				++m_iterator;
				return *this;
			}
			auto operator*() const -> decltype(m_func(*m_iterator)) {
				return m_func(*m_iterator);
			}
			bool operator==(const TSelf& rhs) const {
				return m_iterator == rhs.m_iterator;
			}
			bool operator!=(const TSelf& rhs) const {
				return m_iterator != rhs.m_iterator;
			}
		};

		template <typename TIt, typename TFunc>
		class where_iterator {
			using TSelf = where_iterator<TIt, TFunc>;
			TIt m_iterator;
			TIt m_end;
			TFunc m_func;
			void move_iterator(bool next) {
				if (m_iterator == m_end)
					return;
				if (next)
					++m_iterator;
				while (m_iterator != m_end && !(m_func(*m_iterator)))
					++m_iterator;
			}
		public:
			where_iterator(const TIt& iterator, const TIt& end, const TFunc& f)
				: m_iterator(iterator), m_end(end), m_func(f) {
				move_iterator(false);
			}
			TSelf& operator++() {
				move_iterator(true);
				return *this;
			}
			TSelf operator++(int) {
				TSelf t = *this;
				move_iterator(true);
				return t;
			}
			linq::deference_type<TIt> operator*(const TSelf& rhs) const {
				return *m_iterator;
			}
			bool operator==(const TSelf& rhs) const {
				return m_iterator == rhs.m_iterator;
			}
			bool operator!=(const TSelf& rhs) const {
				return m_iterator != rhs.m_iterator;
			}
		};

		template <typename TIt>
		class skip_iterator : public linq_iterator_base<skip_iterator<TIt>> {
			using TSelf = skip_iterator<TIt>;
			TIt m_iterator;
			TIt m_end;
		public:
			skip_iterator(const TIt& iter, const TIt& end, std::size_t count)
				: m_iterator(iter), m_end(end) {
				for (std::size_t i = 0; i < count && m_iterator != m_end; ++i, ++m_iterator);
			}
			linq::deference_type<TIt> operator*() const {
				return *m_iterator;
			}
		};

		template <typename TIt, typename TFunc>
		class skip_while_iterator : public linq_iterator_base<skip_while_iterator<TIt, TFunc>> {
			using TSelf = skip_while_iterator<TIt, TFunc>;
			TIt m_iterator;
			TIt m_end;
			TFunc m_func;
		public:
			skip_while_iterator(const TIt& iter, const TIt& end, const TFunc& func)
				: m_iterator(iter), m_end(end), m_func(func) {
				while (m_iterator != m_end && m_func(*m_iterator))
					++m_iterator;
			}
			linq::deference_type<TIt> operator*() const {
				return *m_iterator;
			}
		};

		template <typename TIt>
		class take_iterator {
			using TSelf = take_iterator<TIt>;
			TIt m_iterator;
			TIt m_end;
			std::size_t m_count;
			std::size_t m_current;
		public:
			take_iterator(const TIt& iter, const TIt& end, std::size_t count)
				: m_iterator(iter), m_end(end), m_count(count), m_current(0) {
				if (m_count == m_current)
					m_iterator = m_end;
			}
			TSelf& operator++() {
				if (++m_current == m_count)
					m_iterator = m_end;
				else
					++m_iterator;
				return *this;
			}
			TSelf operator++(int) {
				TSelf t = *this;
				if (++m_current == m_count)
					m_iterator = m_end;
				else
					++m_iterator;
				return t;
			}
			linq::deference_type<TIt> operator*() const {
				return *m_iterator;
			}
			bool operator==(const TSelf& rhs) const {
				return m_iterator == rhs.m_iterator;
			}
			bool operator!=(const TSelf& rhs) const {
				return m_iterator != rhs.m_iterator;
			}
		};

		template <typename TIt, typename TFunc>
		class take_while_iterator {
			using TSelf = take_while_iterator<TIt, TFunc>;
			TIt m_iterator;
			TIt m_end;
			TFunc m_func;
		public:
			take_while_iterator(const TIt& iter, const TIt& end, const TFunc& func)
				: m_iterator(iter), m_end(end), m_func(func) {
				while (m_iterator != m_end && !m_func(*m_iterator))
					m_iterator = m_end;
			}
			TSelf& operator++() {
				if (!m_func(*++m_iterator))
					m_iterator = m_end;
				return *this;
			}
			TSelf operator++(int) {
				TSelf t = *this;
				if (!m_func(*++m_iterator))
					m_iterator = m_end;
				return t;
			}
			linq::deference_type<TIt> operator*() const {
				return *m_iterator;
			}
			bool operator==(const TSelf& rhs) const {
				return m_iterator == rhs.m_iterator;
			}
			bool operator!=(const TSelf& rhs) const {
				return m_iterator != rhs.m_iterator;
			}
		};

		template <typename TIt1, typename TIt2>
		class concat_iterator {
			using TSelf = concat_iterator<TIt1, TIt2>;
			TIt1 m_current1;
			TIt1 m_end1;
			TIt2 m_current2;
			TIt2 m_end2;
			bool m_endFirst;
		public:
			concat_iterator(const TIt1& current1, const TIt1& end1, const TIt2& current2, const TIt2& end2)
				:m_current1(current1), m_end1(end1), m_current2(current2), m_end2(end2), m_endFirst(current1 == end1) {}
			TSelf& operator++() {
				if (!m_endFirst) {
					if (++m_current1 == m_end1)
						m_endFirst = true;
				}
				else
					++m_current2;
				return *this;
			}
			TSelf operator++(int) {
				TSelf t = *this;
				if (!m_endFirst) {
					if (++m_current1 == m_end1)
						m_endFirst = true;
				}
				else
					++m_current2;
				return t;
			}
			linq::deference_type<TIt1> operator*() const {
				return m_endFirst ? *m_current2 : *m_current1;
			}
			bool operator==(const TSelf& rhs) const {
				if (m_endFirst != rhs.m_endFirst)
					return false;
				return m_endFirst ? m_current2 == rhs.m_current2 : m_current1 == rhs.m_current1;
			}
			bool operator!=(const TSelf& rhs) const {
				if (m_endFirst != rhs.m_endFirst)
					return true;
				return m_endFirst ? m_current2 != rhs.m_current2 : m_current1 != rhs.m_current1;
			}
		};

		template<typename TIt1, typename TIt2>
		class zip_iterator
		{
			using TSelf = zip_iterator<TIt1, TIt2>;
			using TElement = zip_pair<linq::deference_type<TIt1>, linq::deference_type<TIt2>>;
			TIt1 m_current1;
			TIt1 m_end1;
			TIt2 m_current2;
			TIt2 m_end2;
		public:
			zip_iterator(const TIt1& current1, const TIt1& end1, const TIt2& current2, const TIt2& end2)
				: m_current1(current1), m_end1(end1), m_current2(current2), m_end2(end2) {}
			TSelf& operator++() {
				if (m_current1 != m_end1 && m_current2 != m_end2)
				{
					m_current1++;
					m_current2++;
				}
				return *this;
			}
			TSelf operator++(int)
			{
				TSelf t = *this;
				if (m_current1 != m_end1 && m_current2 != m_end2)
				{
					m_current1++;
					m_current2++;
				}
				return t;
			}
			TElement operator*() const {
				return TElement(*m_current1, *m_current2);
			}
			bool operator==(const TSelf& rhs) const {
				return m_current1 == rhs.m_current1 && m_current2 == rhs.m_current2;
			}
			bool operator!=(const TSelf& rhs) const {
				return m_current1 != rhs.m_current1 || m_current2 != rhs.m_current2;
			}
		};
	}

	namespace linq {
		using namespace linq_iterator;
		template <typename TIt>
		class LinqEnumerable;

		template <typename T>
		class Linq;
		
		template <typename T>
		Linq<T> from_values(std::shared_ptr<std::vector<T>> xs) {
			return LinqEnumerable<storage_iterator<T>>(
				storage_iterator<T>(xs, xs->begin()),
				storage_iterator<T>(xs, xs->end())
				);
		}

		template <typename T>
		Linq<T> from_values(const T& v) {
			auto xs = std::make_shared<std::vector<T>>();
			xs->push_back(v);
			return from_values(xs);
		}

		template <typename T>
		Linq<T> from_empty() {
			return LinqEnumerable<empty_iterator<T>>(
				empty_iterator<T>(),
				empty_iterator<T>()
				);
		}

		template <typename T>
		auto from(const T& container)->LinqEnumerable<decltype(std::begin(container))>;

		template <typename TIt>
		class LinqEnumerable {
			using TElement = std::decay_t<deference_type<TIt>>;
			TIt m_begin;
			TIt m_end;
		public:
			LinqEnumerable() {}
			LinqEnumerable(const TIt& begin, const TIt& end)
				: m_begin(begin), m_end(end) {}
			TIt begin() const {
				return m_begin;
			}
			TIt end() const {
				return m_end;
			}

#define SN_LINQ_SUPPORT_STL_CONTAINER(NAME) \
	template <typename TIt2> \
	auto NAME(const LinqEnumerable<TIt2>& e) const -> decltype(NAME##_(e)) { \
		return NAME##_(e); \
	} \
	\
	template <typename TC> \
	auto NAME(const LinqEnumerable<TC>& e) const -> decltype(NAME##_(from(e))) { \
		return NAME##_(from(e)); \
	} \
	\
	template <typename T> \
	auto NAME(const std::initializer<T>& e) const -> decltype(NAME##_(from(e))) { \
		return NAME##_(from(e)); \
	} \

#define SN_LINQ_SUPPORT_STL_CONTAINER_EX(NAME, TYPES, PARAMETERS, ARGUMENTS) \
	template <typename TIt2, TYPES> \
	auto NAME(const LinqEnumerable<TIt2>& e, PARAMETERS) const -> decltype(this->NAME##_(e, ARGUMENTS)) { \
		return NAME##_(e, ARGUMENTS); \
	} \
	\
	template <typename TC, TYPES> \
	auto NAME(const LinqEnumerable<TC>& e, PARAMETERS) const -> decltype(this->NAME##_(from(e), ARGUMENTS)) { \
		return NAME##_(from(e), ARGUMENTS); \
	} \
	\
	template <typename T, TYPES> \
	auto NAME(const std::initializer<T>& e, PARAMETERS) const -> decltype(this->NAME##_(from(e), ARGUMENTS)) { \
		return NAME##_(from(e), ARGUMENTS); \
	} \



		};

	}


}






#endif