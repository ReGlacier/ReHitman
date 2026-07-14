#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/Serializer/ZSerializable.h>
#include <Glacier/Serializer/ZSerializerVisitor.h>
#include <Glacier/ZSTL/REFTAB32.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    namespace
    {
        ZToken TokenFromName(ISerializerStream* pStream, const char* psName)
        {
            ZToken token;
            pStream->GetToken(&token, psName);
            return token;
        }

        template <typename T>
        void ExchangePrimitive(ISerializerStream* pStream, const ZToken token, T& data)
        {
            ISerializerStream::EPropertyType propertyType = pStream->GetPropertyType(data);
            pStream->ExchangeHeader(token, propertyType);
            pStream->ExchangeData(data);
            pStream->ExchangeFooter(propertyType);
        }

        template <typename T>
        void ExchangeArrayImpl(ISerializerStream* pStream, const ZToken token, T* data, uint32_t count)
        {
            pStream->BeginArray(token, count);

            for (uint32_t i = 0; i < count; ++i)
                pStream->Exchange(token, data[i]);

            pStream->EndArray();
        }
    }

    ISerializerStream::ISerializerStream()
        : m_Type(TYPE_ILLEGAL),
          m_Content(CONTENT_ILLEGAL),
          m_Status(true),
          m_Finished(false)
    {
    }

    ISerializerStream::ISerializerStream(EType eType, EContent eContent)
        : m_Type(eType),
          m_Content(eContent),
          m_Status(true),
          m_Finished(false)
    {
    }

    ISerializerStream::~ISerializerStream()
    {
        ZASSERT(m_Finished);

        while (auto* pNode = m_Cache.GetFirst())
        {
            auto* pCache = static_cast<ZTokenCache*>(pNode);
            pCache->Flush();
        }
    }

    void ISerializerStream::End()
    {
        m_Finished = true;
    }

    ZToken ISerializerStream::GetToken(ZTokenCache* pTokenCache)
    {
        if (pTokenCache->m_Stream != this)
        {
            if (pTokenCache->m_Stream)
                pTokenCache->Unlink();

            ZToken token;
            GetToken(&token, pTokenCache->m_Name);
            pTokenCache->m_Token = token;
            m_Cache.m_Head.AddPrev(pTokenCache);
            pTokenCache->m_Stream = this;
        }

        return *pTokenCache;
    }

    ZToken ISerializerStream::GetToken(ZTokenCache& tokenCache)
    {
        return GetToken(&tokenCache);
    }

    bool ISerializerStream::IsLoading() const
    {
        return !IsSaving();
    }

    bool ISerializerStream::TestStreamFilter(uint32_t lStreamFilter) const
    {
        return (lStreamFilter & (1u << m_Content)) != 0;
    }

    bool ISerializerStream::CheckType(uint32_t lTypeFilter) const
    {
        return (static_cast<uint32_t>(m_Type) & lTypeFilter) != 0;
    }

    void ISerializerStream::Exchange(const char* psName, uint16_t& data) { Exchange(TokenFromName(this, psName), data); }
    void ISerializerStream::Exchange(const ZToken token, uint16_t& data) { ExchangePrimitive(this, token, data); }
    void ISerializerStream::Exchange(const char* psName, uint32_t& data) { Exchange(TokenFromName(this, psName), data); }
    void ISerializerStream::Exchange(const ZToken token, uint32_t& data) { ExchangePrimitive(this, token, data); }
    void ISerializerStream::Exchange(const char* psName, uint8_t& data) { Exchange(TokenFromName(this, psName), data); }
    void ISerializerStream::Exchange(const ZToken token, uint8_t& data) { ExchangePrimitive(this, token, data); }
    void ISerializerStream::Exchange(const char* psName, int16_t& data) { Exchange(TokenFromName(this, psName), data); }
    void ISerializerStream::Exchange(const ZToken token, int16_t& data) { ExchangePrimitive(this, token, data); }
    void ISerializerStream::Exchange(const char* psName, int32_t& data) { Exchange(TokenFromName(this, psName), data); }
    void ISerializerStream::Exchange(const ZToken token, int32_t& data) { ExchangePrimitive(this, token, data); }
    void ISerializerStream::Exchange(const char* psName, float& data) { Exchange(TokenFromName(this, psName), data); }
    void ISerializerStream::Exchange(const ZToken token, float& data) { ExchangePrimitive(this, token, data); }
    void ISerializerStream::Exchange(const char* psName, char& data) { Exchange(TokenFromName(this, psName), data); }
    void ISerializerStream::Exchange(const ZToken token, char& data)
    {
        EPropertyType propertyType = GetPropertyType(data);
        ExchangeHeader(token, propertyType);
        ExchangeData(reinterpret_cast<int8_t&>(data));
        ExchangeFooter(propertyType);
    }
    void ISerializerStream::Exchange(const char* psName, const char*& data) { Exchange(TokenFromName(this, psName), data); }
    void ISerializerStream::Exchange(ZTokenCache& tokenCache, const char*& data) { Exchange(GetToken(tokenCache), data); }
    void ISerializerStream::Exchange(const ZToken token, const char*& data) { ExchangePrimitive(this, token, data); }
    void ISerializerStream::Exchange(const char* psName, bool& data) { Exchange(TokenFromName(this, psName), data); }
    void ISerializerStream::Exchange(const ZToken token, bool& data) { ExchangePrimitive(this, token, data); }

    void ISerializerStream::ExchangeArray(const char* psName, uint32_t* data, uint32_t count) { ExchangeArray(TokenFromName(this, psName), data, count); }
    void ISerializerStream::ExchangeArray(const ZToken token, uint32_t* data, uint32_t count) { ExchangeArrayImpl(this, token, data, count); }
    void ISerializerStream::ExchangeArray(const char* psName, uint8_t* data, uint32_t count) { ExchangeArray(TokenFromName(this, psName), data, count); }
    void ISerializerStream::ExchangeArray(const ZToken token, uint8_t* data, uint32_t count) { ExchangeArrayImpl(this, token, data, count); }
    void ISerializerStream::ExchangeArray(const char* psName, int16_t* data, uint32_t count) { ExchangeArray(TokenFromName(this, psName), data, count); }
    void ISerializerStream::ExchangeArray(const ZToken token, int16_t* data, uint32_t count) { ExchangeArrayImpl(this, token, data, count); }
    void ISerializerStream::ExchangeArray(const char* psName, int32_t* data, uint32_t count) { ExchangeArray(TokenFromName(this, psName), data, count); }
    void ISerializerStream::ExchangeArray(const ZToken token, int32_t* data, uint32_t count) { ExchangeArrayImpl(this, token, data, count); }
    void ISerializerStream::ExchangeArray(const char* psName, float* data, uint32_t count) { ExchangeArray(TokenFromName(this, psName), data, count); }
    void ISerializerStream::ExchangeArray(const ZToken token, float* data, uint32_t count) { ExchangeArrayImpl(this, token, data, count); }
    void ISerializerStream::ExchangeArray(const char* psName, const char** data, uint32_t count) { ExchangeArray(TokenFromName(this, psName), data, count); }
    void ISerializerStream::ExchangeArray(const ZToken token, const char** data, uint32_t count) { ExchangeArrayImpl(this, token, data, count); }
    void ISerializerStream::ExchangeArray(const char* psName, REFTAB32* data, uint32_t count) { ExchangeArray(TokenFromName(this, psName), data, count); }
    void ISerializerStream::ExchangeArray(const ZToken token, REFTAB32* data, uint32_t count)
    {
        for (uint32_t i = 0; i < count; ++i)
            ExchangeReftab(token, &data[i]);
    }

    void ISerializerStream::ExchangeArray(const ZToken token, REFTAB* data, uint32_t count)
    {
        for (uint32_t i = 0; i < count; ++i)
            ExchangeReftab(token, &data[i]);
    }

    void ISerializerStream::ExchangeRaw(const char* psName, void* data, uint32_t size)
    {
        ExchangeRaw(TokenFromName(this, psName), data, size);
    }

    void ISerializerStream::ExchangeEnum(const char* psName, void* data, uint32_t count, const ZEnumInfo& enumInfo)
    {
        ExchangeEnum(TokenFromName(this, psName), data, count, enumInfo);
    }

    void ISerializerStream::ExchangeEnum(const ZToken token, void* data, uint32_t count, const ZEnumInfo& enumInfo)
    {
        auto* bytes = static_cast<uint8_t*>(data);
        for (uint32_t i = 0; i < count; ++i)
        {
            ExchangeHeader(token, PT_Enum);
            ExchangeData(bytes + i * enumInfo.m_Size, &enumInfo);
            ExchangeFooter(PT_Enum);
        }
    }

    void ISerializerStream::ExchangeContainer(const char* psName, uint32_t& count)
    {
        ExchangeContainer(TokenFromName(this, psName), &count);
    }

    void ISerializerStream::ExchangeContainer(ZTokenCache& tokenCache, uint32_t& count)
    {
        ExchangeContainer(GetToken(tokenCache), &count);
    }

    void ISerializerStream::ExchangeBitfield(const ZToken token, ZBitfieldBase& bitfield, const ZEnumInfo& enumInfo)
    {
        ExchangeHeader(token, PT_Bitfield);
        ExchangeData(&bitfield, &enumInfo);
        ExchangeFooter(PT_Bitfield);
    }

    IInputSerializerStream::IInputSerializerStream()
        : ISerializerStream(),
          PostProcessWrite(0)
    {
    }

    IInputSerializerStream::~IInputSerializerStream()
    {
    }

    bool IInputSerializerStream::IsSaving() const
    {
        return false;
    }

    void IInputSerializerStream::_ExchangeObject(ZSerializable* pSerializable)
    {
        pSerializable->PreLoad(*this);
        pSerializable->LoadObject(*this);

        if (!pSerializable->PostLoad(*this))
            m_ProcessTable[PostProcessWrite++] = pSerializable;

        if (m_Content == CONTENT_SavedGame)
            pSerializable->PostProcess(1u << m_Content, 0);
    }

    void IInputSerializerStream::SkipObject()
    {
        ZSerializerVisitor_SkipObject visitor;

        do
        {
            ZASSERT(Visit(&visitor));
        }
        while (visitor);
    }

    void IInputSerializerStream::Skip()
    {
        ZSerializerVisitor_SkipToNextMark visitor;

        do
        {
            ZASSERT(Visit(&visitor));
        }
        while (visitor);
    }

    void IInputSerializerStream::SetNumberOfObjects(uint32_t lCount)
    {
        ZSerializable* pInit = nullptr;
        m_ProcessTable.Resize(lCount, pInit);
    }

    void IInputSerializerStream::PostProcess()
    {
        uint32_t pass = 0;

        while (PostProcessWrite)
        {
            uint32_t count = PostProcessWrite;
            PostProcessWrite = 0;

            for (uint32_t i = 0; i < count; ++i)
            {
                ZSerializable* pSerializable = m_ProcessTable[i];
                if (!pSerializable->PostProcess(1u << m_Content, pass))
                    m_ProcessTable[PostProcessWrite++] = pSerializable;
            }

            ++pass;
            ZASSERT(pass < 42);
        }
    }

    bool IInputSerializerStream::IsItInDebugMode() const
    {
        // original code expect missing variable
        return false;
    }

    void IInputSerializerStream::ExchangeREF(const ZToken token, IREFConverter* pRefConverter)
    {
        uint32_t ref = 0;
        Exchange(token, ref);
        pRefConverter->SetRef(ref);
    }

    void IInputSerializerStream::ExchangeReftab(const ZToken token, REFTAB* pRefTab)
    {
        uint32_t count = 0;
        ExchangeContainer(token, &count);
        pRefTab->Clear();

        for (uint32_t i = 0; i < count; ++i)
        {
            uint32_t ref = 0;
            Exchange(ZToken::Void, ref);
            pRefTab->Add(ref);
        }
    }
}
