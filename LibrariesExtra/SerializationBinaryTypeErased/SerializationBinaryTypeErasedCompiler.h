// Copyright (c) 2022-2023, Stefano Cristiano
//
// All Rights Reserved. Reproduction is not allowed.
#pragma once
#include "../../Libraries/Reflection/Reflection.h"
#include "../../Libraries/Reflection/ReflectionCompiler.h"
namespace SC
{
namespace Reflection
{

struct VectorVTable
{
    enum class DropEccessItems
    {
        No,
        Yes
    };

    using FunctionGetSegmentSpan      = bool (*)(TypeInfo property, Span<uint8_t> object, Span<uint8_t>& itemBegin);
    using FunctionGetSegmentSpanConst = bool (*)(TypeInfo property, Span<const uint8_t> object,
                                                 Span<const uint8_t>& itemBegin);

    using FunctionResize = bool (*)(Span<uint8_t> object, Reflection::TypeInfo property, uint64_t sizeInBytes,
                                    DropEccessItems dropEccessItems);
    using FunctionResizeWithoutInitialize = bool (*)(Span<uint8_t> object, Reflection::TypeInfo property,
                                                     uint64_t sizeInBytes, DropEccessItems dropEccessItems);
    FunctionGetSegmentSpan          getSegmentSpan;
    FunctionGetSegmentSpanConst     getSegmentSpanConst;
    FunctionResize                  resize;
    FunctionResizeWithoutInitialize resizeWithoutInitialize;
    uint32_t                        linkID;

    constexpr VectorVTable()
        : getSegmentSpan(nullptr), getSegmentSpanConst(nullptr), resize(nullptr), resizeWithoutInitialize(nullptr),
          linkID(0)
    {}
};
template <int MAX_VTABLES>
struct ReflectionVTables
{
    ArrayWithSize<VectorVTable, MAX_VTABLES> vector;
};

struct TypeBuilderTypeErased : public SchemaBuilder<TypeBuilderTypeErased>
{
    using Type = ReflectedType<TypeBuilderTypeErased>;

    static const uint32_t          MAX_VTABLES = 100;
    ReflectionVTables<MAX_VTABLES> vtables;

    constexpr TypeBuilderTypeErased(Type* output, const uint32_t capacity) : SchemaBuilder(output, capacity) {}
};

template <typename Container, typename ItemType, int N>
struct VectorArrayVTable<TypeBuilderTypeErased, Container, ItemType, N>
{
    [[nodiscard]] constexpr static bool build(TypeBuilderTypeErased& builder)
    {
        VectorVTable vector;
        vector.resize = &resize;
        assignResizeWithoutInitialize(vector);
        vector.getSegmentSpan      = &getSegmentSpan<uint8_t>;
        vector.getSegmentSpanConst = &getSegmentSpan<const uint8_t>;
        vector.linkID              = builder.currentLinkID;
        return builder.vtables.vector.push_back(vector);
    }

    static bool resize(Span<uint8_t> object, Reflection::TypeInfo property, uint64_t sizeInBytes,
                       VectorVTable::DropEccessItems dropEccessItems)
    {
        SC_COMPILER_UNUSED(property);
        SC_COMPILER_UNUSED(dropEccessItems);
        if (object.sizeInBytes() >= sizeof(void*))
        {
            auto&      vectorByte = *reinterpret_cast<Container*>(object.data());
            const auto numItems   = N >= 0 ? min(sizeInBytes / sizeof(ItemType), static_cast<decltype(sizeInBytes)>(N))
                                           : sizeInBytes / sizeof(ItemType);
            return vectorByte.resize(static_cast<size_t>(numItems));
        }
        else
        {
            return false;
        }
    }
    static bool resizeWithoutInitialize(Span<uint8_t> object, Reflection::TypeInfo property, uint64_t sizeInBytes,
                                        VectorVTable::DropEccessItems dropEccessItems)
    {
        SC_COMPILER_UNUSED(property);
        SC_COMPILER_UNUSED(dropEccessItems);
        if (object.sizeInBytes() >= sizeof(void*))
        {
            auto&      vectorByte = *reinterpret_cast<Container*>(object.data());
            const auto numItems   = N >= 0 ? min(sizeInBytes / sizeof(ItemType), static_cast<decltype(sizeInBytes)>(N))
                                           : sizeInBytes / sizeof(ItemType);
            return vectorByte.resizeWithoutInitializing(static_cast<size_t>(numItems));
        }
        else
        {
            return false;
        }
    }

    template <typename ByteType>
    [[nodiscard]] static constexpr bool getSegmentSpan(Reflection::TypeInfo property, Span<ByteType> object,
                                                       Span<ByteType>& itemBegin)
    {
        SC_COMPILER_UNUSED(property);
        if (object.sizeInBytes() >= sizeof(void*))
        {
            using VectorType = typename SameConstnessAs<ByteType, Container>::type;
            auto& vectorByte = *reinterpret_cast<VectorType*>(object.data());
            itemBegin        = itemBegin.reinterpret_bytes(vectorByte.data(), vectorByte.size() * sizeof(ItemType));
            return true;
        }
        else
        {
            return false;
        }
    }
    template <typename Q = ItemType>
    [[nodiscard]] static typename EnableIf<not IsTriviallyCopyable<Q>::value, void>::type //
        constexpr assignResizeWithoutInitialize(VectorVTable& vector)
    {
        SC_COMPILER_UNUSED(vector);
    }

    template <typename Q = ItemType>
    [[nodiscard]] static typename EnableIf<IsTriviallyCopyable<Q>::value, void>::type //
        constexpr assignResizeWithoutInitialize(VectorVTable& vector)
    {
        vector.resizeWithoutInitialize = &resizeWithoutInitialize;
    }
};

using FlatSchemaTypeErased = Reflection::Compiler<TypeBuilderTypeErased>;

} // namespace Reflection

namespace SerializationBinaryTypeErased
{

struct ArrayAccess
{
    Span<const Reflection::VectorVTable> vectorVtable;

    [[nodiscard]] bool getSegmentSpan(uint32_t linkID, Reflection::TypeInfo property, Span<uint8_t> object,
                                      Span<uint8_t>& itemBegin);
    [[nodiscard]] bool getSegmentSpan(uint32_t linkID, Reflection::TypeInfo property, Span<const uint8_t> object,
                                      Span<const uint8_t>& itemBegin);

    using DropEccessItems = Reflection::VectorVTable::DropEccessItems;
    enum class Initialize
    {
        No,
        Yes
    };

    bool resize(uint32_t linkID, Span<uint8_t> object, Reflection::TypeInfo property, uint64_t sizeInBytes,
                Initialize initialize, DropEccessItems dropEccessItems);
};
} // namespace SerializationBinaryTypeErased
} // namespace SC
