// Copyright (c) 2022-2023, Stefano Cristiano
//
// All Rights Reserved. Reproduction is not allowed.
#pragma once
// This needs to go before the compiler
#include "../Reflection/ReflectionSC.h"
// Compiler must be after
#include "SerializationBinarySkipper.h"
#include "SerializationBinaryTypeErasedCompiler.h"
namespace SC
{
namespace SerializationBinaryTypeErased
{
struct VersionSchema
{
    Span<const Reflection::MetaProperties> sourceProperties;
};

struct SerializerReadVersioned
{
    struct Options
    {
        bool allowFloatToIntTruncation    = true;
        bool allowDropEccessArrayItems    = true;
        bool allowDropEccessStructMembers = true;
    };
    Options options;

    template <typename T>
    [[nodiscard]] bool readVersioned(T& object, Serialization::BinaryBuffer& source, VersionSchema& schema)
    {
        constexpr auto flatSchema = Reflection::FlatSchemaTypeErased::compile<T>();
        sourceProperties          = schema.sourceProperties;
        sinkProperties            = {flatSchema.properties.values, flatSchema.properties.size};
        sinkNames                 = {flatSchema.names.values, flatSchema.names.size};
        sinkObject                = SpanVoid<void>(&object, sizeof(T));
        sourceObject              = &source;
        sinkTypeIndex             = 0;
        sourceTypeIndex           = 0;
        arrayAccess.vectorVtable  = {flatSchema.vtables.vector.values,
                                     static_cast<size_t>(flatSchema.vtables.vector.size)};

        if (sourceProperties.sizeInBytes() == 0 ||
            sourceProperties.data()[0].type != Reflection::MetaType::TypeStruct || sinkProperties.sizeInBytes() == 0 ||
            sinkProperties.data()[0].type != Reflection::MetaType::TypeStruct)
        {
            return false;
        }
        return read();
    }

  private:
    Span<const Reflection::SymbolStringView> sinkNames;

    ArrayAccess arrayAccess;

    Span<const Reflection::MetaProperties> sinkProperties;
    SpanVoid<void>                         sinkObject;
    Reflection::MetaProperties             sinkProperty;
    uint32_t                               sinkTypeIndex = 0;

    Span<const Reflection::MetaProperties> sourceProperties;
    Serialization::BinaryBuffer*           sourceObject = nullptr;
    Reflection::MetaProperties             sourceProperty;
    uint32_t                               sourceTypeIndex = 0;

    [[nodiscard]] bool read();
    [[nodiscard]] bool readStruct();
    [[nodiscard]] bool readArrayVector();
    [[nodiscard]] bool skipCurrent();
};

} // namespace SerializationBinaryTypeErased
} // namespace SC