// Copyright (c) 2022-2023, Stefano Cristiano
//
// All Rights Reserved. Reproduction is not allowed.
#pragma once
#include "StringView.h"
#include "Vector.h"

namespace SC
{
struct StringConverter;
} // namespace SC

struct SC::StringConverter
{
    /// Converts text to null terminated utf8 sequence. Uses the passed in buffer if necessary.
    [[nodiscard]] static bool toNullTerminatedUTF8(StringView text, Vector<char>& buffer, StringView& encodedText,
                                                   bool forceCopy);

    /// Converts text to null terminated utf16 sequence. Uses the passed in buffer if necessary.
    [[nodiscard]] static bool toNullTerminatedUTF16(StringView text, Vector<char>& buffer, StringView& encodedText,
                                                    bool forceCopy);
    [[nodiscard]] static bool toNullTerminated(StringEncoding encoding, StringView text, Vector<char>& buffer,
                                               StringView& encodedText, bool forceCopy);
};