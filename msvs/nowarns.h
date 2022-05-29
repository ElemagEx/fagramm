#pragma once

#ifdef _MSC_VER

// prevent vector warnings:
//  4365 - 'action' : conversion from 'type_1' to 'type_2', signed/unsigned mismatch
#pragma warning(disable:4365)
#include <vector>
#pragma warning(default:4365)

// prevent unsignificant warnings in fagramm
#pragma warning(disable:4061) // enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label
#pragma warning(disable:4514) // 'function' : unreferenced inline function has been removed
#pragma warning(disable:4820) // 'bytes' bytes padding added after construct 'member_name'
#pragma warning(disable:5045) // Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified

#endif
