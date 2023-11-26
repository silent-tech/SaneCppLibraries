// Copyright (c) 2022-2023, Stefano Cristiano
//
// All Rights Reserved. Reproduction is not allowed.
#include "../../Containers/Array.h"
#include "../../Containers/Vector.h"
#include "../../Strings/StringBuilder.h"
#include "../../Testing/Testing.h"

// TODO: Split the Auto reflection tests in ReflectionAuto
#if SC_META_ENABLE_AUTO_REFLECTION
#if SC_LANGUAGE_CPP_LESS_THAN_20
#include "../../../LibrariesExtra/ReflectionAuto/ReflectionAutoAggregates.h"
#else
#include "../../../LibrariesExtra/ReflectionAuto/ReflectionAutoStructured.h"
#endif
#endif
#include "../ReflectionCompiler.h"
#include "../ReflectionSC.h"

namespace TestNamespace
{
struct SimpleStructure
{
    // Base Types
    SC::uint8_t  f1  = 0;
    SC::uint16_t f2  = 1;
    SC::uint32_t f3  = 2;
    SC::uint64_t f4  = 3;
    SC::int8_t   f5  = 4;
    SC::int16_t  f6  = 5;
    SC::int32_t  f7  = 6;
    SC::int64_t  f8  = 7;
    float        f9  = 8;
    double       f10 = 9;

    int arrayOfInt[3] = {1, 2, 3};
};

struct IntermediateStructure
{
    SC::Vector<int> vectorOfInt;
    SimpleStructure simpleStructure;
};

struct ComplexStructure
{
    SC::uint8_t                 f1 = 0;
    SimpleStructure             simpleStructure;
    SimpleStructure             simpleStructure2;
    SC::uint16_t                f4 = 0;
    IntermediateStructure       intermediateStructure;
    SC::Vector<SimpleStructure> vectorOfStructs;
};
} // namespace TestNamespace

namespace SC
{
namespace Reflection
{

template <>
struct Reflect<TestNamespace::SimpleStructure> : ReflectStruct<TestNamespace::SimpleStructure>
{
    template <typename MemberVisitor>
    static constexpr bool visit(MemberVisitor&& visitor)
    {
        return                                                        //
            visitor(0, "f1", &T::f1, SC_COMPILER_OFFSETOF(T, f1)) and //
            visitor(1, "f2", &T::f2, SC_COMPILER_OFFSETOF(T, f2)) and //
            visitor(2, "arrayOfInt", &T::arrayOfInt, SC_COMPILER_OFFSETOF(T, arrayOfInt));
    }
};

template <>
struct Reflect<TestNamespace::IntermediateStructure> : ReflectStruct<TestNamespace::IntermediateStructure>
{
    template <typename MemberVisitor>
    static constexpr bool visit(MemberVisitor&& visitor)
    {
        return //
            visitor(1, "vectorOfInt", &T::vectorOfInt, SC_COMPILER_OFFSETOF(T, vectorOfInt)) and
            visitor(0, "simpleStructure", &T::simpleStructure, SC_COMPILER_OFFSETOF(T, simpleStructure));
    }
};

template <>
struct Reflect<TestNamespace::ComplexStructure> : ReflectStruct<TestNamespace::ComplexStructure>
{
    template <typename MemberVisitor>
    static constexpr bool visit(MemberVisitor&& visitor)
    {
        return                                                                                                  //
            visitor(0, "f1", &T::f1, SC_COMPILER_OFFSETOF(T, f1)) and                                           //
            visitor(1, "simpleStructure", &T::simpleStructure, SC_COMPILER_OFFSETOF(T, simpleStructure)) and    //
            visitor(2, "simpleStructure2", &T::simpleStructure2, SC_COMPILER_OFFSETOF(T, simpleStructure2)) and //
            visitor(3, "f4", &T::f4, SC_COMPILER_OFFSETOF(T, f4)) and                                           //
            visitor(4, "intermediateStructure", &T::intermediateStructure,
                    SC_COMPILER_OFFSETOF(T, intermediateStructure)) and //
            visitor(5, "vectorOfStructs", &T::vectorOfStructs, SC_COMPILER_OFFSETOF(T, vectorOfStructs));
    }
};
} // namespace Reflection
} // namespace SC

namespace SC
{
struct ReflectionTest;
}

namespace TestNamespace
{
struct PackedStructWithArray;
struct PackedStruct;
struct UnpackedStruct;
struct NestedUnpackedStruct;
struct StructWithArrayPacked;
struct StructWithArrayUnpacked;
} // namespace TestNamespace

