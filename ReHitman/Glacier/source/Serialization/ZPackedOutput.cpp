#include <Glacier/Serializer/ZPackedOutput.h>
#include <Glacier/Serializer/ZOutputStreamBase.h>
#include <Glacier/Serializer/ZMemoryOutputStream.h>
#include <Glacier/Serializer/IDictionary.h>
#include <Glacier/Serializer/IDictionary_Serializerlib.h>
#include <Glacier/Serializer/ZDictionary_Serializerlib.h>
#include <Glacier/Serializer/ZFastDictionary_Serializerlib.h>
#include <Glacier/Serializer/ZPackedDictionary.h>
#include <Glacier/Serializer/ZSerializable.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>

#include <cstring>


namespace Glacier
{
    ZPackedOutput::~ZPackedOutput()
    {
        if (m_StringTable)
        {
            if (m_Content == CONTENT_SavedGame)
                ZUniMemory::Delete(static_cast<ZDictionary_Serializerlib*>(m_StringTable));
            else
                ZUniMemory::Delete(static_cast<ZFastDictionary_Serializerlib*>(m_StringTable));

            m_StringTable = nullptr;
            m_StringTableSave = nullptr;
        }

        if (m_DataStream)
        {
            if (m_TemporaryStream)
                ZUniMemory::Delete(m_TemporaryStream);
            else
                ZUniMemory::Delete(static_cast<ZOutputStream*>(m_DataStream));

            m_DataStream = nullptr;
            m_TemporaryStream = nullptr;
        }
    }

    void ZPackedOutput::Skip()
    {
        Tag(ZPackedStream::ETag::TAG_SkipMark);
    }

    void ZPackedOutput::End()
    {
        if (CheckType(TYPE_Tags))
        {
            Tag(ZPackedStream::ETag::TAG_EndOfStream);
        }

        if (CheckType(TYPE_Dictionary))
        {
            ZPackedDictionary_Serializerlib dictionary(m_Dictionary);
            dictionary.Save(m_Output);
        }

        if (CheckType(TYPE_StringTable))
        {
            m_StringTableSave->SaveAsTokenTable(m_Output);
        }

        m_Output.WriteWithEndianness<uint32_t>(m_NumberOfObjects);

        if (m_TemporaryStream)
        {
            m_TemporaryStream->CopyTo(m_Output);
        }

        ISerializerStream::End();
    }

    void ZPackedOutput::ExchangeContainer(const ZToken token, unsigned int* lSize)
    {
        ExchangeHeader(token, ISerializerStream::EPropertyType::PT_Container);
        m_DataStream->WriteWithEndianness<uint32_t>(*lSize);
    }

    void ZPackedOutput::ExchangeRaw(const ZToken token, void* pData, const uint32_t lSize)
    {
        ExchangeHeader(token, ISerializerStream::EPropertyType::PT_RawData);
        if (CheckType(TYPE_Tags))
        {
            m_DataStream->WriteWithEndianness<uint32_t>(lSize);
        }
        m_DataStream->WriteRaw((char*)pData, lSize);
    }

    void ZPackedOutput::ExchangeREF(const ZToken token, IREFConverter* pConverter)
    {
        uint32_t iRef = pConverter->GetRef();

        ExchangeHeader(token, ISerializerStream::EPropertyType::PT_UInt32);
        ExchangeData(iRef);
        ExchangeFooter(ISerializerStream::EPropertyType::PT_UInt32);
    }

    void ZPackedOutput::ExchangeReftab(const ZToken token, REFTAB* pRefTab)
    {
        uint32_t lSize = pRefTab->Count();
        ExchangeContainer(token, &lSize);

        for (const auto entry : (*pRefTab))
        {
            uint32_t iRef = entry;
            ExchangeHeader(ZToken::Void, ISerializerStream::EPropertyType::PT_UInt32);
            ExchangeData(iRef);
            ExchangeFooter(ISerializerStream::EPropertyType::PT_UInt32);
        }
    }
    
    ZToken ZPackedOutput::GetToken(const char* psToken)
    {
        if (CheckType(TYPE_Dictionary))
        {
            return m_Dictionary.GetToken(psToken);
        }

        return ZToken::Void;
    }

