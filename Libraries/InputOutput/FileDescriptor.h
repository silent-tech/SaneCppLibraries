// Copyright (c) 2022-2023, Stefano Cristiano
//
// All Rights Reserved. Reproduction is not allowed.
#pragma once
#include "../Foundation/Opaque.h"
#include "../Foundation/Result.h"
#include "../Foundation/Vector.h"
namespace SC
{
struct FileDescriptor;
struct FileDescriptorPipe;
struct FileDescriptorWindows;
struct FileDescriptorPosix;

#if SC_PLATFORM_WINDOWS
using FileDescriptorNative                                        = void*;   // HANDLE
static constexpr FileDescriptorNative FileDescriptorNativeInvalid = nullptr; // INVALID_HANDLE_VALUE
#else
using FileDescriptorNative                                        = int; // Posix FD
static constexpr FileDescriptorNative FileDescriptorNativeInvalid = -1;  // Posix Invalid FD
#endif
ReturnCode FileDescriptorNativeClose(FileDescriptorNative&);
struct FileDescriptorNativeHandle : public UniqueTaggedHandle<FileDescriptorNative, FileDescriptorNativeInvalid,
                                                              ReturnCode, FileDescriptorNativeClose>
{
};

} // namespace SC

struct SC::FileDescriptorWindows
{
    SC::FileDescriptor& fileDescriptor;
};

struct SC::FileDescriptorPosix
{
    SC::FileDescriptor&      fileDescriptor;
    [[nodiscard]] ReturnCode duplicateAndReplace(int fds);
};

struct FileDescriptorOptions
{
    bool                   inheritable = false;
    FileDescriptorOptions& setInheritable(bool inheritable)
    {
        this->inheritable = inheritable;
        return *this;
    }
};

struct SC::FileDescriptor
{
    struct ReadResult
    {
        size_t actuallyRead = 0;
        bool   isEOF        = false;
    };
    [[nodiscard]] Result<ReadResult> readAppend(Vector<char>& output, Span<char> fallbackBuffer);

    [[nodiscard]] ReturnCode setBlocking(bool blocking);
    [[nodiscard]] ReturnCode setInheritable(bool inheritable);

    FileDescriptorPosix   posix() { return {*this}; }
    FileDescriptorWindows windows() { return {*this}; }

    FileDescriptorNativeHandle handle;

    struct Internal;
};

struct SC::FileDescriptorPipe
{
    enum InheritableReadFlag
    {
        ReadInheritable,
        ReadNonInheritable
    };
    enum InheritableWriteFlag
    {
        WriteInheritable,
        WriteNonInheritable
    };
    FileDescriptor readPipe;
    FileDescriptor writePipe;

    /// Creates a Pipe. Default is non-inheritable / blocking
    [[nodiscard]] ReturnCode createPipe(InheritableReadFlag readFlag, InheritableWriteFlag writeFlag);
};