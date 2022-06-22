#pragma once

#if !defined(__GNUC__)
#define _CRT_SECURE_NO_WARNINGS
#endif

#if defined(__clang__)

#pragma clang diagnostic ignored "-Wswitch-enum"

#pragma clang diagnostic ignored "-Wc++98-compat-pedantic"
#pragma clang diagnostic ignored "-Wc++98-compat"
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wpadded"

#elif defined(__GNUC__)

//#pragma GCC diagnostic ignored "-Wunused-parameter"

#elif defined(_MSC_VER)

#pragma warning(disable:4710) // 'function' : function not inlined
#pragma warning(disable:4711) // function 'function' selected for inline expansion

// prevent MS C std library warnings:
//  4365 - 'action' : conversion from 'type_1' to 'type_2', signed/unsigned mismatch
#pragma warning(disable:4365)
#include <vector>
#include <atomic>
#pragma warning(default:4365)

// prevent unsignificant warnings
#pragma warning(disable:4061) // enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label
#pragma warning(disable:4100) // 'identifier' : unreferenced formal parameter
#pragma warning(disable:4514) // 'function' : unreferenced inline function has been removed
#pragma warning(disable:4820) // 'bytes' bytes padding added after construct 'member_name'
#pragma warning(disable:5045) // Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
#pragma warning(disable:5039) // 'function': pointer or reference to potentially throwing function passed to 'extern "C"' function under -EHc. Undefined behavior may occur if this function throws an exception.

//#ifndef maybe_unused
//#pragma warning(disable:5051) // attribute 'attribute-name' requires at least 'standard-level'; ignored
//#endif
#pragma warning(disable:4848) // support for attribute 'no_unique_address' in C++17 and earlier is a vendor extension

#endif
