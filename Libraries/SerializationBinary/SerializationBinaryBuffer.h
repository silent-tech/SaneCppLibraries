// Copyright (c) 2022-2023, Stefano Cristiano
//
// All Rights Reserved. Reproduction is not allowed.
#pragma once
#include "../Containers/Vector.h"
#include "../Foundation/Span.h"

namespace SC
{
namespace SerializationBinary
{
// Keeping SerializationBinary::Buffer in header just so that SerializationBinary is header only

//! @addtogroup group_serialization_binary
//! @{

/// @brief A binary serialization reader / writer backed by a memory buffer.
struct Buffer
{
    SC::Vector<uint8_t> buffer; ///< The underlying buffer holding serialization data

    size_t readPosition       = 0; ///< Current read  position in the buffer
    size_t numberOfOperations = 0; ///< How many read or write operations have been issued so far

    /// @brief Write span of bytes to buffer
    /// @param object Span of bytes
    /// @return `true` if write succeeded
    [[nodiscard]] bool serializeBytes(Span<const uint8_t> object)
    {
        numberOfOperations++;
        return buffer.append(object);
    }

    /// @brief Read bytes into given span of memory. Updates readPosition
    /// @param object The destination span
    /// @return `true` if read succeded
    [[nodiscard]] bool serializeBytes(Span<uint8_t> object)
    {
        if (readPosition + object.sizeInBytes() > buffer.size())
            return false;
        numberOfOperations++;
        memcpy(object.data(), &buffer[readPosition], object.sizeInBytes());
        readPosition += object.sizeInBytes();
        return true;
    }

    /// @brief Advance read position by numBytes
    /// @param numBytes How many bytes to advance read position of
    /// @return `true` if size of buffer has not been exceeded.
    [[nodiscard]] bool advanceBytes(size_t numBytes)
    {
        if (readPosition + numBytes > buffer.size())
            return false;
        readPosition += numBytes;
        return true;
    }
};

/// @brief A binary serialization bytes writer based on SerializationBinary::Buffer
struct BufferWriter : public Buffer
{
    using SerializationBinary::Buffer::serializeBytes;
    /// @brief Write given object to buffer
    /// @param object The source object
    /// @param numBytes Size of source object
    /// @return `true` if write succeeded
    [[nodiscard]] bool serializeBytes(const void* object, size_t numBytes)
    {
        return SerializationBinary::Buffer::serializeBytes(Span<const uint8_t>::reinterpret_bytes(object, numBytes));
    }
};

/// @brief A binary serialization bytes reader based on SerializationBinary::Buffer
struct BufferReader : public Buffer
{
    using SerializationBinary::Buffer::serializeBytes;
    /// @brief Read from buffer into given object
    /// @param object Destination object
    /// @param numBytes How many bytes to read from object
    /// @return `true` if read succeeded
    [[nodiscard]] bool serializeBytes(void* object, size_t numBytes)
    {
        return Buffer::serializeBytes(Span<uint8_t>::reinterpret_bytes(object, numBytes));
    }
};
//! @}

} // namespace SerializationBinary
} // namespace SC