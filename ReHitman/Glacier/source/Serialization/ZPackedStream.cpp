#include <Glacier/Serializer/ZPackedStream.h>
#include <Glacier/ZUniAssert.h>

#include <cstdio>


namespace Glacier
{
    bool ZPackedStream::IsNamed(ETag eTag)
    {
        return (static_cast<int32_t>(eTag) & TAG_FLAG_Named) != 0;
    }

    uint32_t ZPackedStream::GetNamedHeaderTag(ISerializerStream::EPropertyType ePropType)
    {
        switch (ePropType)
        {
            case ISerializerStream::PT_RawData: return TAG_NamedRawData;
            case ISerializerStream::PT_Char: return TAG_NamedChar;
            case ISerializerStream::PT_Bool: return TAG_NamedBool;
            case ISerializerStream::PT_Int8: return TAG_NamedInt8;
            case ISerializerStream::PT_UInt8: return TAG_NamedInt8;
            case ISerializerStream::PT_Int16: return TAG_NamedInt16;
            case ISerializerStream::PT_UInt16: return TAG_NamedInt16;
            case ISerializerStream::PT_Int32: return TAG_NamedInt32;
            case ISerializerStream::PT_UInt32: return TAG_NamedInt32;
            case ISerializerStream::PT_Float32: return TAG_NamedFloat32;
            case ISerializerStream::PT_Float64: return TAG_NamedFloat64;
            case ISerializerStream::PT_String: return TAG_NamedString;
            case ISerializerStream::PT_Enum: return TAG_NamedEnum;
            case ISerializerStream::PT_Bitfield: return TAG_NamedBitfield;
            case ISerializerStream::PT_Object: return TAG_BeginNamedObject;
            case ISerializerStream::PT_Reference: return TAG_NamedReference;
            case ISerializerStream::PT_Container: return TAG_NamedContainer;
            default: return TAG_NULL;
        }
    }

    const char* ZPackedStream::GetName(ETag eTag)
    {
        switch (eTag)
        {
            case TAG_Array: return "ZPackedStream::TAG_Array";
            case TAG_BeginObject: return "ZPackedStream::TAG_BeginObject";
            case TAG_Reference: return "ZPackedStream::TAG_Reference";
            case TAG_Container: return "ZPackedStream::TAG_Container";
            case TAG_Char: return "ZPackedStream::TAG_Char";
            case TAG_Bool: return "ZPackedStream::TAG_Bool";
            case TAG_Int8: return "ZPackedStream::TAG_Int8";
            case TAG_Int16: return "ZPackedStream::TAG_Int16";
            case TAG_Int32: return "ZPackedStream::TAG_Int32";
            case TAG_Float32: return "ZPackedStream::TAG_Float32";
            case TAG_Float64: return "ZPackedStream::TAG_Float64";
            case TAG_String: return "ZPackedStream::TAG_String";
            case TAG_RawData: return "ZPackedStream::TAG_RawData";
            case TAG_Enum: return "ZPackedStream::TAG_Enum";
            case TAG_Bitfield: return "ZPackedStream::TAG_Bitfield";
            case TAG_EndArray: return "ZPackedStream::TAG_EndArray";
            case TAG_SkipMark: return "ZPackedStream::TAG_SkipMark";
            case TAG_EndObject: return "ZPackedStream::TAG_EndObject";
            case TAG_EndOfStream: return "ZPackedStream::TAG_EndOfStream";
            case TAG_NamedArray: return "ZPackedStream::TAG_NamedArray";
            case TAG_BeginNamedObject: return "ZPackedStream::TAG_BeginNamedObject";
            case TAG_NamedReference: return "ZPackedStream::TAG_NamedReference";
            case TAG_NamedContainer: return "ZPackedStream::TAG_NamedContainer";
            case TAG_NamedChar: return "ZPackedStream::TAG_NamedChar";
            case TAG_NamedBool: return "ZPackedStream::TAG_NamedBool";
            case TAG_NamedInt8: return "ZPackedStream::TAG_NamedInt8";
            case TAG_NamedInt16: return "ZPackedStream::TAG_NamedInt16";
            case TAG_NamedInt32: return "ZPackedStream::TAG_NamedInt32";
            case TAG_NamedFloat32: return "ZPackedStream::TAG_NamedFloat32";
            case TAG_NamedFloat64: return "ZPackedStream::TAG_NamedFloat64";
            case TAG_NamedString: return "ZPackedStream::TAG_NamedString";
            case TAG_NamedRawData: return "ZPackedStream::TAG_NamedRawData";
            case TAG_NamedEnum: return "ZPackedStream::TAG_NamedEnum";
            case TAG_NamedBitfield: return "ZPackedStream::TAG_NamedBitfield";
            default: return "ZPackedStream::TAG_UNKNOWN";
        }
    }

    ZPackedStream::ETag ZPackedStream::GetHeaderTag(ISerializerStream::EPropertyType ePropType)
    {
        switch (ePropType)
        {
            case ISerializerStream::PT_RawData: return TAG_RawData;
            case ISerializerStream::PT_Char: return TAG_Char;
            case ISerializerStream::PT_Bool: return TAG_Bool;
            case ISerializerStream::PT_Int8: return TAG_Int8;
            case ISerializerStream::PT_UInt8: return TAG_Int8;
            case ISerializerStream::PT_Int16: return TAG_Int16;
            case ISerializerStream::PT_UInt16: return TAG_Int16;
            case ISerializerStream::PT_Int32: return TAG_Int32;
            case ISerializerStream::PT_UInt32: return TAG_Int32;
            case ISerializerStream::PT_Float32: return TAG_Float32;
            case ISerializerStream::PT_Float64: return TAG_Float64;
            case ISerializerStream::PT_String: return TAG_String;
            case ISerializerStream::PT_Enum: return TAG_Enum;
            case ISerializerStream::PT_Bitfield: return TAG_Bitfield;
            case ISerializerStream::PT_Object: return TAG_BeginObject;
            case ISerializerStream::PT_Reference: return TAG_Reference;
            case ISerializerStream::PT_Container: return TAG_Container;
            default: return TAG_NULL;
        }
    }

    ZPackedStream::ETag ZPackedStream::GetFooterTag(ISerializerStream::EPropertyType ePropType)
    {
        if (ePropType == ISerializerStream::PT_Object)
            return TAG_EndObject;

        return TAG_NULL;
    }

    ZSuccess::ZSuccess(bool success)
        : m_Success(success)
        , m_Handled(false)
    {
    }

    ZSuccess::ZSuccess(const ZSuccess& other)
        : m_Success(other.m_Success)
        , m_Handled(false)
    {
        const_cast<ZSuccess&>(other).m_Handled = true;
    }

    ZSuccess::~ZSuccess()
    {
        if (!m_Success && !m_Handled)
            std::printf("Return code of a failed function wasn't handled by the caller function\n");
    }

    ZSuccess::operator bool()
    {
        m_Handled = true;
        return m_Success;
    }
}
