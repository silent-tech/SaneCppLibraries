// Copyright (c) 2022-2023, Stefano Cristiano
//
// All Rights Reserved. Reproduction is not allowed.
#pragma once
#include "Array.h"
#include "StringFormat.h"
#include "StringView.h"
#include "Vector.h"

namespace SC
{
struct String;

template <int N>
struct SmallString;
} // namespace SC
struct SC::String
{
    String(StringEncoding encoding = StringEncoding::Utf8) : encoding(encoding) {}

    // TODO: Figure out if removing this constructor in favour of the fallible assign makes the api ugly
    String(const StringView& sv) { SC_RELEASE_ASSERT(assign(sv)); }

    [[nodiscard]] bool assign(const StringView& sv);

    // const methods
    [[nodiscard]] StringEncoding getEncoding() const { return encoding; }
    [[nodiscard]] size_t         sizeInBytesIncludingTerminator() const { return data.size(); }
    //  - if string is empty        --> data.size() == 0
    //  - if string is not empty    --> data.size() > 2
    [[nodiscard]] const char_t* bytesIncludingTerminator() const { return data.data(); }
#if SC_PLATFORM_WINDOWS
    [[nodiscard]] wchar_t* nativeWritableBytesIncludingTerminator() { return reinterpret_cast<wchar_t*>(data.data()); }
#else
    [[nodiscard]] char_t* nativeWritableBytesIncludingTerminator() { return data.data(); }
#endif
    [[nodiscard]] bool isEmpty() const { return data.isEmpty(); }

    [[nodiscard]] StringView view() const;

    [[nodiscard]] bool popNulltermIfExists();
    [[nodiscard]] bool pushNullTerm();

    // Operators
    [[nodiscard]] bool operator==(const String& other) const { return view() == (other.view()); }
    [[nodiscard]] bool operator!=(const String& other) const { return not operator==(other); }
    [[nodiscard]] bool operator==(const StringView other) const { return view() == (other); }
    [[nodiscard]] bool operator!=(const StringView other) const { return not operator==(other); }
    [[nodiscard]] bool operator<(const StringView other) const { return view() < other; }

    // TODO: we should probably make these private and provide alternative abstractions to manipulate it
    StringEncoding encoding;
    Vector<char>   data;
};

template <int N>
struct SC::SmallString : public String
{
    Array<char, N> buffer;
    SmallString(StringEncoding encoding = StringEncoding::Utf8) : String(encoding)
    {
        SegmentHeader* header         = SegmentHeader::getSegmentHeader(buffer.items);
        header->options.isSmallVector = true;
        String::data.items            = buffer.items;
    }
    SmallString(StringView view) : String(view.getEncoding())
    {
        SegmentHeader* header         = SegmentHeader::getSegmentHeader(buffer.items);
        header->options.isSmallVector = true;
        String::data.items            = buffer.items;
        SC_RELEASE_ASSERT(assign(view));
    }
    SmallString(SmallString&& other) : String(forward<String>(other)) {}
    SmallString(const SmallString& other) : String(other) {}
    SmallString& operator=(SmallString&& other)
    {
        String::operator=(forward<String>(other));
        return *this;
    }
    SmallString& operator=(const SmallString& other)
    {
        String::operator=(other);
        return *this;
    }

    SmallString& operator=(StringView other)
    {
        SC_RELEASE_ASSERT(assign(other));
        return *this;
    }

    SmallString(String&& other) : String(forward<String>(other)) {}
    SmallString(const String& other) : String(other) {}
    SmallString& operator=(String&& other)
    {
        String::operator=(forward<String>(other));
        return *this;
    }
    SmallString& operator=(const String& other)
    {
        String::operator=(other);
        return *this;
    }
};

template <>
struct SC::StringFormatterFor<SC::String>
{
    static bool format(StringFormatOutput& data, const StringView specifier, const String& value);
};

namespace SC
{
template <int N>
using StringNative = SmallString<N * sizeof(utf_char_t)>;
} // namespace SC
