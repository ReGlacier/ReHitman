#pragma once

#include <Glacier/Serializer/ISerializerStream.h>
#include <cstdint>


namespace Glacier
{
    struct ZPackedStream
    {
        // In PS2 it's different, based on PC/XBox
        enum ETag : int32_t
        {
            TAG_NULL = 0x0,
            TAG_FLAG_Named = 0x80,
            TAG_Array = 0x1,
            TAG_BeginObject = 0x2,
            TAG_Reference = 0x3,
            TAG_Container = 0x4,
            TAG_Char = 0x5,
            TAG_Bool = 0x6,
            TAG_Int8 = 0x7,
            TAG_Int16 = 0x8,
            TAG_Int32 = 0x9,
            TAG_Float32 = 0xA,
            TAG_Float64 = 0xB,
            TAG_String = 0xC,
            TAG_RawData = 0xD,
            TAG_Enum = 0xE,
            TAG_Bitfield = 0xF,
            TAG_EndArray = 0x7C,
            TAG_SkipMark = 0x7D,
            TAG_EndObject = 0x7E,
            TAG_EndOfStream = 0x7F,
            TAG_NamedArray = 0x81,
            TAG_BeginNamedObject = 0x82,
            TAG_NamedReference = 0x83,
            TAG_NamedContainer = 0x84,
            TAG_NamedChar = 0x85,
            TAG_NamedBool = 0x86,
            TAG_NamedInt8 = 0x87,
            TAG_NamedInt16 = 0x88,
            TAG_NamedInt32 = 0x89,
            TAG_NamedFloat32 = 0x8A,
            TAG_NamedFloat64 = 0x8B,
            TAG_NamedString = 0x8C,
            TAG_NamedRawData = 0x8D,
            TAG_NamedEnum = 0x8E,
            TAG_NamedBitfield = 0x8F,
        };

        static bool IsNamed(ETag eTag);
        static ETag GetNamedHeaderTag(ISerializerStream::EPropertyType ePropType);
        static const char* GetName(ETag eTag);
        static ETag GetHeaderTag(ISerializerStream::EPropertyType ePropType);
        static ETag GetFooterTag(ISerializerStream::EPropertyType ePropType);
    };

    struct ZSuccess
    {
        ZSuccess(bool success = true);
        ZSuccess(const ZSuccess& other);
        ~ZSuccess();

        explicit operator bool();

        const bool m_Success;
        bool m_Handled;
    };
    RE_VERIFY_SIZE(ZSuccess, 0x2);
}
