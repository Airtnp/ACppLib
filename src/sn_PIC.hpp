#ifndef SN_PI_CALCULUS_H
#define SN_PI_CALCULUS_H

#include "sn_CommonHeader.h"
#include "sn_Assist.hpp"

// ref: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4464.pdf
// ref: https://github.com/cleitonsantoia/concurrency/tree/master/pi-calculus
// ref: https://en.wikipedia.org/wiki/%CE%A0-calculus
// TODO: add polyadic/bisimulation
// TODO: add reduction
namespace sn_PIC {
	using sn_Assist::sn_function_traits::function_traits;

	// 0
	struct Nil {};
	constexpr Nil nil;

	// !Q
	class Replication {
	public:
		Replication(const std::function<void()>& func) : m_func(func) {}
		void operator()() {
			while (1) {
				m_func();
			}
		}
	private:
		std::function<void()> m_func;
	};

	class Process {
	public:
		Process() {}
		Process(const Nil&) : m_func([](){}) {}
		Process(const std::size_t n) : m_func([]() {}) {
			if (n != 0)
				throw std::logic_error("Nonzero number cannot represent pi-calculus elements");
		}
		Process(const Process&) = default;
		Process& operator=(const Process&) = default;
		Process(Process&& rhs) : m_func(std::move(rhs.m_func)) {}

		template <typename F>
		Process(const F& func) : m_func(func) {}
		Process(const std::function<void()>& func) : m_func(func) {}
		template <typename T>
		Process(void(T::*func)(), T& obj) : m_func(std::bind(func, &obj)) {}

		void operator()() {
			m_func();
		}
		Process operator!() {
			return Replication(m_func);
		}

		void go() {
			std::thread(*this).join();
		}
	private:
		std::function<void()> m_func;
	};

	// Var free/bound names
	template <typename T>
	class Var {
	private:
		mutable std::recursive_mutex m_mtx;
		std::shared_ptr<std::shared_ptr<T>> m_ref;

		template <typename U>
		void send_to(Var<U>& u) { u = *this; }
		template <typename U>
		void receive_from(const Var<U>& u) { *this = u; }

		template <typename Tp>
		struct has_function_call_op {
			template <typename U, Process(U::*)(Var<Tp>&)>
			struct SFINAE {};
			template <typename U>
			static char Test(SFINAE<U, &U::operator()>*);
			template <typename U>
			static int Test(...);
			static const bool value = sizeof(Test<Tp>(0) == sizeof(char));
		};

		template <typename Tp>
		struct has_array_op {
			template <typename U, Process(U::*)(Var<Tp>&)>
			struct SFINAE {};
			template <typename U>
			static char Test(SFINAE<U, &U::operator[]>*);
			template <typename U>
			static int Test(...);
			static const bool value = sizeof(Test<Tp>(0) == sizeof(char));
		};

	public:
		Var() : Var(T()) {}
		Var(const Var& rhs) : m_ref(rhs.m_ref) {}
		Var(const T& val) {
			m_ref.reset(new std::shared_ptr<T>());
			m_ref->reset(new T(val));
		}

		T& operator*() {
			std::lock_guard<std::recursive_mutex> lock(m_mtx);
			return *(*m_ref);
		}
		T* operator->() {
			std::lock_guard<std::recursive_mutex> lock(m_mtx);
			return (*m_ref).get();
		}
		T& operator=(const T& val) {
			std::lock_guard<std::recursive_mutex> lock(m_mtx);
			*(*this) = val;
			return *(*this);
		}

		void make_local_construct_process() {
			std::lock_guard<std::recursive_mutex> lock(m_mtx);
			m_ref.reset(new std::shared_ptr<T>());
			m_ref->reset(new T());
		}
		void make_local_exec_process() {
			std::lock_guard<std::recursive_mutex> lock(m_mtx);
			m_ref->reset(new T());
		}

		template <typename U>
		Process operator()(const Var<U>& var) {
			struct has_op {
				static Process build_process(Var<T>& var, const Var<U>& param) {
					return Process([&]() { var(param); });
				}
			};
			struct not_has_op {
				static Process build_process(Var<T>& var, const Var<U>& param) {
					return Process([&]() { *var = *param; });
				}
			};

			return typename std::conditional_t<has_function_call_op<T>::value, has_op, not_has_op>::build_process(*this, var);
		}

		template <typename U>
		Process operator()(const U& val) {
			return Process([&]() { *this = val; });
		}

