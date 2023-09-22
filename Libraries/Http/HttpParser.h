// Copyright (c) 2022-2023, Stefano Cristiano
//
// All Rights Reserved. Reproduction is not allowed.
#pragma once
#include "../Foundation/Objects/Result.h"

namespace SC
{
struct HttpParser;
} // namespace SC

struct SC::HttpParser
{
    enum class Method
    {
        HttpGET,
        HttpPUT,
        HttpPOST,
    };
    Method method = Method::HttpGET;

    size_t   tokenStart = 0, tokenLength = 0;
    uint32_t statusCode    = 0;
    uint64_t contentLength = 0;

    enum class State
    {
        Parsing,
        Result,
        Finished,
    };

    enum class Result
    {
        Method,
        Url,
        Version,
        HeaderName,
        HeaderValue,
        HeadersEnd,

        StatusCode,
        StatusString,
        Body
    };

    Result result = Result::HeadersEnd;
    State  state  = State::Parsing;

    enum class Type
    {
        Request,
        Response
    };
    Type type = Type::Request;

    ReturnCode parse(Span<const char> data, size_t& readBytes, Span<const char>& parsedData);
    enum class HeaderType
    {
        ContentLength = 0
    };

    bool matchesHeader(HeaderType headerName) const;

  private:
    size_t globalStart           = 0;
    size_t globalLength          = 0;
    int    topLevelCoroutine     = 0;
    int    nestedParserCoroutine = 0;
    bool   parsedcontentLength   = false;
    size_t matchIndex            = 0;

    static constexpr size_t numMatches = 1;

    size_t   matchingHeader[numMatches]      = {0};
    bool     matchingHeaderValid[numMatches] = {false};
    uint64_t number                          = 0;

    bool parseHeaderName(char currentChar);
    bool parseHeaderValue(char currentChar);
    bool parseStatusCode(char currentChar);
    bool parseNumberValue(char currentChar);
    bool parseHeadersEnd(char currentChar);
    bool parseMethod(char currentChar);
    bool parseUrl(char currentChar);

    template <bool spaces>
    bool parseVersion(char currentChar);

    template <bool (HttpParser::*Func)(char), Result currentResult>
    ReturnCode process(Span<const char>& data, size_t& readBytes, Span<const char>& parsedData);
};