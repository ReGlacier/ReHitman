#include <Glacier/Serializer/ZPackedStream.h>
#include <gtest/gtest.h>

using namespace Glacier;

TEST(ZPackedStream, HeaderTagsMatchOriginalMagicValues)
{
    EXPECT_EQ(ZPackedStream::GetHeaderTag(ISerializerStream::PT_RawData), ZPackedStream::TAG_RawData);
    EXPECT_EQ(ZPackedStream::GetHeaderTag(ISerializerStream::PT_Char), ZPackedStream::TAG_Char);
    EXPECT_EQ(ZPackedStream::GetHeaderTag(ISerializerStream::PT_Bool), ZPackedStream::TAG_Bool);
    EXPECT_EQ(ZPackedStream::GetHeaderTag(ISerializerStream::PT_Int8), ZPackedStream::TAG_Int8);
    EXPECT_EQ(ZPackedStream::GetHeaderTag(ISerializerStream::PT_UInt8), ZPackedStream::TAG_Int8);
    EXPECT_EQ(ZPackedStream::GetHeaderTag(ISerializerStream::PT_Int16), ZPackedStream::TAG_Int16);
    EXPECT_EQ(ZPackedStream::GetHeaderTag(ISerializerStream::PT_UInt16), ZPackedStream::TAG_Int16);
    EXPECT_EQ(ZPackedStream::GetHeaderTag(ISerializerStream::PT_Int32), ZPackedStream::TAG_Int32);
    EXPECT_EQ(ZPackedStream::GetHeaderTag(ISerializerStream::PT_UInt32), ZPackedStream::TAG_Int32);
    EXPECT_EQ(ZPackedStream::GetHeaderTag(ISerializerStream::PT_Float32), ZPackedStream::TAG_Float32);
    EXPECT_EQ(ZPackedStream::GetHeaderTag(ISerializerStream::PT_Float64), ZPackedStream::TAG_Float64);
    EXPECT_EQ(ZPackedStream::GetHeaderTag(ISerializerStream::PT_String), ZPackedStream::TAG_String);
    EXPECT_EQ(ZPackedStream::GetHeaderTag(ISerializerStream::PT_Enum), ZPackedStream::TAG_Enum);
    EXPECT_EQ(ZPackedStream::GetHeaderTag(ISerializerStream::PT_Bitfield), ZPackedStream::TAG_Bitfield);
    EXPECT_EQ(ZPackedStream::GetHeaderTag(ISerializerStream::PT_Object), ZPackedStream::TAG_BeginObject);
    EXPECT_EQ(ZPackedStream::GetHeaderTag(ISerializerStream::PT_Reference), ZPackedStream::TAG_Reference);
    EXPECT_EQ(ZPackedStream::GetHeaderTag(ISerializerStream::PT_Container), ZPackedStream::TAG_Container);
}

TEST(ZPackedStream, NamedHeaderTagsMatchOriginalMagicValues)
{
    EXPECT_EQ(ZPackedStream::GetNamedHeaderTag(ISerializerStream::PT_RawData), ZPackedStream::TAG_NamedRawData);
    EXPECT_EQ(ZPackedStream::GetNamedHeaderTag(ISerializerStream::PT_Char), ZPackedStream::TAG_NamedChar);
    EXPECT_EQ(ZPackedStream::GetNamedHeaderTag(ISerializerStream::PT_Bool), ZPackedStream::TAG_NamedBool);
    EXPECT_EQ(ZPackedStream::GetNamedHeaderTag(ISerializerStream::PT_Int8), ZPackedStream::TAG_NamedInt8);
    EXPECT_EQ(ZPackedStream::GetNamedHeaderTag(ISerializerStream::PT_UInt8), ZPackedStream::TAG_NamedInt8);
    EXPECT_EQ(ZPackedStream::GetNamedHeaderTag(ISerializerStream::PT_Int16), ZPackedStream::TAG_NamedInt16);
    EXPECT_EQ(ZPackedStream::GetNamedHeaderTag(ISerializerStream::PT_UInt16), ZPackedStream::TAG_NamedInt16);
    EXPECT_EQ(ZPackedStream::GetNamedHeaderTag(ISerializerStream::PT_Int32), ZPackedStream::TAG_NamedInt32);
    EXPECT_EQ(ZPackedStream::GetNamedHeaderTag(ISerializerStream::PT_UInt32), ZPackedStream::TAG_NamedInt32);
    EXPECT_EQ(ZPackedStream::GetNamedHeaderTag(ISerializerStream::PT_Float32), ZPackedStream::TAG_NamedFloat32);
    EXPECT_EQ(ZPackedStream::GetNamedHeaderTag(ISerializerStream::PT_Float64), ZPackedStream::TAG_NamedFloat64);
    EXPECT_EQ(ZPackedStream::GetNamedHeaderTag(ISerializerStream::PT_String), ZPackedStream::TAG_NamedString);
    EXPECT_EQ(ZPackedStream::GetNamedHeaderTag(ISerializerStream::PT_Enum), ZPackedStream::TAG_NamedEnum);
    EXPECT_EQ(ZPackedStream::GetNamedHeaderTag(ISerializerStream::PT_Bitfield), ZPackedStream::TAG_NamedBitfield);
    EXPECT_EQ(ZPackedStream::GetNamedHeaderTag(ISerializerStream::PT_Object), ZPackedStream::TAG_BeginNamedObject);
    EXPECT_EQ(ZPackedStream::GetNamedHeaderTag(ISerializerStream::PT_Reference), ZPackedStream::TAG_NamedReference);
    EXPECT_EQ(ZPackedStream::GetNamedHeaderTag(ISerializerStream::PT_Container), ZPackedStream::TAG_NamedContainer);
}

TEST(ZPackedStream, FooterTagsOnlyExistForObjects)
{
    EXPECT_EQ(ZPackedStream::GetFooterTag(ISerializerStream::PT_Object), ZPackedStream::TAG_EndObject);
    EXPECT_EQ(ZPackedStream::GetFooterTag(ISerializerStream::PT_Int32), ZPackedStream::TAG_NULL);
}

TEST(ZPackedStream, NameAndNamedFlagMatchMagicValues)
{
    EXPECT_TRUE(ZPackedStream::IsNamed(ZPackedStream::TAG_NamedArray));
    EXPECT_TRUE(ZPackedStream::IsNamed(ZPackedStream::TAG_NamedInt32));
    EXPECT_TRUE(ZPackedStream::IsNamed(ZPackedStream::TAG_NamedBitfield));

    EXPECT_FALSE(ZPackedStream::IsNamed(ZPackedStream::TAG_Array));
    EXPECT_FALSE(ZPackedStream::IsNamed(ZPackedStream::TAG_Int32));
    EXPECT_FALSE(ZPackedStream::IsNamed(ZPackedStream::TAG_EndOfStream));

    EXPECT_STREQ(ZPackedStream::GetName(ZPackedStream::TAG_NamedString), "ZPackedStream::TAG_NamedString");
    EXPECT_STREQ(ZPackedStream::GetName(static_cast<ZPackedStream::ETag>(0x123456)), "ZPackedStream::TAG_UNKNOWN");
}
