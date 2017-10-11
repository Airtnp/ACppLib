#ifndef SN_FUNCTION_Y_H
#define SN_FUNCTION_Y_H

#include <utility>
#include <functional>

namespace sn_Function {
    namespace YCombinator {
		// ref: https://gist.github.com/kenpusney/9274727

		// auto-version
		template <typename TF>
		class YBuilder {
		private:
			TF partial;
		public:
			YBuilder(TF partial_)
				: partial(std::forward<TF>(partial_)) {}	
			template <typename ...Args>
			decltype(auto) operator()(Args&&... args) {
				return partial(
					std::ref(*this),
					std::forward<Args>(args)...
				);
			}
		};

		template <typename R, typename ...Args>
		class YBuilder<R(Args...)> {
		private:
			using FT = std::function<R(std::function<R(Args...)>, Args...)>;
			std::function<R(std::function<R(Args...)>, Args...)> partial;
		public:
			YBuilder(FT partial_)
				: partial(std::forward<FT>(partial_)) {}
			
			R operator(Args&&... args) {
				return partial(
					[this](Args&&... args) {
						return this->operator()(std::forward<Args>(args)...);
					}, std::forward<Args>(args)...);
				)
			}
		};

		template <typename C>
		auto Y(C&& partial) {
			using Tr = sn_Assist::sn_function_traits::function_traits;
			using FT = typename Tr<C>::function_type;
			return YBuilder<FT>(std::forward<C>(partial));
		}

		// lambda with auto self
		template <typename C>
		auto YA(C&& partial) {
			return YBuilder<C>(std::forward<C>(partial));
		}

		/*
		Usage:
			auto fact = Y([](std::function<int(int)> self, int n) -> int {
				return n < 2 ? 1 : n * self(n - 1)
			});
		*/

		auto YL = [](auto f) { return 
						[](auto x) { 
							return x (x); 
						}
					}(
						[=](auto y) { return 
							f (
								[=](auto a) { return 
									(y(y))(a); 
								}
							);
						}
					);

	}
}


#endif