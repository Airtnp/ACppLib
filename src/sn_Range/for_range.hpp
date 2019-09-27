#include "../sn_CommonHeader.h"

namespace sn_Range {
    
    namespace for_range{
		template <std::size_t N>
		struct StaticForRange {};

			template <typename T>
			struct FakeIt {
				T n;
				bool operator!=(const FakeIt& rhs) {
					return n != rhs.n;
				}
				const FakeIt& operator++() {
					++n;
					return *this;
				}
				T operator*() {
					return n;
				}
			};


		template <std::size_t N>
		FakeIt<std::size_t> begin(StaticForRange<N>& fr) {
			return { 0 };
		}

		template <std::size_t N>
        FakeIt<std::size_t> end(StaticForRange<N>& fr) {
			return { N };
		}

		template <std::size_t N>
		using SFR = StaticForRange<N>;

		template <typename OT>
		struct ForRange {
			using T = std::decay_t<OT>;
        	static_assert(std::is_integral<T>::value, "T must be a integral type.");
			const T begin_;
			const T end_;
			ForRange(T m, T n) : begin_(m), end_(n) {}
			ForRange(T n) : begin_(0), end_(n) {}
			FakeIt<T> end() const noexcept {
				return { end_ };
			}
			FakeIt<T> begin() const noexcept {
				return { begin_ };
			}
		};

		template <typename T>
		FakeIt<T> begin(ForRange<T>& fr) {
			return { fr.begin() };
		}

		template <typename T>
		FakeIt<T> end(ForRange<T>& fr) {
			return { fr.end() };
		}

		// That's make-function for ForRange
		// TODO: add constructible/common_type/integral check
		// Concept: ++/<
		// Note: C++17 can deduct from ctor (so detail::ForRange(10) is valid)
		//       However, this is invalid for alias template (Alias templates are never deduced)
		// ref: https://stackoverflow.com/questions/41008092/class-template-argument-deduction-not-working-with-alias-template
		template <typename T, typename U>
		ForRange<T> FR(T m, U n) {
			return ForRange<T>(m, n);   
		}

		template <typename T>
		ForRange<T> FR(T n) {
			return ForRange<T>(n);   
		}

	}
}