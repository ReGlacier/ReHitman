#include <Glacier/Serializer/ZPackedInput.h>
#include <Glacier/Serializer/ZSerializable.h>
#include <Glacier/Serializer/ZPackedDictionary.h>
#include <Glacier/Serializer/ZSerializerVisitor.h>
#include <Glacier/ZSTL/zstring.h>
#include <Glacier/ZUniAssert.h>

#include <cstring>


namespace Glacier
{
    ZInputStreamBase::~ZInputStreamBase() = default;

    uint32_t ZInputStreamBase::Read(char* address, uint32_t count)
    {
        return ReadRaw(address, count);
    }

    void ZInputStreamBase::SetBigEndian(bool bigEndian)
    {
        m_ChangeEndianness = bigEndian;
    }

    ZPackedInput::~ZPackedInput() = default;

    ZPackedInput::ZPackedInput(ZInputStreamBase* pInput, ZInputStreamBase* pDataStream)
        : IInputSerializerStream()
        , m_Input(pInput)
        , m_DataStream(pDataStream ? pDataStream : pInput)
        , m_Dictionary()
        , m_StringTable()
        , m_BigEndian(false)
        , m_Finishing(false)
    {
        Init();
    }

    void ZPackedInput::Init()
    {
        CheckSignature();

        m_BigEndian = m_Input->Get<uint8_t>() != 0;
        m_Input->SetBigEndian(m_BigEndian);
        m_DataStream->SetBigEndian(m_BigEndian);

        m_Type = static_cast<EType>(m_Input->GetAndChangeEndiannessIfRequired<uint32_t>());
        m_Content = static_cast<EContent>(m_Input->GetAndChangeEndiannessIfRequired<uint32_t>());

        if (CheckType(TYPE_Dictionary))
        {
            m_Dictionary.Load(*m_Input);
        }

        if (CheckType(TYPE_StringTable))
        {
            m_StringTable.Load(*m_Input);
        }

        SetNumberOfObjects(m_Input->GetAndChangeEndiannessIfRequired<uint32_t>());
        m_Finishing = false;
    }

    void ZPackedInput::CheckSignature()
    {
        static constexpr char kSignature[] = "IOPacked v0.1";

        for (char expected : kSignature)
        {
            const char actual = m_Input->Get<char>();
            ZASSERT(actual == expected && "This is not a packed file"); // engine/serializerlib/packed_stream.cpp
        }
    }

    ZSuccess ZPackedInput::CheckTag(ZPackedStream::ETag eTag)
    {
        if (!CheckType(TYPE_Tags))
            return ZSuccess(true);

        const auto foundTag = static_cast<ZPackedStream::ETag>(m_DataStream->Get<uint8_t>());
        return ZSuccess(foundTag == eTag);
    }

    ZSuccess ZPackedInput::CheckName(ZToken token)
    {
        if (!CheckType(TYPE_Dictionary))
            return ZSuccess(true);

        const ZToken foundToken(m_DataStream->Get<uint32_t>());
        return ZSuccess(foundToken == token || token == ZToken::Joker);
    }

    void ZPackedInput::VisitStringTable(ISerializerVisitor* visitor, const ZToken& token)
    {
        if (CheckType(TYPE_StringTable))
        {
            uint32_t iIndex = m_DataStream->GetAndChangeEndiannessIfRequired<uint32_t>();
            visitor->Data(token, m_StringTable.GetWord(iIndex));
        }
        else
        {
            // Stored directly
            uint32_t iLength = m_DataStream->GetAndChangeEndiannessIfRequired<uint32_t>();
            char* pBuffer = (char*)alloca(iLength + 1); // I hate alloca

            m_DataStream->ReadRaw(pBuffer, iLength);
            pBuffer[iLength] = '\0';
            visitor->Data(token, pBuffer);
        }
    }