struct TestNamespace::PackedStructWithArray
{
    SC::uint8_t arrayValue[4] = {0, 1, 2, 3};
    float       floatValue    = 1.5f;
    SC::int64_t int64Value    = -13;
};
// Fails on GCC and MSVC in C++ 14 mode
#if !SC_META_ENABLE_AUTO_REFLECTION || !SC_LANGUAGE_CPP_AT_LEAST_20
SC_META_STRUCT_VISIT(TestNamespace::PackedStructWithArray)
SC_META_STRUCT_FIELD(0, arrayValue)
SC_META_STRUCT_FIELD(1, floatValue)
SC_META_STRUCT_FIELD(2, int64Value)
SC_META_STRUCT_LEAVE()
#endif

struct TestNamespace::PackedStruct
{
    float x = 0.0f, y = 0.0f, z = 0.0f;
};

#if !SC_META_ENABLE_AUTO_REFLECTION
SC_META_STRUCT_VISIT(TestNamespace::PackedStruct)
SC_META_STRUCT_FIELD(0, x)
SC_META_STRUCT_FIELD(1, y)
SC_META_STRUCT_FIELD(2, z)
SC_META_STRUCT_LEAVE()
#endif

struct TestNamespace::UnpackedStruct
{
    SC::int16_t x = 10;
    float       y = 2;
    float       z = 3;
};

#if !SC_META_ENABLE_AUTO_REFLECTION
SC_META_STRUCT_VISIT(TestNamespace::UnpackedStruct)
SC_META_STRUCT_FIELD(0, x)
SC_META_STRUCT_FIELD(1, y)
SC_META_STRUCT_FIELD(2, z)
SC_META_STRUCT_LEAVE()
#endif

struct TestNamespace::NestedUnpackedStruct
{
    UnpackedStruct unpackedMember;
};
#if !SC_META_ENABLE_AUTO_REFLECTION
SC_META_STRUCT_VISIT(TestNamespace::NestedUnpackedStruct)
SC_META_STRUCT_FIELD(0, unpackedMember);
SC_META_STRUCT_LEAVE()
#endif

struct TestNamespace::StructWithArrayPacked
{
    PackedStruct packedMember[3];
};
// Fails on Clang and GCC in C++ 14 mode
#if !SC_META_ENABLE_AUTO_REFLECTION || !SC_LANGUAGE_CPP_AT_LEAST_20
SC_META_STRUCT_VISIT(TestNamespace::StructWithArrayPacked)
SC_META_STRUCT_FIELD(0, packedMember);
SC_META_STRUCT_LEAVE()
#endif

struct TestNamespace::StructWithArrayUnpacked
{
    NestedUnpackedStruct unpackedMember[3];
};
#if !SC_META_ENABLE_AUTO_REFLECTION
SC_META_STRUCT_VISIT(TestNamespace::StructWithArrayUnpacked)
SC_META_STRUCT_FIELD(0, unpackedMember);
SC_META_STRUCT_LEAVE()
#endif

namespace SC
{
// TODO: Move printFlatSchema somewhere else
template <int NUM_TYPES, typename TypeInfo>
inline void printFlatSchema(Console& console, const TypeInfo (&type)[NUM_TYPES],
                            const Reflection::TypeStringView (&names)[NUM_TYPES])
{
    int typeIndex = 0;
    while (typeIndex < NUM_TYPES)
    {
        typeIndex += printTypes(console, typeIndex, type + typeIndex, names + typeIndex, 0) + 1;
    }
}

template <typename TypeInfo>
inline int printTypes(Console& console, int currentTypeIndex, const TypeInfo* type,
                      const Reflection::TypeStringView* typeName, int indentation)
{
    String        buffer(StringEncoding::Ascii);
    StringBuilder builder(buffer);
    SC_TRUST_RESULT(builder.append("[{:02}]", currentTypeIndex));
    for (int i = 0; i < indentation; ++i)
        SC_TRUST_RESULT(builder.append("\t"));
    SC_TRUST_RESULT(builder.append("[LinkIndex={:2}] {} ({} types)\n", currentTypeIndex,
                                   StringView(typeName->data, typeName->length, false, StringEncoding::Ascii),
                                   type->getNumberOfChildren()));
    for (int i = 0; i < indentation; ++i)
        SC_TRUST_RESULT(builder.append("\t"));
    SC_TRUST_RESULT(builder.append("{\n"));
    for (int idx = 0; idx < type->getNumberOfChildren(); ++idx)
    {
        auto& field     = type[idx + 1];
        auto  fieldName = typeName[idx + 1];
        SC_TRUST_RESULT(builder.append("[{:02}]", currentTypeIndex + idx + 1));

        for (int i = 0; i < indentation + 1; ++i)
            SC_TRUST_RESULT(builder.append("\t"));

        SC_TRUST_RESULT(builder.append("Type={}\tOffset={}\tSize={}\tName={}", (int)field.type,
                                       field.memberInfo.offsetInBytes, field.sizeInBytes,
                                       StringView(fieldName.data, fieldName.length, false, StringEncoding::Ascii)));
        if (field.hasValidLinkIndex())
        {
            SC_TRUST_RESULT(builder.append("\t[LinkIndex={}]", field.getLinkIndex()));
        }
        SC_TRUST_RESULT(builder.append("\n"));
    }
    for (int i = 0; i < indentation; ++i)
        SC_TRUST_RESULT(builder.append("\t"));

    SC_TRUST_RESULT(builder.append("}\n"));
    console.print(buffer.view());
    return type->getNumberOfChildren();
}
} // namespace SC

