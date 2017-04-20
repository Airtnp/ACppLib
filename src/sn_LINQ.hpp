#ifndef SN_LINQ_H
#define SN_LINQ_H

#include "sn_CommonHeader.h"
#include "sn_Assist.hpp"
#include "sn_Macro.hpp"

// Simple implement: just use pipeline in sn_Function
// R = ChainHead (beginning of chain) | where<a, b> (template function restrict)... | ....
// from(v) (vector or some container) | R (functor) 
// ref: https://github.com/vczh-libraries/Release/blob/master/Import/Vlpp.h (Change ref by license)
// TODO: for Cpp1z, result_of_t -> invoke_result_t
// more version: https://github.com/MichaelSuen-thePointer/mqLinq/blob/master/mqLinq/linq.h
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
				const typename std::vector<T>::iterator& iter)
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
			using TElement = linq::zip_pair<linq::deference_type<TIt1>, linq::deference_type<TIt2>>;
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
		Linq<T> from_value(const T& v) {
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

			// overload for container/iterator type of LinqEnumerable
			// template <TIt2>
			// friend class LinqEnumerable<TIt2>;

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
	auto NAME(const std::initializer_list<T>& e) const -> decltype(NAME##_(from(e))) { \
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
	auto NAME(const std::initializer_list<T>& e, PARAMETERS) const -> decltype(this->NAME##_(from(e), ARGUMENTS)) { \
		return NAME##_(from(e), ARGUMENTS); \
	} \

#define SN_LINQ_SUPPORT_ITERATOR_FUNC(NAME) \
	template <typename TFunc> \
	LinqEnumerable<NAME##_iterator<TIt, TFunc>> NAME(const TFunc& f) const { \
		return LinqEnumerable<NAME##_iterator<TIt, TFunc>>( \
			NAME##_iterator<TIt, TFunc>(m_begin, m_end, f), \
			NAME##_iterator<TIt, TFunc>(m_end, m_end, f), \
			); \
	} \


			template <typename TFunc>
			LinqEnumerable<select_iterator<TIt, TFunc>> select(const TFunc& f) const {
				return LinqEnumerable<select_iterator<TIt, TFunc>>(
					select_iterator<TIt, TFunc>(m_begin, f),
					select_iterator<TIt, TFunc>(m_end, f),
					);
			}

			template <typename TFunc>
			LinqEnumerable<where_iterator<TIt, TFunc>> where(const TFunc& f) const {
				return LinqEnumerable<where_iterator<TIt, TFunc>>(
					where_iterator<TIt, TFunc>(m_begin, m_end, f),
					where_iterator<TIt, TFunc>(m_end, m_end, f),
					);
			}


			SN_LINQ_SUPPORT_ITERATOR_FUNC(take_while)

			SN_LINQ_SUPPORT_ITERATOR_FUNC(skip_while)

			LinqEnumerable<skip_iterator<TIt>> skip(std::size_t count) const {
				return LinqEnumerable<skip_iterator<TIt>>(
					skip_iterator<TIt>(m_begin, m_end, count),
					skip_iterator<TIt>(m_end, m_end, count),
					);
			}

			LinqEnumerable<take_iterator<TIt>> take(std::size_t count) const {
				return LinqEnumerable<skip_iterator<TIt>>(
					take_iterator<TIt>(m_begin, m_end, count),
					take_iterator<TIt>(m_end, m_end, count),
					);
			}

			template <typename TIt2>
			LinqEnumerable<concat_iterator<TIt, TIt2>> concat_(const LinqEnumerable<TIt2>& e) const {
				return LinqEnumerable<concat_iterator<TIt, TIt2>>(
					concat_iterator<TIt, TIt2>(m_begin, m_end, e.begin(), e.end()),
					concat_iterator<TIt, TIt2>(m_end, m_end, e.end(), e.end()),
					);
			}

			SN_LINQ_SUPPORT_STL_CONTAINER(concat)

			template <typename T>
			bool contains(const T& t) const {
				for (auto it = m_begin; it != m_end; ++i)
					if (*it == t)
						return true;
				return false;
			}

			std::size_t count() const {
				std::size_t counter = 0;
				for (auto it = m_begin; it != m_end; ++i)
					++counter;
				return counter;
			}

			Linq<TElement> default_if_empty(const TElement& value) const {
				if (count() == 0)
					return from_value(value);
				else
					return *this;
			}

			TElement element_at(std::size_t index) const {
				if (index >= 0) {
					std::size_t counter = 0;
					for (auto it = m_begin; it != m_end; ++i) {
						if (counter == index)
							return *it;
						++counter;
					}
				}
				throw LinqException("Error: Argument out of range.");
			}

			bool empty() const {
				return m_begin == m_end;
			}

			TElement first() const {
				if (empty())
					throw LinqException("Error: Failed to get a value from an empty collection.");
				return *m_begin;
			}

			TElement first_or(const TElement& value) const {
				return empty() ? value : *m_begin;
			}

			TElement last() const {
				if (empty())
					throw LinqException("Error: Failed to get a value from an empty collection.");
				auto it = m_begin;
				TElement result = *it;
				while (++it != m_end)
					result = *it;
				return result;
			}

			TElement last_or(const TElement& value) const {
				auto it = m_begin;
				TElement result = value;
				while (++it != m_end)
					result = *it;
				return result;
			}

			LinqEnumerable<TIt> single() const {
				auto it = m_begin;
				if (it == m_end)
					throw LinqException("Error: Failed to get a value from an empty collection.");
				++it;
				if (it != m_end)
					throw LinqException("Error: Failed to get single value from a nonsingle collection.");
				return *this;
			}

			Linq<TIt> single_or(const TElement& value) const {
				auto it = m_begin;
				if (it == m_end)
					return from_value(value);
				++it;
				if (it != m_end)
					throw LinqException("Error: Failed to get single value from a nonsingle collection.");
				return *this;
			}

			template <typename TIt2>
			bool sequence_equal_(const LinqEnumerable<TIt2>& e) const {
				auto x = m_begin;
				auto xe = m_end;
				auto y = e.m_begin;
				auto y2 = e.m_end;

				while (x != xe && y != ye)
					if (*x++ != *y++)
						return false;
				return x == xe && y == ye;
			}

			SN_LINQ_SUPPORT_STL_CONTAINER(sequence_equal)

			Linq<TElement> distinct() const {
				std::set<TElement> set;
				auto xs = std::make_shared<std::vector<TElement>>();
				for (auto it = m_begin; it != m_end; ++it) {
					if (set.insert(*it).second) {
						xs->push_bach(*it);
					}
				}
				return from_values(xs);
			}

			template <typename TIt2>
			Linq<TElement> except_with_(const LinqEnumerable<TIt2>& e) const {
				std::set<TElement> set(e.begin(), e.end());
				auto xs = std::make_shared<std::vector<TElement>>();
				for (auto it = m_begin; it != m_end; ++it) {
					if (set.insert(*it).second) {
						xs->push_bach(*it);
					}
				}
				return from_values(xs);
			}

			template <typename TIt2>
			Linq<TElement> intersect_with_(const LinqEnumerable<TIt2>& e) const {
				std::set<TElement> seti, set(e.begin(), e.end());
				auto xs = std::make_shared<std::vector<TElement>>();
				for (auto it = m_begin; it != m_end; ++it) {
					if (seti.insert(*it).second && !set.insert(*it).second) {
						xs->push_bach(*it);
					}
				}
				return from_values(xs);
			}

			template <typename TIt2>
			Linq<TElement> union_with_(const LinqEnumerable<TIt2> e) const {
				return concat(e).distinct();
			}

			SN_LINQ_SUPPORT_STL_CONTAINER(except_with)
	
			SN_LINQ_SUPPORT_STL_CONTAINER(intersect_with)
			
			SN_LINQ_SUPPORT_STL_CONTAINER(union_with)

			template <typename TFunc>
			TElement aggregate(const TFunc& f) const {
				auto it = m_begin;
				if (it == m_end)
					throw LinqException("Error: Failed to get a value from an empty collection.");
				TElement result = *it;
				while (++it != m_end)
					result = f(result, *it);
				return result;
			}

			template <typename TR, typename TFunc>
			TR aggregate(const TR init, const TFunc& f) const {
				auto it = m_begin;
				if (it == m_end)
					throw LinqException("Error: Failed to get a value from an empty collection.");
				TR result = init;
				while (++it != m_end)
					result = f(result, *it);
				return result;
			}

			template <typename TFunc>
			bool all(const TFunc& f) const {
				return select(f).aggregate(true, [](bool a, bool b) {
					return a && b;
				});
			}

			template <typename TFunc>
			bool any(const TFunc& f) const {
				return !where(f).empty();
			}

			template <typename TR>
			TR average() const {
				if (m_begin == m_end)
					throw LinqException("Error: Failed to get a value from an empty collection.");
				TR sum = 0;
				std::size_t counter = 0;
				for (auto it = m_begin; it != m_end; ++it) {
					sum += static_cast<TR>(*it);
					++counter;
				}
				return sum / counter;
			}

			TElement max() const {
				return aggregate([](const TElement& a, const TElement& b) {
					return a > b ? a : b;
				});
			}

			TElement min() const {
				return aggregate([](const TElement& a, const TElement& b) {
					return a < b ? a : b;
				});
			}

			TElement sum() const {
				return aggregate(0, [](const TElement& a, const TElement& b) {
					return a + b;
				});
			}

			TElement produce() const {
				return aggregate(0, [](const TElement& a, const TElement& b) {
					return a * b;
				});
			}

			template <typename TFunc>
			auto select_many(const TFunc& f) const 
				-> Linq<deference_type<iterator_type<std::result_of_t<decltype(f)(TElement)>>>> {
				using TCollection = std::result_of_t<decltype(f)(TElement)>;
				using TValue = deference_type<iterator_type<TCollection>>;
				return select(f).aggregate(from_empty<TValue>{}, [](const Linq<TValue>& a, const TCollection& b) {
					return a.concat(b);
				});
			}

			template <typename TFunc>
			auto group_by(const TFunc& f) const
				-> Linq<zip_pair<std::result_of_t<decltype(f)(TElement)>, Linq<TElement>>> {
				using TKey = std::result_of_t<decltype(f)(TElement)>;
				using TValueVector = std::vector<TElement>;
				using TValueVectorPtr = std::shared_ptr<TValueVector>;

				std::map<TKey, TValueVectorPtr> map;
				for (auto it = m_begin; it != m_end; ++it) {
					auto value = *it;
					auto key = f(value);
					auto it2 = map.find(key);
					if (it2 == map.end()) {
						auto xs = std::make_shared<TValueVector>();
						xs->push_back(value);
					}
					else
						it2->second->push_back(value);
				}

				auto result = std::make_shared<std::vector<zip_pair<TKey, Linq<TElement>>>>();
				for (auto p : map)
					result->push_back(zip_pair<TKey, Linq<TElement>>(p.first, from_values(p.second)));
				return from_values(result);
			}

			template <typename TIt2, typename TFunc1, typename TFunc2>
			auto full_join_(const LinqEnumerable<TIt2>& e, const TFunc1& f1, const TFunc2& f2) const
				-> Linq<join_pair<
					std::remove_reference_t<std::result_of_t<decltype(f1)(TElement)>>,
					Linq<std::remove_cv_t<std::remove_reference_t<deference_type<TIt>>>>,
					Linq<std::remove_cv_t<std::remove_reference_t<deference_type<TIt2>>>>
				>> {
				using TKey = std::remove_reference_t<std::result_of_t<decltype(f1)(TElement)>>;
				using TV1 = std::remove_cv_t<std::remove_reference_t<deference_type<TIt>>>;
				using TV2 = std::remove_cv_t<std::remove_reference_t<deference_type<TIt2>>>;
				using TJoin = join_pair<TKey, Linq<TV1>, Linq<TV2>>;
				std::multimap<TKey, TV1> map1;
				std::multimap<TKey, TV2> map2;

				for (auto it = m_begin; it != m_end; ++it) {
					auto value = *it;
					auto key = f1(value);
					map1.insert(std::make_pair(key, value));
				}
				for (auto it = e.begin(); it != e.end(); ++it) {
					auto value = *it;
					auto key = f2(value);
					map2.insert(std::make_pair(key, value));
				}

				auto result = std::make_shared<std::vector<TJoin>>();
				auto lower1 = map1.begin();
				auto lower2 = map2.begin();
				while (lower1 != map1.end() && lower2 != map2.end()) {
					auto key1 = lower1->first;
					auto key2 = lower2->first;
					auto upper1 = map1.upper_bound(key1);
					auto upper2 = map2.upper_bound(key2);
					if (key1 < key2) {
						auto outers = std::make_shared<std::vector<TV1>>();
						for (auto it = lower1; it != upper1; ++it)
							outers->push_back(it->second);
						result->push_back(TJoin({ key1, { from_values(outers), from_empty<TV2>() } }));
						lower1 = upper1;
					}
					else if (key1 > key2) {
						auto inners = std::make_shared<std::vector<TV2>>();
						for (auto it = lower2; it != upper2; ++it)
							inners->push_back(it->second);
						result->push_back(TJoin({ key2, { from_empty<TV1>(), from_values(inners) } }));
						lower2 = upper2;
					}
					else {
						auto outers = std::make_shared<std::vector<TV1>>();
						for (auto it = lower1; it != upper1; ++it)
							outers->push_back(it->second);
						auto inners = std::make_shared<std::vector<TV2>>();
						for (auto it = lower2; it != upper2; ++it)
							inners->push_back(it->second);
						result->push_back(TJoin({ key1,{ from_values(outers), from_values(inners) } }));
						lower2 = upper2;
						lower1 = upper1;
					}
				}
				return from_values(result);
			}

			SN_LINQ_SUPPORT_STL_CONTAINER_EX(
				full_join,
				MACRO_EXPAND(typename TFunc1, typename TFunc2),
				MACRO_EXPAND(const TFunc1& f1, const TFunc2& f2),
				MACRO_EXPAND(f1, f2)
			)

			template <typename TIt2, typename TFunc1, typename TFunc2>
			auto group_join_(const LinqEnumerable<TIt2>& e, const TFunc1& f1, const TFunc2& f2) const 
				-> Linq<join_pair<
					std::remove_reference_t<std::result_of_t<decltype(f1)(TElement)>>,
					std::remove_cv_t<std::remove_reference_t<deference_type<TIt>>>,
					Linq<std::remove_cv_t<std::remove_reference_t<deference_type<TIt2>>>>
				>> {
				using TKey = std::remove_reference_t<std::result_of_t<decltype(f1)(TElement)>>;
				using TV1 = std::remove_cv_t<std::remove_reference_t<deference_type<TIt>>>;
				using TV2 = std::remove_cv_t<std::remove_reference_t<deference_type<TIt2>>>;
				using TJoin = join_pair<TKey, Linq<TV1>, Linq<TV2>>;
				using TGroup = join_pair<TKey, TV1, Linq<TV2>>;

				auto f = full_join(e, f1, f2);
				auto g = f.select_many([](const TJoin& item) -> Linq<TGroup> {
					Linq<TV1> outers = item.m_second.m_first;
					return outers.select([item](const TV1& outer) -> TGroup {
						return TGroup({ item.m_first, {outer, item.m_second.m_second} });
					});
				});
				return g;
			}

			SN_LINQ_SUPPORT_STL_CONTAINER_EX(
				group_join, 
				MACRO_EXPAND(typename TFunc1, typename TFunc2),
				MACRO_EXPAND(const TFunc1& f1, const TFunc2& f2),
				MACRO_EXPAND(f1, f2)
			)

			template <typename TIt2, typename TFunc1, typename TFunc2>
			auto join_(const LinqEnumerable<TIt2>& e, const TFunc1& f1, const TFunc2& f2) const
				->Linq<join_pair<
					std::remove_reference_t<std::result_of_t<decltype(f1)(TElement)>>,
					std::remove_cv_t<std::remove_reference_t<deference_type<TIt>>>,
					std::remove_cv_t<std::remove_reference_t<deference_type<TIt2>>>
				>> {
				using TKey = std::remove_reference_t<std::result_of_t<decltype(f1)(TElement)>>;
				using TV1 = std::remove_cv_t<std::remove_reference_t<deference_type<TIt>>>;
				using TV2 = std::remove_cv_t<std::remove_reference_t<deference_type<TIt2>>>;
				using TGroup = join_pair<TKey, TV1, Linq<TV2>>;
				using TJ = join_pair<TKey, TV1, TV2>;

				auto g = group_join(e, f1, f2);
				auto j = g.select_many([](const TGroup& item) -> Linq<TJ> {
					Linq<TV2> inners = item.m_second.m_second;
					return inners.select([item](const TV2& inner) -> TJ {
						return TJ({ item.m_first, {item.m_second.m_first, inner} });
					});
				});
				return j;
			}

			SN_LINQ_SUPPORT_STL_CONTAINER_EX(
				join,
				MACRO_EXPAND(typename TFunc1, typename TFunc2),
				MACRO_EXPAND(const TFunc1& f1, const TFunc2& f2),
				MACRO_EXPAND(f1, f2)
			)

			template <typename TFunc>
			auto first_order_by(const TFunc& f) const
				-> Linq<Linq<TElement>> {
				using TKey = std::remove_reference_t<std::result_of_t<decltype(f)(TElement)>>;
				return group_by(f).select([](const zip_pair<TKey, Linq<TElement>>& p{
					return p.m_second;
				}));
			}

			template <typename TFunc>
			auto then_order_by(const TFunc& f) const 
				-> Linq<deference_type<TIt>> {
				return select_many([f](const TElement& values) {
					return values.first_order_by(f);
				});
			}

			template <typename TFunc>
			auto order_by(const TFunc& f) const
				-> Linq<TElement> {
				return first_order_by(f).select_many([](const Linq<TElement>& values) {  // map is ordered by default
					return values;
				});
			}

			template <typename TIt2>
			LinqEnumerable<zip_iterator<TIt, TIt2>> zip_with_(const LinqEnumerable<TIt2>& e) const {
				return LinqEnumerable<zip_iterator<TIt, TIt2>>(
					zip_iterator<TIt, TIt2>(m_begin, m_end, e.begin(), e.end()),
					zip_iterator<TIt, TIt2>(m_end, m_end, e.end(), e.end()),
				);
			}

			SN_LINQ_SUPPORT_STL_CONTAINER(zip_with)

			std::vector<TElement> to_vector() const {
				std::vector<TElement> container;
				for (auto it = m_begin; it != m_end; ++it)
					container.push_back(*it);
				return std::move(container);
			}

			std::list<TElement> to_list() const {
				std::list<TElement> container;
				for (auto it = m_begin; it != m_end; ++it)
					container.push_back(*it);
				return std::move(container);
			}

			std::set<TElement> to_set() const {
				std::set<TElement> container;
				for (auto it = m_begin; it != m_end; ++it)
					container.insert(*it);
				return std::move(container);
			}

			template <typename TFunc>
			std::map<std::result_of_t<decltype(f)(TElement)>, TElement> to_map(const TFunc& f) const {
				std::map<std::result_of_t<decltype(f)(TElement)>, TElement> container;
				for (auto it = m_begin; it != m_end; ++it)
					container.insert(std::make_pair(f(*it), *it));
				return std::move(container);
			}

