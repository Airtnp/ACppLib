#ifndef SN_COMMON_HEADER_H
#define SN_COMMON_HEADER_H

/*
	For C-style library
*/
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <intrin.h>
#include <inttypes.h>
#include <ctime>
#include <cassert>
#ifndef _STDC_FORMAT_MACROS
#define _STDC_FORMAT_MACROS
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#if defined(__GNUC__)

/* gcc policy-based data structure
for detail: https://gcc.gnu.org/onlinedocs/libstdc++/ext/pb_ds/
*/
#include <ext/pb_ds/>
#include <x86intrin.h>
#else
#include <intrin.h>
#endif

/*
	For STL library
*/

//#include <bits/stdc++.h>

// For STL io/stream operation
#include <iostream>
#include <codecvt>
#include <sstream>
#include <iomanip>

// For STL generic operation
#include <algorithm>
#include <numeric>
#include <functional>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <utility>

// For STL memory/thread operation
#include <mutex>
#include <memory>
#include <atomic>
#include <future>

// For STL ds
#include <array>
#include <valarray>
#include <tuple>
#include <string>
#include <vector>
#include <list>
#include <bitset>
#include <complex>
#include <map>
#include <unordered_map>
#include <queue>
#include <initializer_list>
#include <iterator>
#include <set>
#include <unordered_set>
#include <stack>

// For exception
#include <stdexcept>
#include <exception>


// For platform-related
#ifdef _WIN32
#include <fstream>
#else
#include <fcntl.h>
#endif

template <typename T>
using observer_ptr = T*;

#endif