		template <typename U>
		Process operator[](const Var<U>& var) {
			struct has_op {
				static Process build_process(Var<T>& var, const Var<U>& param) {
					return Process([&]() { var[param]; });
				}
			};
			struct not_has_op {
				static Process build_process(Var<T>& var, const Var<U>& param) {
					return Process([&]() { param = var; });
				}
			};

			return typename std::conditional_t<has_array_op<T>::value, has_op, not_has_op>::build_process(*this, var);
		}

		template <typename U>
		Process operator[](const U& val) {
			return Process([&]() { val = *(*this); });
		}
	};

	template <>
	class Var<void> {
	public:
		Var() {}
		void make_local_construct_process() {}
		void make_local_exec_process() {}
	};

	// Binding(ref) vars -> v{e} * ....
	// vx
	class v {
		class VarWrapperRef {
			struct Base {
				virtual void make_local_construct_process() = 0;
				virtual void make_local_exec_process() = 0;
			};
			template <typename T>
			class Impl : public Base {
				Var<T> m_var;
			public:
				Impl(const Var<T>& var) : m_var(var) {}
				void make_local_construct_process() { m_var.make_local_construct_process(); }
				void make_local_exec_process() { m_var.make_local_exec_process(); }
			};

			std::shared_ptr<Base> m_pimpl;
		public:
			template <typename T>
			VarWrapperRef(const Var<T>& var) : m_pimpl(std::make_shared<Impl<T>>(var)) {}
			VarWrapperRef() = default;
			VarWrapperRef(const VarWrapperRef& ref) = default;
			VarWrapperRef& operator=(const VarWrapperRef&) = default;

			void make_local_construct_process() { m_pimpl->make_local_construct_process(); }
			void make_local_exec_process() { m_pimpl->make_local_exec_process(); }
		};

		std::vector<VarWrapperRef> m_vars;
	public:
		v(const std::initializer_list<VarWrapperRef>& vars) : m_vars(vars) {
			for (auto& var : m_vars)
				var.make_local_construct_process();
		}
		void operator()() {
			for (auto& var : m_vars)
				var.make_local_exec_process();
		}
	};

	struct Stop {
		void operator()() {};
	};

	// Q * R
	class Sequence {
		std::vector<Process> m_processes;
	public:
		Sequence() {};
		Sequence(const std::initializer_list<Process>& p) : m_processes(p) {}
		void operator()() {
			for (auto& p : m_processes)
				p();
		}
		Sequence& operator*(const Process& p) {
			m_processes.push_back(p);
			return *this;
		}
	};

	Sequence operator*(const v&, const Process& p2) {
		return Sequence({ p2 });
	}

	Sequence operator*(const Process& p1, const v&) {
		return Sequence({ p1 });
	}

	Sequence operator*(const Process& p1, const Process& p2) {
		return Sequence({ p1, p2 });
	}

	// Q | R
	class Parallel {
		std::vector<Process> m_processes;
	public:
		Parallel(const std::initializer_list<Process>& p) : m_processes(p) {}
		Parallel(Process p, std::size_t num) : m_processes(num, p) {}
		Parallel& operator|(const Process& p) {
			m_processes.push_back(p);
			return *this;
		}
		Parallel& operator|(const Parallel& p) {
			m_processes.insert(m_processes.end(), p.m_processes.begin(), p.m_processes.end());
			return *this;
		}
		void Parallel::operator()() {
			std::vector<std::thread> threads;
			for (auto& p : m_processes) {
				threads.emplace_back(&Process::operator(), p);
			}
			for (auto& t : threads) {
				if (t.joinable()) {
					t.join();
				}
			}
		}
	};


	Parallel operator|(const Process& p1, const Process& p2) {
		return Parallel({ p1, p2 });
	}

	Parallel operator^(const Process& p1, int num) {
		return Parallel(p1, num);
	}

	class Choice {};

	// Q + R
	// If(f).Then(p)[.Else(p)]
	class If {
		class ElseT {
		protected:
			Process m_then;
			Process m_else;
			std::function<bool()> m_pred;
		public:
			ElseT(const std::function<bool()>& pred, const Process& then, const Process& elsep)
				: m_then(then), m_else(elsep), m_pred(pred) {}
			void operator()() {
				if (m_pred())
					m_then();
				else
					m_else();
			}
		};
		class ThenT {
		protected:
			Process m_then;
			std::function<bool()> m_pred;
		public:
			ThenT(const std::function<bool()>& pred, const Process& then)
				: m_then(then), m_pred(pred) {}
			void operator()() {
				if (m_pred())
					m_then();
			}
			ElseT Else(const Process& p) {
				return ElseT(m_pred, m_then, p);
			}
		};
		std::function<bool()> m_pred;
	public:
		If(const std::function<bool()>& p) : m_pred(p) {}
		ThenT Then(const Process& p) {
			return ThenT(m_pred, p);
		}
	};

