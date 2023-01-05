// Copyright (c) 2022, Stefano Cristiano
//
// All Rights Reserved. Reproduction is not allowed.
#include "Memory.h"
#include "Assert.h"
#include "Language.h"
#include "Limits.h"

// system includes
#include <memory.h> // memcmp
#include <stdlib.h> // malloc

void* SC::memoryReallocate(void* memory, SC::size_t numBytes) { return realloc(memory, numBytes); }
void* SC::memoryAllocate(SC::size_t numBytes) { return malloc(numBytes); }
void  SC::memoryRelease(void* allocatedMemory) { return free(allocatedMemory); }

#if SC_MSVC
void* operator new(size_t len) { return malloc(len); }
void* operator new[](size_t len) { return malloc(len); }
#else
void*           operator new(SC::size_t len) { return malloc(len); }
void*           operator new[](SC::size_t len) { return malloc(len); }
void*           __cxa_pure_virtual = 0;
extern "C" int  __cxa_guard_acquire(SC::uint64_t* guard_object) { return 0; }
extern "C" void __cxa_guard_release(SC::uint64_t* guard_object) {}
extern "C" void __cxa_guard_abort(SC::uint64_t* guard_object) {}
#endif

void operator delete(void* p) noexcept
{
    if (p != 0)
        SC_LIKELY { free(p); }
}
void operator delete[](void* p) noexcept
{
    if (p != 0)
        SC_LIKELY { free(p); }
}