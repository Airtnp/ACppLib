#ifndef SN_COMMON_HEADER_H
#define SN_COMMON_HEADER_H

#pragma warning(disable: 4814)

/*
	For C-style library
*/
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <inttypes.h>
#include <ctime>
#include <cassert>
#include <cstddef>
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
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>
#include <x86intrin.h>
#include <cxxabi.h>
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
#include <condition_variable>

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
#include <regex>
#include <variant>

// For exception
#include <stdexcept>
#include <exception>
#include <system_error>

// For platform-related
#ifdef _WIN32
#include <fstream>
#include <windows.h>

#ifdef SN_ENABLE_STACK_WALKER
#include <stdio.h>
#include <tchar.h>
#pragma comment(lib, "version.lib")
#endif

#elif defined(__linux__)
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
	#ifdef SN_ENABLE_STACK_WALKER
        #include <signal.h>
        #include <stdio.h>
        #include <stdlib.h>
        #include <execinfo.h>
        #include <string.h>
	#endif
#endif

// Some feature switch
// #define SN_ENABLE_WINDOWS_API
// #define SN_ENABLE_CPP_17_EXPERIMENTAL
// #define SN_ENABLE_STACK_WALKER
// #define SN_ENABLE_TEMPORARY_UNAVAILABLE
// #define SN_ENABLE_SUSPICIOUS_IMPLEMENTATION
// #define SN_ENABLE_SELF_ASSERT
// #define SN_ENABLE_FILE_MAP
// #define SN_ENABLE_TYPELIST_LAZY


template <typename T>
using observer_ptr = T*;

template <typename T>
using owned_ptr = T*;

using size_t = std::size_t;

#endif