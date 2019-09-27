#include "../sn_CommonHeader.h"

namespace sn_Range {
    namespace span {
        template <typename It>
        class Span {
        public:
            using iterator_category = typename std::iterator_traits<It>::iterator_category;
            using value_type = typename std::iterator_traits<It>::value_type;
            using difference_type = typename std::iterator_traits<It>::difference_type;
            using reference = typename std::iterator_traits<It>::reference;
            using pointer = typename std::iterator_traits<It>::pointer;

            constexpr Span(It begin, It end)
                : m_begin(begin), m_end(end) {}
            template <typename S>
            constexpr explicit Span(S&& sp)
                : m_begin(std::begin(sp)), m_end(std::end(sp)) {}
            constexpr It begin() const {
                return m_begin;
            }
            constexpr It end() const {
                return m_end;
            }
            constexpr reference back() const {
                // assert(!empty()); // debug assert
                return *std::prev(m_end);
            }
            constexpr reference operator[](difference_type idx) const {
                // assert(idx < size()); // debug assert
                return *std::next(m_begin, idx);
            }
            constexpr bool empty() const {
                return m_begin == m_end;
            }
            constexpr difference_type size() const {
                return std::distance(m_begin, m_end);
            }
            Span& pop_front() {
                return pop_front(1);
            }
            Span& pop_front(difference_type n) {
                // assert(size() >= n); // debug assert
                std::advance(m_begin, n);
                return *this;
            }
            Span& pop_front_upto(difference_type n) {
                return pop_front(size() - n);
            }
            Span& pop_back() {
                return pop_back(1);
            }
            Span& pop_back(difference_type n) {
                // assert(size() >= n); // debug assert
                std::advance(m_end, -n);
                return *this;
            }
            Span& pop_back_upto(difference_type n) {
                return pop_back(size() - n);
            }

            std::pair<Span, Span> split(difference_type idx) const {
                // assert(idx < size()) // debug assert
                auto&& mid = std::next(m_begin, idx);
                return std::make_pair(Span(m_begin, mid), Span(mid, m_end));
            }

            Span slice(difference_type beg, difference_type ed) const {
                // assert(ed >= beg);
                // assert(ed - beg <= size());
                return Span(std::next(m_begin, beg), std::next(m_begin, ed));
            }

            Span slice(difference_type beg) const {
                return slice(beg, size() - beg);         
            }
        private:
            It m_begin;
            It m_end;
        };

        template <typename It>
        class SpanIterator {
        public:
            using iterator_category = typename std::iterator_traits<It>::iterator_category;
            using value_type = typename std::iterator_traits<It>::value_type;
            using difference_type = typename std::iterator_traits<It>::difference_type;
            using reference = typename std::iterator_traits<It>::reference;
            using pointer = typename std::iterator_traits<It>::pointer;

            constexpr explicit SpanIterator(It iter)
                : m_iter(iter) {}
            constexpr operator pointer() const {
                return std::addressof(*m_iter);
            }
            decltype(auto) operator*() const {
                return *m_iter;
            }
            SpanIterator& operator++() & {
                return *this += 1;
            }
            SpanIterator& operator--() & {
                return *this -= 1;
            }
            SpanIterator operator+(difference_type n) const {
                return SpanIterator(std::next(m_iter, n));
            }
            SpanIterator operator-(difference_type n) const {
                return SpanIterator(std::prev(m_iter, n));
            }
            SpanIterator& operator+=(difference_type n) & {
                std::advance(m_iter, n);
                return *this;
            }
            SpanIterator& operator-=(difference_type n) & {
                std::advance(m_iter, -n);
                return *this;
            }
            decltype(auto) operator[](difference_type n) const {
                return *std::next(m_iter, n);
            }
            bool operator<=(const SpanIterator& rhs) const {
                return std::distance(m_iter, rhs.m_iter) <= 0;
            }
            bool operator>=(const SpanIterator& rhs) const {
                return std::distance(m_iter, rhs.m_iter) >= 0;
            }
            bool operator==(const SpanIterator& rhs) const {
                return m_iter == rhs.m_iter;
            }
            bool operator!=(const SpanIterator& rhs) const {
                return !(m_iter == rhs.m_iter);
            }
            bool operator<(const SpanIterator& rhs) const {
                return !(*this >= rhs);
            }
            bool operator>(const SpanIterator& rhs) const {
                return !(*this <= rhs);
            }
        private:
            It m_iter;
        };

        template <typename It>
        constexpr auto make_span(It beg, It ed) {
            return Span<It>(beg, ed);
        }

        template <typename S>
        constexpr auto make_span(S&& sp) {
            return Span<decltype(std::begin(sp))>(sp);
        }

        template <typename S>
        constexpr auto make_span_iter(S&& sp) {
            return Span<SpanIterator<decltype(std::begin(sp))>>(sp);
        }

    }
}