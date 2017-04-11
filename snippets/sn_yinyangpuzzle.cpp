#ifdef SN_YINYANG_PUZZLE
#include <queue>
#include <iostream>
// #include <cxxabi.h>
#include <memory>
#include <cstring>


struct ControlFlow {
	template <typename F>
	auto callcc(F f) {
		return f([*this](){ return *this; });
	}

	struct Yin {
		void display() {
			std::cout << "@";
		}
		struct id {
			template <typename T>
			T operator()(T t) {
				return t;
			}
		};
		auto yin_callcc() {
			return pcf->callcc(id{});
		}
		void new_and_copy() {
			auto cf_lambda = yin_callcc();
			ControlFlow* pcf2 = new ControlFlow;
			auto ccf = cf_lambda();
			std::memcpy(pcf2, &ccf, sizeof(ccf));
			qpcf.push(pcf2);
		}
		void call() {
			display();
			new_and_copy();
		}
		template <typename T>
		void call(T* other) {
			auto pcf2 = qpcf.front();
			qpcf.pop();
			*pcf = *pcf2;
			qpcf.push(other->qpcf.front());
			pcf->seq -= 1;
		}
		ControlFlow* pcf;
		std::queue<ControlFlow*> qpcf;
	};

	struct Yang {
		void display() {
			std::cout << "*";
		}
		struct id {
			template <typename T>
			T operator()(T t) {
				return t;
			}
		};
		auto yang_callcc() {
			return pcf->callcc(id{});
		}
		void new_and_copy() {
			auto cf_lambda = yang_callcc();
			ControlFlow* pcf2 = new ControlFlow;
			auto ccf = cf_lambda();
			std::memcpy(pcf2, &ccf, sizeof(ccf));
			qpcf.push(pcf2);
		}
		void call() {
			display();
			new_and_copy();
		}
		template <typename T>
		void call(T* other) {
			auto pcf2 = qpcf.front();
			qpcf.pop();
			*pcf = *pcf2;
			qpcf.push(other->qpcf.front());
			pcf->seq -= 1;
		}
		ControlFlow* pcf;
		std::queue<ControlFlow*> qpcf;
	};
	void* yin;
	void* yang;
	int seq;
	ControlFlow() {
		yin = new Yin();
		yang = new Yang();
		reinterpret_cast<Yin*>(yin)->pcf = this;
		reinterpret_cast<Yang*>(yang)->pcf = this;
		seq = 0;
	}
	void call() {
		switch (seq) {
		case 0:
			reinterpret_cast<Yin*>(yin)->call();
			++seq;
			break;
		case 1:
			reinterpret_cast<Yang*>(yang)->call();
			++seq;
			break;
		case 2:
			reinterpret_cast<Yin*>(yin)->call(reinterpret_cast<Yang*>(yang));
			++seq;
			break;
		default:
			break;
		}
	}

};



int main() {
	ControlFlow cf;
	while (true) {
		cf.call();
	}
}

#endif