#ifndef SN_LINQ_H
#define SN_LINQ_H

#include "sn_CommonHeader.h"

namespace sn_LINQ {
	// Simple implement: just use pipeline in sn_Function
	// R = ChainHead (beginning of chain) | where<a, b> (template function restrict)... | ....
	// from(v) (vector or some container) | R (functor) 
	namespace linq {
		template <typename T>
		class LINQ {
		private:
			T m_range;
		public:
			using value_type = T::value_type;
			LINQ(T& range) : m_range(range) {}


		};
	}
}






#endif