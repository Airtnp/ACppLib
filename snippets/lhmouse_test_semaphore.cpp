#include <iostream>
#include <thread>
#include <array>
#include <stdexcept>
#include <atomic>
#include <cstddef>
#include <cstdlib>

// #define NATIVE_COND 1

#ifdef NATIVE_COND
#  include <windows.h>
#else
#  include <mutex>
#  include <condition_variable>
#endif

class semaphore {
private:
#ifdef NATIVE_COND
	HANDLE x_cond;
#else
	std::mutex x_m;
	std::condition_variable x_cv;
	std::size_t x_n;
#endif

public:
	explicit semaphore(std::size_t n_init = 0)
#ifdef NATIVE_COND
		: x_cond(CreateSemaphoreW(nullptr, 0, LONG_MAX, nullptr))
#else
		: x_m(), x_cv(), x_n(n_init)
#endif
	{ }
	~semaphore(){
#ifdef NATIVE_COND
		CloseHandle(x_cond);
#else
		//
#endif
	}

public:
	void p(){
#ifdef NATIVE_COND
		WaitForSingleObject(x_cond, INFINITE);
#else
		std::unique_lock<std::mutex> lock(x_m);
		x_cv.wait(lock, [&]{ return x_n != 0; });
		--x_n;
#endif
	}
	void v(){
#ifdef NATIVE_COND
		ReleaseSemaphore(x_cond, 1, nullptr);
#else
		std::unique_lock<std::mutex> lock(x_m);
		if(x_n == static_cast<std::size_t>(-1)){
			std::abort();
		}
		++x_n;
		x_cv.notify_one();
#endif
	}
};

int main(){
	constexpr std::size_t n_threads = 4;
	constexpr std::size_t n_loops = 1000000;

	std::atomic<std::size_t> n_ping(0), n_pong(0);

	std::array<std::thread, n_threads> threads;
	semaphore sem_g;
	std::array<semaphore, n_threads> sems;
	for(std::size_t i = 0; i < n_threads; ++i){
		threads.at(i) = std::thread([&,i]{
			for(std::size_t j = 0; j < n_loops; ++j){
				sems.at(i).p();
				n_pong.fetch_add(1, std::memory_order_relaxed);
				sem_g.v();
			}
		});
	}
	const auto t1 = std::chrono::high_resolution_clock::now();
	for(std::size_t j = 0; j < n_loops; ++j){
		for(std::size_t i = 0; i < n_threads; ++i){
			n_ping.fetch_add(1, std::memory_order_relaxed);
			sems.at(i).v();
		}
		for(std::size_t i = 0; i < n_threads; ++i){
			sem_g.p();
		}
	}
	const auto t2 = std::chrono::high_resolution_clock::now();
	for(std::size_t i = 0; i < n_threads; ++i){
		threads.at(i).join();
	}

	std::cout <<"n_ping = " <<n_ping.load(std::memory_order_relaxed) <<std::endl
	          <<"n_pong = " <<n_pong.load(std::memory_order_relaxed) <<std::endl
	          <<"delta_milliseconds = " <<std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() <<std::endl;
}