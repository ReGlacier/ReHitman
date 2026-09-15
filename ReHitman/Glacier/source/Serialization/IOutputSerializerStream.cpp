#include <Glacier/Serializer/IOutputSerializerStream.h>
#include <Glacier/ZSTL/REFTAB.h>


namespace Glacier
{
    IOutputSerializerStream::~IOutputSerializerStream() = default;
    IOutputSerializerStream::IOutputSerializerStream() = default;

    IOutputSerializerStream::IOutputSerializerStream(ISerializerStream::EType eType,ISerializerStream::EContent eContent)
        : ISerializerStream(eType, eContent)
    {
    }

    void IOutputSerializerStream::ExchangeREF(const ZToken token, IREFConverter* pConverter)
    {
        uint32_t iConvertedRef = pConverter->GetRef();
        Exchange(token, iConvertedRef);
    }

	void IOutputSerializerStream::ExchangeReftab(const ZToken token, REFTAB* pRefTab)
    {
        uint32_t iCount = pRefTab->Count();

        ExchangeContainer(token, &iCount);

        if (pRefTab)
        {
            for (auto entry : (*pRefTab))
            {
                ZToken token { ZToken::Void };
                Exchange(token, entry);
            }
        }
    }

    bool IOutputSerializerStream::IsSaving() const
    {
        return true;
    }

    void IOutputSerializerStream::SaveRaw(const char* psToken, void* pData, uint32_t lSize)
    {
        ZToken token = GetToken(psToken);
        SaveRaw(token, pData, lSize);
    }

    void IOutputSerializerStream::SaveRaw(ZToken token, void* pData, uint32_t lSize)
    {
        ExchangeRaw(token, pData, lSize);
    }

    void IOutputSerializerStream::SaveContainer(char const* psToken, uint32_t& lCapacity)
    {
        ZToken token = GetToken(psToken);
        SaveContainer(token, lCapacity);
    }
    
    void IOutputSerializerStream::SaveContainer(ZToken token, uint32_t& lCapacity)
    {
        ExchangeContainer(token, &lCapacity);
    }
}