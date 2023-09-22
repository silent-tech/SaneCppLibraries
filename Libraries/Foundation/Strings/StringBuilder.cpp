// Copyright (c) 2022-2023, Stefano Cristiano
//
// All Rights Reserved. Reproduction is not allowed.
#include "StringBuilder.h"
#include "StringConverter.h"
namespace SC
{
StringBuilder::StringBuilder(Vector<char>& stringData, StringEncoding encoding, Flags f)
    : stringData(stringData), encoding(encoding)
{
    if (f == Clear)
    {
        clear();
    }
}
StringBuilder::StringBuilder(String& str, Flags f) : stringData(str.data), encoding(str.getEncoding())
{
    if (f == Clear)
    {
        clear();
    }
}
bool StringBuilder::format(StringView text)
{
    clear();
    return append(text);
}

bool StringBuilder::append(StringView str)
{
    if (str.isEmpty())
        return true;
    SC_TRY_IF(StringConverter::popNulltermIfExists(stringData, encoding));
    return StringConverter::convertEncodingTo(encoding, str, stringData);
}

bool StringBuilder::appendReplaceAll(StringView source, StringView occurrencesOf, StringView with)
{
    if (not source.hasCompatibleEncoding(occurrencesOf) or not source.hasCompatibleEncoding(with) or
        not source.hasCompatibleEncoding(view()))
    {
        return false;
    }
    if (source.isEmpty())
    {
        return true;
    }
    if (occurrencesOf.isEmpty())
    {
        return append(source);
    }
    SC_TRY_IF(StringConverter::popNulltermIfExists(stringData, encoding));
    StringView current             = source;
    const auto occurrencesIterator = occurrencesOf.getIterator<StringIteratorASCII>();
    bool       res;
    do
    {
        auto sourceIt    = current.getIterator<StringIteratorASCII>();
        res              = sourceIt.advanceBeforeFinding(occurrencesIterator);
        StringView soFar = StringView::fromIteratorFromStart(sourceIt);
        SC_TRY_IF(stringData.appendCopy(soFar.bytesWithoutTerminator(), soFar.sizeInBytes()));
        if (res)
        {
            SC_TRY_IF(stringData.appendCopy(with.bytesWithoutTerminator(), with.sizeInBytes()));
            res     = sourceIt.advanceByLengthOf(occurrencesIterator);
            current = StringView::fromIteratorUntilEnd(sourceIt);
        }
    } while (res);
    SC_TRY_IF(stringData.appendCopy(current.bytesWithoutTerminator(), current.sizeInBytes()));
    return StringConverter::pushNullTerm(stringData, encoding);
}

[[nodiscard]] bool StringBuilder::appendReplaceMultiple(StringView source, Span<const StringView[2]> substitutions)
{
    String buffer, other;
    SC_TRY_IF(buffer.assign(source));
    for (auto it : substitutions)
    {
        SC_TRY_IF(StringBuilder(other, StringBuilder::Clear).appendReplaceAll(buffer.view(), it[0], it[1]));
        swap(other, buffer);
    }
    return append(buffer.view());
}

bool StringBuilder::appendHex(SpanVoid<const void> data)
{
    const unsigned char* bytes = data.castTo<const unsigned char>().data();
    if (encoding == StringEncoding::Utf16)
        return false; // TODO: Support appendHex for UTF16
    const auto oldSize = stringData.size();
    SC_TRY_IF(stringData.resizeWithoutInitializing(stringData.size() + data.sizeInBytes() * 2));
    const auto sizeInBytes = data.sizeInBytes();
    for (size_t i = 0; i < sizeInBytes; i++)
    {
        stringData[oldSize + i * 2]     = "0123456789ABCDEF"[bytes[i] >> 4];
        stringData[oldSize + i * 2 + 1] = "0123456789ABCDEF"[bytes[i] & 0x0F];
    }
    return StringConverter::pushNullTerm(stringData, encoding);
}

StringView StringBuilder::view() const
{
    return stringData.isEmpty() ? StringView() : StringView(stringData.data(), stringData.size(), true, encoding);
}

void StringBuilder::clear() { stringData.clearWithoutInitializing(); }

} // namespace SC