struct SC::ReflectionTest : public SC::TestCase
{
    ReflectionTest(SC::TestReport& report) : TestCase(report, "ReflectionTest")
    {
        using namespace SC;
        using namespace SC::Reflection;

        if (test_section("Print Simple structure"))
        {
            constexpr auto SimpleStructureFlatSchema = Schema::compile<TestNamespace::SimpleStructure>();
            printFlatSchema(report.console, SimpleStructureFlatSchema.typeInfos.values,
                            SimpleStructureFlatSchema.typeNames.values);
        }
        if (test_section("Print Complex structure"))
        {
            constexpr auto ComplexStructureFlatSchema = Schema::compile<TestNamespace::ComplexStructure>();
            printFlatSchema(report.console, ComplexStructureFlatSchema.typeInfos.values,
                            ComplexStructureFlatSchema.typeNames.values);
        }

        constexpr auto packedStructWithArray      = Schema::compile<TestNamespace::PackedStructWithArray>();
        constexpr auto packedStructWithArrayFlags = packedStructWithArray.typeInfos.values[0].structInfo;
        static_assert(packedStructWithArrayFlags.isPacked, "nestedPacked struct should be recursively packed");

        constexpr auto packedStruct      = Schema::compile<TestNamespace::PackedStruct>();
        constexpr auto packedStructFlags = packedStruct.typeInfos.values[0].structInfo;
        static_assert(packedStructFlags.isPacked, "nestedPacked struct should be recursively packed");

        constexpr auto unpackedStruct      = Schema::compile<TestNamespace::UnpackedStruct>();
        constexpr auto unpackedStructFlags = unpackedStruct.typeInfos.values[0].structInfo;
        static_assert(not(unpackedStructFlags.isPacked), "Unpacked struct should be recursively packed");

        constexpr auto nestedUnpackedStruct      = Schema::compile<TestNamespace::NestedUnpackedStruct>();
        constexpr auto nestedUnpackedStructFlags = nestedUnpackedStruct.typeInfos.values[0].structInfo;
        static_assert(not(nestedUnpackedStructFlags.isPacked), "nestedPacked struct should not be recursively packed");

        constexpr auto structWithArrayPacked      = Schema::compile<TestNamespace::StructWithArrayPacked>();
        constexpr auto structWithArrayPackedFlags = structWithArrayPacked.typeInfos.values[0].structInfo;
        static_assert(structWithArrayPackedFlags.isPacked,
                      "structWithArrayPacked struct should not be recursively packed");

        constexpr auto structWithArrayUnpacked      = Schema::compile<TestNamespace::StructWithArrayUnpacked>();
        constexpr auto structWithArrayUnpackedFlags = structWithArrayUnpacked.typeInfos.values[0].structInfo;
        static_assert(not(structWithArrayUnpackedFlags.isPacked),
                      "structWithArrayUnpacked struct should not be recursively packed");

        constexpr auto       className        = TypeToString<TestNamespace::ComplexStructure>::get();
        constexpr StringView classNameExected = "TestNamespace::ComplexStructure";
        constexpr StringView classNameView(className.data, className.length, false, StringEncoding::Ascii);
        static_assert(classNameView == classNameExected, "Please update SC::ClNm for your compiler");
        constexpr auto intName     = TypeToString<int>::get();
        constexpr auto intNameView = StringView(intName.data, intName.length, false, StringEncoding::Ascii);
        static_assert(intNameView == "int", "Please update SC::ClNm for your compiler");
    }
};

namespace SC
{
void runReflectionTest(SC::TestReport& report) { ReflectionTest test(report); }
} // namespace SC
