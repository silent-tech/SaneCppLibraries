// Copyright (c) 2022-2023, Stefano Cristiano
//
// All Rights Reserved. Reproduction is not allowed.
#pragma once
#include "Compiler.h"
#include "Types.h"

extern "C"
{
#if SC_MSVC
    __declspec(dllimport) __declspec(noreturn) void __cdecl exit(int _Code);
    void* __cdecl memcpy(void* dst, const void* src, size_t n);
    int __cdecl memcmp(const void* s1, const void* s2, size_t n);
    void* __cdecl memset(void* dst, SC::int32_t c, size_t len);
    [[nodiscard]] void const* __cdecl memchr(const void* ptr, SC::int32_t c, size_t count);
#else
    void  exit(SC::int32_t val) __attribute__((__noreturn__));
    void* memcpy(void* dst, const void* src, SC::size_t n);
    int   memcmp(const void* s1, const void* s2, SC::size_t n);
    void* memset(void* dst, SC::int32_t c, SC::size_t len);
    void* memchr(const void* ptr, SC::int32_t c, SC::size_t count);
#endif
    // string
}
