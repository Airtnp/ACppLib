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
		// or just use
		// std::chrono::system_clock::now()
		// and
		// static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count());
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
				return std::chrono::duration_cast<std::chrono::nanoseconds>(t.time_since_epoch());
			}

		};

		template <typename T>
		std::chrono::time_point<std::chrono::steady_clock> profile<T>::t{ 1 };
		template <typename T>
		std::chrono::time_point<std::chrono::steady_clock> profile<T>::s;

		template <typename Traits>
		class basic_timer {
			using tm_t = typename Traits::time_type;
			using diff_t = typename Traits::difference_type
			tm_t m_start;
			tm_t m_stop;
			inline static tm_t now() {
				return Traits::get_time();
			}
			double elapsed(const tm_t end) const {
				static const tm_t freq = Traits::get_freq();
				return static_cast<double>(static_cast<diff_t>(end - m_start)) / freq;
			}
		public:
			using time_type = tm_t;
			using difference_type = diff_t;
			basic_timer()
				: start_() {}
			difference_type lap() const { return now() - m_start; }
			time_type start() { return m_start = now(); }
			difference_type stop() { return (m_stop = now()) - m_start; }
			difference_type interval() const { return m_stop - m_start; }
			double as_seconds() const { return elapsed(m_stop); }
			double elapsed() const { return elapsed(now()); }
		};

		// #include <ctime>
		struct clock_time_traits {
			using time_type = size_t;
			using difference_type = ptrdiff_t;
			static time_type get_time() {
				time_t t;
				return std::time(&t);
			}
			static time_type get_freq() {
				return 1;
			}
		};

		struct cpu_time_traits {
			using time_type = size_t;
			using difference_type = ptrdiff_t;
			static time_type get_time() {
				return std::clock();
			}
			static time_type get_freq() {
				return CLOCKS_PER_SEC;
			}
		};

#ifdef _WIN32
		// #include <windows.h>
		struct windows_clock_time_traits {
			using time_type = ULONGLONG;
			using difference_type = LONGLONG;
			static time_type get_time() {
				LARGE_INTEGER i;
				QueryPerformanceCounter(&i);
				return i.QuadPart;
			}
			static time_type get_freq() {
				LARGE_INTEGER value;
				QueryPerformanceFrequency(&value);
				return value.QuadPart;
			}
		};
		using platform_clock_traits = windows_clock_time_traits;
#elif defined(__APPLE__)
		// #include <sys/time.h>
		struct macosx_clock_time_traits {
			using time_type = uint64_t;
			using difference_type = int64_t;
			static time_type get_time() {
				timeval now;
				gettimeofday(&now, 0);
				return static_cast<time_type>(now.tv_sec) * get_freq() + now.tv_usec;
			}
			static time_type get_freq() {
				return 1000000;
			}
		};
		using platform_clock_traits = macosx_clock_time_traits;
#endif
	}

	namespace dummy {
		struct dummy_test {
			int v;
			// dummy_test() { std::cout << std::addressof(*this) << " default ctor.\n"; }
			dummy_test(int xv = 0): v(xv) { std::cout << std::addressof(*this) <<  " parameter ctor.\n"; }
			dummy_test(const dummy_test& rhs): v(rhs.v) { std::cout << std::addressof(*this) << " copy ctor.\n"; }
			dummy_test(dummy_test&& rhs): v(std::move(rhs.v)) { std::cout << std::addressof(*this) << " move ctor.\n"; }
			dummy_test& operator=(const dummy_test& rhs) { v = rhs.v; std::cout << std::addressof(*this) << " copy operator.\n"; return *this; }
			dummy_test& operator=(dummy_test&& rhs) { v = std::move(rhs.v); std::cout << std::addressof(*this) << " move operator.\n"; return *this; }
			~dummy_test() { std::cout << std::addressof(*this) << " dtor.\n"; }
		};
		// std::cout << "---\n";
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