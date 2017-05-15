#ifndef SN_TEST_H
#define SN_TEST_H

#include "sn_CommonHeader.h"
#include "sn_Log.hpp"
#include "sn_StdStream.hpp"

namespace sn_TEST {

#define SN_ASSERT(expr, msg) \
	assert(expr && msg)

#define SN_STATIC_ASSERT(cond) \
	((void)sizeof(char[1 - 2 * !!(cond)]))

#define SN_TESTCASE(Name) \
	extern void sn_test_case_function_##Name(void); \
	struct SN_TestCaseClass_##Name { \
		SN_TestCaseClass_##Name() { \
			SN_LOG_DEBUG << #Name << std::nl; \
			sn_test_case_function_##Name(); \
		} \
	} sn_test_case_instance_##Name; \
	void sn_test_case_function_##Name(void)

#define SN_TEST(Name) \
	sn_test_case_instance_##Name();

	// for clang/gcc use __builtin_expect
#define SN_LIKELY(cond) \
	(!!(cond))

#define SN_EXPECT(cond) \
	(SN_LIKELY(cond) ? static_cast<void>(0) : static_cast<void>(0);  SN_BASIC_LOG(std::cout, "Expect Failed."); std::terminate();)

#define SN_ENSURE(cond) \
	(SN_LIKELY(cond) ? static_cast<void>(0) : static_cast<void>(0);  SN_BASIC_LOG(std::cout, "Ensure Failed."); std::terminate();)

	namespace profile {
		template <typename T>
		struct profile {
			static std::chrono::time_point<std::chrono::steady_clock> t;
			static std::chrono::time_point<std::chrono::steady_clock> s;
			static void start() {
				s = std::chrono::high_resolution_clock::now();
			}
			static void finish() {
				const auto u = std::chrono::high_resolution_clock::now();
				t += u - s;
				s = u;
			}
			static void reset() {
				t = 0;
			}
			static std::chrono::nanoseconds report() {
				return t.time_since_epoch();
			}
		};

		template <typename T>
		std::chrono::time_point<std::chrono::steady_clock> profile<T>::t{ 1 };
		template <typename T>
		std::chrono::time_point<std::chrono::steady_clock> profile<T>::s;
	}

	namespace dummy {
		struct dummy_test {
			int v;
			dummy_test() { std::cout << std::addressof(*this) << " default ctor.\n"; }
			dummy_test(int xv): v(xv) { std::cout << std::addressof(*this) <<  " parameter ctor.\n"; }
			dummy_test(const dummy_test& rhs): v(rhs.v) { std::cout << std::addressof(*this) << " copy ctor.\n"; }
			dummy_test(dummy_test&& rhs): v(std::move(rhs.v)) { std::cout << std::addressof(*this) << " move ctor.\n"; }
			dummy_test& operator=(const dummy_test& rhs) { v = rhs.v; std::cout << std::addressof(*this) << " copy operator.\n"; }
			dummy_test& operator=(dummy_test&& rhs) { v = std::move(rhs.v); std::cout << std::addressof(*this) << " move operator.\n"; }
			~dummy_test() { std::cout << std::addressof(*this) << " dtor.\n"; }
		};
	}

	namespace error {
		template <class T, class E = std::error_condition>
		class expected {
		private:
			T t_;
			E e_;
			bool has_v = false;	
		public:
			using value_type = T;
			using error_type = E;

			constexpr expected(const T& t) : t_(t), has_v(true) {}
			constexpr expected(T&& t) : t_(std::move(t)), has_v(true) {}
			constexpr expected(const E& e) : e_(e), has_v(false) {}
			constexpr expected(E&& e) : e_(std::move(e)), has_v(false) {}
			
			constexpr T& value() {
				if (has_v) {
					return t_;
				}
				else {
					throw std::runtime_error("...");
				}
			}

			template <class U>
			constexpr T value_or(U&& u) {
				if (has_v) {
					return t_;
				}
				else {
					return std::forward<T>(u);
				}
			}


		}
	}

}







#endif