    void ZPackedOutput::BeginArray(const ZToken token, const unsigned int lSize)
    {
        // If no name (token) of array presented OR no string dictionary presented
        if (token == ZToken::Void || !CheckType(TYPE_Dictionary))
        {
            if (!CheckType(TYPE_Tags))
            {
                // And tags disabled - do nothing
                return;
            }

            // Put tag
            Tag(ZPackedStream::ETag::TAG_Array);
        }
        else
        {
            Tag(ZPackedStream::ETag::TAG_NamedArray);

            if (CheckType(TYPE_Dictionary))
            {
                m_DataStream->WriteWithEndianness<int32_t>(static_cast<int32_t>(token));
            }
        }

        // In any valid case we need to store length of array
        if (CheckType(TYPE_Tags))
        {
            m_DataStream->WriteWithEndianness<int32_t>(lSize);
        }
    }

    void ZPackedOutput::EndArray()
    {
        Tag(ZPackedStream::ETag::TAG_EndArray);
    }

    void ZPackedOutput::ExchangeHeader(const ZToken token, const EPropertyType eType)
    {
        if (token == ZToken::Void || !CheckType(TYPE_Dictionary))
        {
            if (CheckType(TYPE_Tags))
            {
                Tag(ZPackedStream::GetHeaderTag(eType));
            }
        }
        else
        {
            if (CheckType(TYPE_Tags))
            {
                Tag(ZPackedStream::GetNamedHeaderTag(eType));
            }

            if (CheckType(TYPE_Dictionary))
            {
                m_DataStream->WriteWithEndianness(static_cast<int32_t>(token));
            }
        }
    }

    void ZPackedOutput::ExchangeFooter(const EPropertyType eType)
    {
        if (eType == EPropertyType::PT_Object && CheckType(TYPE_Tags))
        {
            Tag(ZPackedStream::ETag::TAG_EndObject);
        }
    }

    void ZPackedOutput::ExchangeData(ZSerializable* pSerializable)
    {
        ++m_NumberOfObjects;
        
        pSerializable->PreSave(*this);
        pSerializable->SaveObject(*this);
        pSerializable->PostSave(*this);

        if (m_Content == CONTENT_SavedGame)
        {
            pSerializable->LoadSave(*this, true);
        }
    }

    void ZPackedOutput::ExchangeData(ZBitfieldBase * data, const ZEnumInfo * description)
    {
        if (!CheckType(TYPE_Tags))
        {
            m_DataStream->WriteWithEndianness(data->GetBitfield());
            return;
        }

        uint32_t count = 0;
        for (uint32_t i = 0; i < ZBitfieldBase::MAX_BITS_NR; ++i)
        {
            if (data->TestBit(i))
                ++count;
        }

        m_DataStream->WriteWithEndianness<uint32_t>(count);

        for (uint32_t i = 0; i < ZBitfieldBase::MAX_BITS_NR; ++i)
        {
            if (!data->TestBit(i))
                continue;

            auto* pFoundStrRepr = description->FindItem(1u << i);
            ZASSERT(pFoundStrRepr != nullptr);

            if (CheckType(TYPE_StringTable))
            {
                auto token = m_StringTable->GetToken(pFoundStrRepr);
                m_DataStream->WriteWithEndianness(static_cast<int32_t>(token));
            }
            else
            {
                const uint32_t length = static_cast<uint32_t>(strlen(pFoundStrRepr));
                m_DataStream->WriteWithEndianness<uint32_t>(length);
                m_DataStream->Write(pFoundStrRepr, length + 1);
            }
        }
    }

    void ZPackedOutput::ExchangeData(void * data, const ZEnumInfo * description)
    {
        if (CheckType(TYPE_StringEnums))
        {
            if (CheckType(TYPE_StringTable))
            {
                const char* name = description->FindItem(&data);
                ZASSERT(name);

                m_DataStream->WriteWithEndianness(static_cast<int32_t>(m_StringTable->GetToken(name)));
                return;
            }

            const char* name = description->FindItem(&data);
            ZASSERT(name);

            const uint32_t length = static_cast<uint32_t>(std::strlen(name));
            m_DataStream->WriteWithEndianness<uint32_t>(length);
            m_DataStream->Write(name, length + 1);
        }
        else
        {
            switch (description->GetSize())
            {
                case 1:
                {
                    const int32_t value = *static_cast<int8_t*>(data);
                    m_DataStream->WriteWithEndianness<int32_t>(value);
                    break;
                }

                case 2:
                {
                    const int32_t value = *static_cast<int16_t*>(data);
                    m_DataStream->WriteWithEndianness<int32_t>(value);
                    break;
                }

                case 4:
                {
                    m_DataStream->WriteWithEndianness<uint32_t>(*static_cast<uint32_t*>(data));
                    break;
                }

                default:
                {
                    ZASSERT(false && "Illegal enum size");
                }
            }
        }
    }

