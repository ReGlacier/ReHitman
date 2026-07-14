#include <Glacier/Serializer/ZPackedInput.h>
#include <Glacier/Serializer/ZSerializable.h>
#include <Glacier/Serializer/ZSerializerVisitor.h>
#include <Glacier/ZSTL/zstring.h>
#include <Glacier/ZUniAssert.h>

#include <cstring>


namespace Glacier
{
    ZInputStreamBase::~ZInputStreamBase() = default;

    uint32_t ZInputStreamBase::Read(char* address, uint32_t count)
    {
        if (m_ChangeEndianness)
            return ReadChangeEndianness(address, count, 0xFFFFFFFFu);

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

        m_Type = static_cast<EType>(m_Input->Get<uint32_t>());
        m_Content = static_cast<EContent>(m_Input->Get<uint32_t>());

        // Dictionary and token-table loading need their PS2 Load methods reversed.
        ZASSERT(!CheckType(TYPE_Dictionary));
        ZASSERT(!CheckType(TYPE_StringTable));

        SetNumberOfObjects(m_Input->Get<uint32_t>());
        m_Finishing = false;
    }

    void ZPackedInput::CheckSignature()
    {
        static constexpr char Signature[] = "IOPacked v0.1";

        for (char expected : Signature)
        {
            const char actual = m_Input->Get<char>();
            ZASSERT(actual == expected);
        }
    }

    ZSuccess ZPackedInput::CheckTag(ZPackedStream::ETag eTag)
    {
        if (!CheckType(TYPE_Tags))
            return ZSuccess(true);

        const auto foundTag = static_cast<ZPackedStream::ETag>(m_DataStream->Get<uint32_t>());
        return ZSuccess(foundTag == eTag);
    }

    ZSuccess ZPackedInput::CheckName(ZToken token)
    {
        if (!CheckType(TYPE_Dictionary))
            return ZSuccess(true);

        const ZToken foundToken(m_DataStream->Get<uint32_t>());
        return ZSuccess(foundToken == token || token == ZToken::Joker);
    }

    ZToken* ZPackedInput::GetToken(ZToken* result, const char* word)
    {
        if (word)
            return m_Dictionary.GetToken(result, word);

        *result = ZToken::Joker;
        return result;
    }

    void ZPackedInput::ExchangeRaw(const ZToken token, void* data, const unsigned int size)
    {
        ExchangeHeader(token, PT_RawData);

        if (CheckType(TYPE_Tags))
        {
            const uint32_t storedSize = m_DataStream->Get<uint32_t>();
            ZASSERT(storedSize == size);
        }

        m_DataStream->Read(static_cast<char*>(data), size);
    }

    void ZPackedInput::ExchangeContainer(const ZToken token, unsigned int* count)
    {
        ExchangeHeader(token, PT_Container);
        m_DataStream->Read(count, 1);
    }

    void ZPackedInput::ExchangeData(ZSerializable* serializable)
    {
        _ExchangeObject(serializable);
    }

    void ZPackedInput::ExchangeData(const char*& data)
    {
        ZASSERT(CheckType(TYPE_StringTable));

        const ZToken token(m_DataStream->Get<uint32_t>());
        data = m_StringTable.GetWord(token);
    }

    void ZPackedInput::ExchangeData(double& data)
    {
        m_DataStream->Read(&data, 1);
    }

    void ZPackedInput::ExchangeData(float& data)
    {
        m_DataStream->Read(&data, 1);
    }

    void ZPackedInput::ExchangeData(uint8_t& data)
    {
        m_DataStream->Read(&data, 1);
    }

    void ZPackedInput::ExchangeData(int8_t& data)
    {
        m_DataStream->Read(&data, 1);
    }

    void ZPackedInput::ExchangeData(uint16_t& data)
    {
        m_DataStream->Read(&data, 1);
    }

    void ZPackedInput::ExchangeData(int16_t& data)
    {
        m_DataStream->Read(&data, 1);
    }

    void ZPackedInput::ExchangeData(uint32_t& data)
    {
        m_DataStream->Read(&data, 1);
    }

    void ZPackedInput::ExchangeData(int32_t& data)
    {
        m_DataStream->Read(&data, 1);
    }

    void ZPackedInput::ExchangeData(bool& data)
    {
        uint8_t value = 0;
        m_DataStream->Read(&value, 1);
        data = value != 0;
    }

    bool ZPackedInput::Visit(ISerializerVisitor*)
    {
        ZASSERT(false);
        return false;
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
