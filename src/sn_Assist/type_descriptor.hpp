#include "../sn_CommonHeader.h"

namespace sn_Assist {

	namespace sn_demangle {
		template <typename T>
		std::string demangle_type() {
			using TR = typename std::decay<T>::type;
			std::unique_ptr<char, void(*)(void*)> own;
				(
			#ifndef _MSC_VER
				abi::__cxa_demangle(typeid(TR).name(), nullptr, nullptr, nullptr),
			#else
				nullptr,
			#endif
				std::free
				);
			std::string r = own != nullptr ? own.get() : typeid(TR).name();
			if (std::is_const<T>::value)
				r += " const";
			if (std::is_volatile<T>::value)
				r += " volatile";
			if (std::is_lvalue_reference<T>::value)
				r += "&";
			if (std::is_rvalue_reference<T>::value)
				r += "&&";
			return r;
		}
	}


    //ref: https://zhuanlan.zhihu.com/p/24651043
	//ref: https://www.zhihu.com/question/56399162
	//alternative: TCPL Ch. 3 dcl
	namespace sn_type_descriptor
	{
		using std::string;

		template <class ...Args>
		struct type_descriptor {
			static string descript() {
				std::string tmp = "[unknown type, maybe \"";
				std::initializer_list<int>{(tmp = tmp + sn_demangle::demangle_type<Args>() + " ", 0)...};
				tmp += "\"]";
				return tmp;
			}
		};

		template <>
		struct type_descriptor<> {
			static string descript() {
				return "void";
			}
		};

#define sn_type_descriptor_generator(type_name) \
	template <> \
	struct type_descriptor<type_name> { \
		static string descript() { \
			return #type_name; \
		} \
	};

		sn_type_descriptor_generator(long long int)

		template <>
		struct type_descriptor<void> {
			static string descript() {
				return "void";
			}
		};

		template <>
		struct type_descriptor<char> {
			static string descript() {
				return "char";
			}
		};

		template <>
		struct type_descriptor<wchar_t> {
			static string descript() {
				return "wchar_t";
			}
		};

		template <>
		struct type_descriptor<bool> {
			static string descript() {
				return "bool";
			}
		};

		template <>
		struct type_descriptor<short> {
			static string descript() {
				return "short";
			}
		};

		template <>
		struct type_descriptor<int> {
			static string descript() {
				return "int";
			}
		};

		template <>
		struct type_descriptor<long> {
			static string descript() {
				return "long";
			}
		};

		template <>
		struct type_descriptor<float> {
			static string descript() {
				return "float";
			}
		};

		template <>
		struct type_descriptor<double> {
			static string descript() {
				return "double";
			}
		};

		template<class T1, class ...Args>
		struct type_descriptor<T1, Args...> {
			static string descript() {
				return type_descriptor<T1>::descript() +", " + type_descriptor<Args...>::descript();
			}
		};

		template<class F>
		struct type_descriptor<F()> {
			static string descript() {
				return "function () -> " + type_descriptor<F>::descript();
			}
		};

		template<class F, class ...Args>
		struct type_descriptor<F(Args...)> {
			static string descript() {
				return "function (" + type_descriptor<Args...>::descript()  +") -> " + type_descriptor<F>::descript();
			}
		};

		template<class T>
		struct type_descriptor<T&> {
			static string descript() {
				return "lvalue reference to " + type_descriptor<T>::descript();
			}
		};

		template<class T>
		struct type_descriptor<T&&> {
			static string descript() {
				return "rvalue reference to " + type_descriptor<T>::descript();
			}
		};

		template<class T>
		struct type_descriptor<T*> {
			static string descript() {
				return "pointer to " + type_descriptor<T>::descript();
			}
		};

		template<class R>
		struct type_descriptor<R(*)(void)> {
			static string descript() {
				return "function pointer to (function () -> " + type_descriptor<R>::descript() + ")";
			}
		};

		template<class R, class ...Args>
		struct type_descriptor<R(*)(Args...)> {
			static string descript() {
				return std::string("function pointer to ") + "(function (" + type_descriptor<Args...>::descript() + ") -> " + type_descriptor<R>::descript() + ")";
			}
		};

		template<class T>
		struct type_descriptor<T* const> {  //only match to top-level const, so const T* cannot be match, only T* const
			static string descript() {
				return "const pointer to " + type_descriptor<T>::descript();
			}
		};

		template<class T>
		struct type_descriptor<T* volatile> {
			static string descript() {
				return "volatile pointer to " + type_descriptor<T>::descript();
			}
		};

		template<class T>
		struct type_descriptor<T const> {
			static string descript() {
				return "const " + type_descriptor<T>::descript();
			}
		};

		template<class T>
		struct type_descriptor<T volatile> {
			static string descript() {
				return "volatile " + type_descriptor<T>::descript();
			}
		};

		template<class T>
		struct type_descriptor<T[]> {
			static string descript() {
				return "array of " + type_descriptor<T>::descript();
			}
		};
		using std::to_string;
		template<class T, size_t N>
		struct type_descriptor<T[N]> {
			static string descript() {
				return "array (size " + to_string(N) + ") of " + type_descriptor<T>::descript();
			}
		};

		template<class T, size_t N>
		struct type_descriptor<const T[N]> { //otherwise, top-level const and top-level array will both match
			static string descript() {
				return "array (size " + to_string(N) + ") of const " + type_descriptor<T>::descript();
			}
		};

		template<class T, size_t N>
		struct type_descriptor<volatile T[N]> {
			static string descript() {
				return "array (size " + to_string(N) + ") of volatile " + type_descriptor<T>::descript();
			}
		};

	}
}