    void ZPackedOutput::ExchangeData(zstring& str)
    {
        if (CheckType(TYPE_StringTable))
        {
            auto token = m_StringTable->GetToken(str.c_str());
            m_DataStream->WriteWithEndianness(static_cast<int32_t>(token));
        }
        else
        {
            const uint32_t length = str.length();
            m_DataStream->WriteWithEndianness<uint32_t>(length);
            m_DataStream->Write(str.c_str(), length + 1);
        }
    }

    void ZPackedOutput::ExchangeData(const char *& pStr)
    {
        if (!CheckType(TYPE_StringTable))
        {
            ZASSERT(CheckType(TYPE_StringTable));
            return;
        }

        m_DataStream->WriteWithEndianness(static_cast<int32_t>(m_StringTable->GetToken(pStr)));
    }

    void ZPackedOutput::ExchangeData(double& dVal)
    {
        m_DataStream->WriteWithEndianness(dVal);
    }

    void ZPackedOutput::ExchangeData(float & fVal)
    {
        m_DataStream->WriteWithEndianness(fVal);
    }

    void ZPackedOutput::ExchangeData(uint8_t & iVal)
    {
        m_DataStream->WriteRaw((char*)&iVal, sizeof(uint8_t));
    }

    void ZPackedOutput::ExchangeData(int8_t & iVal)
    {
        m_DataStream->WriteRaw((char*)&iVal, sizeof(uint8_t));
    }

    void ZPackedOutput::ExchangeData(uint16_t & iVal)
    {
        m_DataStream->WriteWithEndianness(iVal);
    }

    void ZPackedOutput::ExchangeData(int16_t & iVal)
    {
        m_DataStream->WriteWithEndianness(iVal);
    }

    void ZPackedOutput::ExchangeData(uint32_t & iVal)
    {
        m_DataStream->WriteWithEndianness(iVal);
    }

    void ZPackedOutput::ExchangeData(int32_t & iVal)
    {
        m_DataStream->WriteWithEndianness(iVal);
    }
    
    void ZPackedOutput::ExchangeData(bool & bValue)
    {
        m_DataStream->WriteRaw((char*)&bValue, sizeof(bool));
    }
        
    ZPackedOutput::ZPackedOutput(IOutputStream& iStream, IOutputStream* pTempStream, ISerializerStream::EType eType, ISerializerStream::EContent eContent, bool bIsBigEndian)
        : IOutputSerializerStream(eType, eContent)
        , m_Output(iStream)
        , m_DataStream(nullptr)
        , m_TemporaryStream(nullptr)
        , m_NumberOfObjects(0)
        , m_Dictionary()
        , m_StringTable(nullptr)
        , m_StringTableSave(nullptr)
    {
        if (pTempStream)
        {
            m_DataStream = ZUniMemory::New<ZOutputStream>(*pTempStream);
        }
        else
        {
            m_TemporaryStream = ZUniMemory::New<ZMemoryOutputStream>();
            m_DataStream = m_TemporaryStream;
        }

        m_DataStream->SetBigEndian(bIsBigEndian);
        m_Output.SetBigEndian(bIsBigEndian);

        m_Output.Write("IOPacked v0.1", sizeof("IOPacked v0.1"));
        m_Output.Write<uint8_t>(bIsBigEndian ? 1u : 0u);
        m_Output.WriteWithEndianness<uint32_t>(static_cast<uint32_t>(m_Type));
        m_Output.WriteWithEndianness<uint32_t>(static_cast<uint32_t>(m_Content));

        if (eContent == CONTENT_SavedGame)
        {
            auto* dictionary = ZUniMemory::New<ZDictionary_Serializerlib>();
            m_StringTable = dictionary;
            m_StringTableSave = dictionary;
        }
        else
        {
            auto* dictionary = ZUniMemory::New<ZFastDictionary_Serializerlib>();
            m_StringTable = dictionary;
            m_StringTableSave = dictionary;
        }
    }

    void ZPackedOutput::Tag(ZPackedStream::ETag tag)
    {
        if (CheckType(TYPE_Tags))
        {
            m_DataStream->Write<uint8_t>(static_cast<uint8_t>(tag));
        }
    }
}