    void ZPackedInput::VisitStringTableBitfield(ISerializerVisitor* visitor, const ZToken& token)
    {
        static constexpr size_t kMaxStrings = 32;

        uint32_t iStringsCount = m_DataStream->GetAndChangeEndiannessIfRequired<uint32_t>();
        ZASSERT(iStringsCount <= kMaxStrings);

        // NOTE: We have possible RCE here. Need to write bugfix later
        const char* aStrings[kMaxStrings];

        for (int i = 0; i < iStringsCount; ++i)
        {
            uint32_t iStringIndex = m_DataStream->GetAndChangeEndiannessIfRequired<uint32_t>();
            aStrings[i] = m_StringTable.GetWord(iStringIndex);
        }

        if (iStringsCount < kMaxStrings)
        {
            std::memset(&aStrings[iStringsCount], 0, sizeof(char*) * (kMaxStrings - iStringsCount));
        }

        visitor->Bitfield(token, &aStrings);
    }

    void ZPackedInput::VisitBitfield(ISerializerVisitor* visitor, const ZToken& token)
    {
        static constexpr size_t kMaxStrings = 32;
        zstring aStrings[kMaxStrings];
        const char* paStrings[kMaxStrings];
        
        uint32_t iStringsCount = m_DataStream->GetAndChangeEndiannessIfRequired<uint32_t>();

        if (iStringsCount)
        {
            for (int i = 0; i < iStringsCount; ++i)
            {
                uint32_t iLength = m_DataStream->GetAndChangeEndiannessIfRequired<uint32_t>();

                char* pBuffer = (char*)alloca(iLength + 1); // I still hate alloca
                pBuffer[iLength] = '\0';
                m_DataStream->ReadRaw(pBuffer, iLength);

                aStrings[i] = pBuffer;
            }
        }
        
        if (iStringsCount < kMaxStrings)
        {
            std::memset(&paStrings[iStringsCount], 0, sizeof(char*) * (kMaxStrings - iStringsCount));
        }

        visitor->Bitfield(token, &paStrings);
    }

    ZToken ZPackedInput::GetToken(const char* word)
    {
        if (word)
        {
            return m_Dictionary.GetToken(word);
        }

        return ZToken::Joker;
    }

    void ZPackedInput::ExchangeRaw(const ZToken token, void* data, const unsigned int size)
    {
        ExchangeHeader(token, PT_RawData);

        if (CheckType(TYPE_Tags))
        {
            const uint32_t storedSize = m_DataStream->GetAndChangeEndiannessIfRequired<uint32_t>();
            ZASSERT(storedSize == size);
        }

        m_DataStream->Read(static_cast<char*>(data), size);
    }

    void ZPackedInput::ExchangeContainer(const ZToken token, unsigned int* count)
    {
        ExchangeHeader(token, PT_Container);
        *count = m_DataStream->GetAndChangeEndiannessIfRequired<uint32_t>();
    }

    void ZPackedInput::ExchangeData(ZSerializable* serializable)
    {
        _ExchangeObject(serializable);
    }

    void ZPackedInput::ExchangeData(const char*& data)
    {
        ZASSERT(CheckType(TYPE_StringTable));

        const ZToken token(m_DataStream->GetAndChangeEndiannessIfRequired<uint32_t>());
        data = m_StringTable.GetWord(token);
    }

    void ZPackedInput::ExchangeData(double& data)
    {
        data = m_DataStream->GetAndChangeEndiannessIfRequired<double>();
    }

    void ZPackedInput::ExchangeData(float& data)
    {
        data = m_DataStream->GetAndChangeEndiannessIfRequired<float>();
    }

    void ZPackedInput::ExchangeData(uint8_t& data)
    {
        data = m_DataStream->Get<uint8_t>();
    }

    void ZPackedInput::ExchangeData(int8_t& data)
    {
        data = m_DataStream->Get<int8_t>();
    }

    void ZPackedInput::ExchangeData(uint16_t& data)
    {
        data = m_DataStream->GetAndChangeEndiannessIfRequired<uint16_t>();
    }

    void ZPackedInput::ExchangeData(int16_t& data)
    {
        data = m_DataStream->GetAndChangeEndiannessIfRequired<int16_t>();
    }