#undef SN_LINQ_SUPPORT_STL_CONTAINER
#undef SN_LINQ_SUPPORT_STL_CONTAINER_EX
#undef SN_LINQ_SUPPORT_ITERATOR_FUNC

		};

		template <typename T>
		class Linq : public LinqEnumerable<hide_type_iterator<T>> {
		public:
			Linq() {}
			template <typename TIt>
			Linq(const LinqEnumerable<TIt>& e)
				: LinqEnumerable<hide_type_iterator<T>>(hide_type_iterator(e.begin()), hide_type_iterator(e.end())) {}
		};

		template <typename T>
		static linq::Linq<T> flatten(const linq::Linq<linq::Linq<T>>& xs) {
			return xs.select_many([](const Linq<T>& ys) -> Linq<T> {
				return ys;
			});
		}

		template <typename TIt>
		linq::LinqEnumerable<TIt> from(const TIt& begin, const TIt& end) {
			return linq::LinqEnumerable<TIt>(begin, end);
		}

		template <typename T>
		auto from(const T& container)
			-> linq::LinqEnumerable<decltype(std::begin(container))> {
			return linq::LinqEnumerable<decltype(std::begin(container))>(std::begin(container), std::end(container));
		}

	}

	using linq::from;
	using linq::flatten;

}






#endif