	template <typename T>
	class In {
		std::function<T()> m_func;
		Var<T> m_var;
	public:
		In(const std::function<T()>& func, Var<T>& var) : m_func(func), m_var(var) {}
		template <typename U>
		In(T(U::*func)(), U& obj, Var<T> var) : m_func(std::bind(func, &obj)), m_var(var) {}
		void operator()() {
			m_var = m_func();
		}
	};

	// x(z) input z receive
	template <typename T>
	class IChannel {
		std::function<T()> m_func;
	public:
		IChannel(const std::function<T()>& func) : m_func(func) {}
		template <typename U>
		Process operator()(Var<U>& var) {
			return Process(In<T>(m_func, var));
		}
	};

	template <typename T>
	class Out {
		std::function<void(const T&)> m_func;
		Var<T> m_var;
	public:
		Out(const std::function<void(const T&)>& func, Var<T>& var) : m_func(func), m_var(var) {}
		template <typename U>
		Out(void(U::*func)(const T&), U& obj, Var<T> var) : m_func(std::bind(func, &obj, std::placeholders::_1)), m_var(var) {}
		void operator()() {
			m_func(*m_var);
		}
	};

	// \tlide(x)(z) output z (x[z]) send
	template <typename T>
	class OChannel {
		std::function<const T&> m_func;
	public:
		OChannel(const std::function<const T&>& func) : m_func(func) {}
		template <typename U>
		Process operator[](Var<U>& var) {
			return Process(Out<T>(m_func, var));
		}
	};

	template<typename I, typename O = I>
	class Sync {
		std::mutex m_mtx;
		std::condition_variable m_notEmpty;
		std::condition_variable m_notFull;

		I pop() {
			std::unique_lock<std::mutex> mlock(m_mtx);
			while (!m_element)
				m_notEmpty.wait(mlock);
			I result = m_func(*m_element);
			m_element = nullptr;
			m_notFull.notify_one();
			return result;
		}

		void push(const O& item) {
			std::unique_lock<std::mutex> mlock(m_mtx);
			while (m_element)
				m_notFull.wait(mlock);
			m_element = &item;
			m_notEmpty.notify_one();
		}

		std::function< I(const O&)> m_func;
		const O* m_element = nullptr;
	public:
		template<typename C>
		Sync(const C& func) : m_func(func) {}
		Sync() : m_func([](const O& input) -> I { return input; }) {}
		Process operator()(Var<I>& var) {
			return Process(In<I>(&Sync<I, O>::pop, *this, var)); 
		}
		Process operator[](const Var<O>& var) { 
			return Process(Out<O>(&Sync<I, O>::push, *this, var)); 
		}
	};

	namespace util {
		template <typename T>
		class Queue {
			using container_type = std::list<T>;
			using value_type = typename container_type::value_type;
			container_type m_queue;
			std::mutex m_mtx;
			std::condition_variable m_notEmpty;
			std::condition_variable m_notFull;
			std::atomic<std::size_t> m_max;
		public:
			value_type pop() {
				std::unique_lock<std::mutex> mlock(m_mtx);
				while (m_queue.empty())
					m_notEmpty.wait(mlock);
				value_type result = m_queue.front();
				m_queue.pop_front();
				mlock.unlock();
				m_notFull.notify_one();
				return result;
			}
			value_type push(const value_type& item) {
				std::unique_lock<std::mutex> mlock(m_mtx);
				while (size() >= m_max)
					m_notFull.wait(mlock);
				m_queue.push_back(item);
				mlock.unlock();
				m_notEmpty.notify_one();
				return result;
			}
			std::size_t size() const {
				return m_queue.size();
			}

			Queue(std::size_t max = 10) : m_max(max) {}
			Queue(const Queue<T>& q) : m_queue(q.m_queue) {}
			Queue& operator=(const Queue<T>& q) {
				m_queue = q.m_queue;
				return *this;
			}
			Process operator()(Var<value_type>& var) {
				return Process(In<value_type>(&Queue<value_type>::pop, *this, var));
			}
			Process operator[](Var<value_type>& var) {
				return Process(Out<value_type>(&Queue<value_type>::push, *this, var));
			}
		};
	}

}









#endif