    void ZPackedInput::ExchangeData(uint32_t& data)
    {
        data = m_DataStream->GetAndChangeEndiannessIfRequired<uint32_t>();
    }

    void ZPackedInput::ExchangeData(int32_t& data)
    {
        data = m_DataStream->GetAndChangeEndiannessIfRequired<int32_t>();
    }

    void ZPackedInput::ExchangeData(bool& data)
    {
        uint8_t value = 0;
        m_DataStream->Read(&value, 1);
        data = value != 0;
    }

    bool ZPackedInput::Visit(ISerializerVisitor* visitor)
    {
        using ETag = ZPackedStream::ETag; // small shortcut

        if (!ISerializerStream::CheckType(TYPE_Tags))
        {
            ZASSERT(false && "Visitor needs tags in the stream");
            return false;
        }

        // Read initial tag (previously known as 'opcodes')
        ZToken token = ZToken::Void;
        auto tag = static_cast<ETag>(m_DataStream->GetAndChangeEndiannessIfRequired<uint8_t>());

        if (ZPackedStream::IsNamed(tag))
        {
            // read token 'name'
            token = m_DataStream->GetAndChangeEndiannessIfRequired<uint32_t>();
        }

        switch (tag)
        {
            case ETag::TAG_Array:
            case ETag::TAG_NamedArray:
            {
                uint32_t iLength = m_DataStream->GetAndChangeEndiannessIfRequired<uint32_t>();
                visitor->BeginArray(token, iLength);
            }
            break;
            case ETag::TAG_BeginObject:
            case ETag::TAG_BeginNamedObject:
                visitor->BeginObject(token);
                break;
            case ETag::TAG_Reference:
            case ETag::TAG_NamedReference:
            {
                uint32_t iRef = m_DataStream->GetAndChangeEndiannessIfRequired<uint32_t>();
                visitor->Reference(token, iRef);
            }
            break;
            case ETag::TAG_Char:
            case ETag::TAG_Int8:
            case ETag::TAG_NamedChar:
            case ETag::TAG_NamedInt8:
            {
                uint8_t iValue =  0u;
                m_DataStream->ReadRaw((char*)&iValue, 1u);
                visitor->Data(token, &iValue);
            }
            break;
            case ETag::TAG_Bool:
            case ETag::TAG_NamedBool:
            {
                bool bValue = false;
                m_DataStream->ReadRaw((char*)&bValue, 1u);
                visitor->Data(token, &bValue);
            }
            break;
            case ETag::TAG_Int16:
            case ETag::TAG_NamedInt16:
            {
                int16_t iValue = 0;
                m_DataStream->ReadRaw((char*)&iValue, 2u);
                visitor->Data(token, &iValue);
            }
            break;
            case ETag::TAG_Int32:
            case ETag::TAG_NamedInt32:
            {
                int32_t iValue = 0;
                m_DataStream->ReadRaw((char*)&iValue, 4u);
                visitor->Data(token, &iValue);
            }
            break;
            case ETag::TAG_Float32:
            case ETag::TAG_NamedFloat32:
            {
                float fValue = 0.f;
                m_DataStream->ReadRaw((char*)&fValue, 4u);
                visitor->Data(token, &fValue);
            }
            break;
            case ETag::TAG_Float64:
            case ETag::TAG_NamedFloat64:
            {
                double dValue = 0.0;
                m_DataStream->ReadRaw((char*)&dValue, 8u);
                visitor->Data(token, &dValue);
            }
            break;
            case ETag::TAG_String:
            case ETag::TAG_NamedString:
            {
                VisitStringTable(visitor, token);
            }
            break;
            case ETag::TAG_RawData:
            case ETag::TAG_NamedRawData:
            {
                uint32_t iLength = m_DataStream->GetAndChangeEndiannessIfRequired<uint32_t>();
                auto* pBuffer = (char*)ZUniMemory::Allocate(iLength * sizeof(uint8_t));

                m_DataStream->ReadRaw(pBuffer, iLength);
                visitor->Data(token, (void*)pBuffer, iLength);

                ZUniMemory::Free(pBuffer);
            }
            break;
            case ETag::TAG_Enum:
            case ETag::TAG_NamedEnum:
            {
                if (CheckType(TYPE_StringEnums))
                {
                    VisitStringTable(visitor, token);
                }
                else
                {
                    uint32_t iValue = m_DataStream->GetAndChangeEndiannessIfRequired<uint32_t>();
                    visitor->Data(token, &iValue);
                }
            }
            break;
            case ETag::TAG_Bitfield:
            case ETag::TAG_NamedBitfield:
            {
                if (CheckType(TYPE_StringEnums))
                {
                    // this part is really weird
                    if (CheckType(TYPE_StringTable))
                    {
                        // here we will try to read array of references to string in string table
                        VisitStringTableBitfield(visitor, token);
                    }
                    else
                    {
                        // here we will try to read array of raw strings as 'array' and build bitfield
                        VisitBitfield(visitor, token);
                    }
                    // ^ in both cases we will construct array of strings and then call visitor->Bitfield(token, &aStrings[0]); 
                }
                else
                {
                    // In 99% we will drop into this line
                    uint32_t iRawValue = m_DataStream->GetAndChangeEndiannessIfRequired<uint32_t>();
                    visitor->Data(token, &iRawValue);
                }
            }
            break;
            case ETag::TAG_EndArray: visitor->EndArray(); break;
            case ETag::TAG_SkipMark: visitor->Skip(); break;
            case ETag::TAG_EndObject: visitor->EndObject(); break;
            case ETag::TAG_EndOfStream: visitor->End(); break;
            default:
                ZASSERT(false && "Unknown tag");
                return false;
        }

        return true;
    }

