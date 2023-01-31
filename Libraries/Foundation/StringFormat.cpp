// Copyright (c) 2022-2023, Stefano Cristiano
//
// All Rights Reserved. Reproduction is not allowed.
#include "StringFormat.h"
#include "Console.h"
#include "StringConverter.h"

#include <inttypes.h> // PRIu64 / PRIi64
#include <stdio.h>    // snprintf

namespace SC
{

template <size_t BUFFER_SIZE = 100, size_t SPECIFIER_LENGTH, typename Value>
bool formatSprintf(StringFormatOutput& data, char (&formatSpecifier)[SPECIFIER_LENGTH], StringIteratorASCII specifier,
                   const Value value)
{
    const int SPECIFIER_SIZE = 50;
    char      compoundSpecifier[SPECIFIER_SIZE];
    compoundSpecifier[0]   = '%';
    size_t specifierLength = specifier.getEnd() - specifier.getIt();
    memcpy(compoundSpecifier + 1, specifier.getIt(), specifierLength);
    memcpy(compoundSpecifier + 1 + specifierLength, formatSpecifier, SPECIFIER_LENGTH);
    compoundSpecifier[1 + specifierLength + SPECIFIER_LENGTH] = 0;
    char_t     buffer[BUFFER_SIZE];
    const int  numCharsExcludingTerminator = snprintf(buffer, sizeof(buffer), compoundSpecifier, value);
    const bool validResult = numCharsExcludingTerminator >= 0 && numCharsExcludingTerminator + 1 < BUFFER_SIZE;
    return validResult && data.write(StringView(buffer, numCharsExcludingTerminator, true, StringEncoding::Ascii));
}
#if SC_MSVC
#else

bool StringFormatterFor<SC::size_t>::format(StringFormatOutput& data, const StringIteratorASCII specifier,
                                            const SC::size_t value)
{
    char_t formatSpecifier[] = "zu";
    return formatSprintf(data, formatSpecifier, specifier, value);
}

bool StringFormatterFor<SC::ssize_t>::format(StringFormatOutput& data, const StringIteratorASCII specifier,
                                             const SC::ssize_t value)
{
    char_t formatSpecifier[] = "zd";
    return formatSprintf(data, formatSpecifier, specifier, value);
}
#endif

bool StringFormatterFor<SC::int64_t>::format(StringFormatOutput& data, const StringIteratorASCII specifier,
                                             const SC::int64_t value)
{
    char_t formatSpecifier[] = PRIi64;
    return formatSprintf(data, formatSpecifier, specifier, value);
}

bool StringFormatterFor<SC::uint64_t>::format(StringFormatOutput& data, const StringIteratorASCII specifier,
                                              const SC::uint64_t value)
{
    char_t formatSpecifier[] = PRIu64;
    return formatSprintf(data, formatSpecifier, specifier, value);
}

bool StringFormatterFor<SC::int32_t>::format(StringFormatOutput& data, const StringIteratorASCII specifier,
                                             const SC::int32_t value)
{
    char_t formatSpecifier[] = "d";
    return formatSprintf(data, formatSpecifier, specifier, value);
}

bool StringFormatterFor<SC::uint32_t>::format(StringFormatOutput& data, const StringIteratorASCII specifier,
                                              const SC::uint32_t value)
{
    char_t formatSpecifier[] = "d";
    return formatSprintf(data, formatSpecifier, specifier, value);
}

bool StringFormatterFor<SC::int16_t>::format(StringFormatOutput& data, const StringIteratorASCII specifier,
                                             const SC::int16_t value)
{
    return StringFormatterFor<SC::int32_t>::format(data, specifier, value);
}

bool StringFormatterFor<SC::uint16_t>::format(StringFormatOutput& data, const StringIteratorASCII specifier,
                                              const SC::uint16_t value)
{
    return StringFormatterFor<SC::uint32_t>::format(data, specifier, value);
}
bool StringFormatterFor<SC::int8_t>::format(StringFormatOutput& data, const StringIteratorASCII specifier,
                                            const SC::int8_t value)
{
    return StringFormatterFor<SC::int32_t>::format(data, specifier, value);
}

bool StringFormatterFor<SC::uint8_t>::format(StringFormatOutput& data, const StringIteratorASCII specifier,
                                             const SC::uint8_t value)
{
    return StringFormatterFor<SC::uint32_t>::format(data, specifier, value);
}

bool StringFormatterFor<float>::format(StringFormatOutput& data, StringIteratorASCII specifier, const float value)
{
    char_t formatSpecifier[] = "f";
    return formatSprintf(data, formatSpecifier, specifier, value);
}

bool StringFormatterFor<double>::format(StringFormatOutput& data, const StringIteratorASCII specifier,
                                        const double value)
{
    char_t formatSpecifier[] = "f";
    return formatSprintf(data, formatSpecifier, specifier, value);
}

bool StringFormatterFor<SC::char_t>::format(StringFormatOutput& data, const StringIteratorASCII specifier,
                                            const SC::char_t value)
{
    return data.write(StringView(&value, 1, false, StringEncoding::Ascii));
}

bool StringFormatterFor<const SC::char_t*>::format(StringFormatOutput& data, const StringIteratorASCII specifier,
                                                   const SC::char_t* value)
{
    return data.write(StringView(value, strlen(value), true, StringEncoding::Ascii));
}

bool StringFormatterFor<SC::StringView>::format(StringFormatOutput& data, const StringIteratorASCII specifier,
                                                const SC::StringView value)
{
    if (value.getEncoding() == StringEncoding::Utf16)
    {
        const char* nullTerminated = nullptr;
        const bool  res            = StringConverter::toNullTerminatedUTF8(value, data.buffer, &nullTerminated, true);
        return res && data.buffer.pop_back() &&
               data.write(StringView(data.buffer.data(), data.buffer.size(), true, StringEncoding::Utf16));
    }
    else
    {
        return data.write(value);
    }
}

bool StringFormatOutput::write(StringView text)
{
    if (writeToStdout)
    {
        Console::print(text);
        return true;
    }
    else
    {
        if ((encoding == text.getEncoding()) or
            (encoding == StringEncoding::Utf8 and text.getEncoding() == StringEncoding::Ascii) or
            (encoding == StringEncoding::Ascii and text.getEncoding() == StringEncoding::Utf8))
        {
            return data.appendCopy(text.getIterator<StringIteratorASCII>().getIt(), text.sizeInBytes());
        }
        else
        {
            // TODO: Implement mixing different encodings when writing to buffer
            SC_DEBUG_ASSERT("Not Implemented" && 0);
            return false;
        }
    }
}

void StringFormatOutput::onFormatBegin()
{
    if (not writeToStdout)
    {
        backupSize = data.size();
    }
}

bool StringFormatOutput::onFormatSucceded()
{
    if (not writeToStdout)
    {
        if (backupSize < data.size())
        {
            return data.push_back(0);
        }
    }
    return true;
}

void StringFormatOutput::onFormatFailed()
{
    if (not writeToStdout)
    {
        (void)data.resize(backupSize);
    }
}
} // namespace SC