    void ZPackedInput::BeginArray(const ZToken token, const unsigned int count)
    {
        ZSuccess tag = token == ZToken::Void ? CheckTag(ZPackedStream::TAG_Array) : CheckTag(ZPackedStream::TAG_NamedArray);
        ZASSERT(static_cast<bool>(tag));

        if (token != ZToken::Void)
        {
            ZSuccess name = CheckName(token);
            ZASSERT(static_cast<bool>(name));
        }

        if (CheckType(TYPE_Tags))
        {
            const uint32_t storedCount = m_DataStream->Get<uint32_t>();
            ZASSERT(storedCount == count);
        }
    }

    void ZPackedInput::EndArray()
    {
        ZSuccess tag = CheckTag(ZPackedStream::TAG_EndArray);
        ZASSERT(static_cast<bool>(tag));
    }

    void ZPackedInput::ExchangeHeader(const ZToken token, const EPropertyType propertyType)
    {
        if (token == ZToken::Void)
        {
            ZSuccess tag = CheckTag(ZPackedStream::GetHeaderTag(propertyType));
            ZASSERT(static_cast<bool>(tag));
        }
        else
        {
            ZSuccess tag = CheckTag(static_cast<ZPackedStream::ETag>(ZPackedStream::GetNamedHeaderTag(propertyType)));
            ZASSERT(static_cast<bool>(tag));

            ZSuccess name = CheckName(token);
            ZASSERT(static_cast<bool>(name));
        }
    }

    void ZPackedInput::ExchangeFooter(const EPropertyType propertyType)
    {
        const ZPackedStream::ETag footerTag = ZPackedStream::GetFooterTag(propertyType);
        if (footerTag != ZPackedStream::TAG_NULL)
        {
            ZSuccess tag = CheckTag(footerTag);
            ZASSERT(static_cast<bool>(tag));
        }
    }

    void ZPackedInput::ExchangeData(ZBitfieldBase* data, const ZEnumInfo*)
    {
        uint32_t value = 0;
        m_DataStream->Read(&value, 1);
        data->SetBitfield(value);
    }

    void ZPackedInput::ExchangeData(void* data, const ZEnumInfo* description)
    {
        ZASSERT(description);
        m_DataStream->Read(static_cast<char*>(data), description->m_Size);
    }

    void ZPackedInput::ExchangeData(zstring&)
    {
        ZASSERT(false);
    }
}
