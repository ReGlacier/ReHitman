#pragma once

#include <Glacier/Serializer/ISerializerStream.h>


namespace Glacier
{
    struct IOutputSerializerStream : public ISerializerStream
    {
        // vtbl
        bool IsSaving() const override;
        void ExchangeREF(const ZToken token, IREFConverter* pConverter) override;
		void ExchangeReftab(const ZToken token, REFTAB* pRefTab) override;

        // methods
        ~IOutputSerializerStream() override;
        IOutputSerializerStream();
        IOutputSerializerStream(ISerializerStream::EType eType,ISerializerStream::EContent eContent);
        
        void SaveRaw(const char* psToken, void* pData, uint32_t lSize);
        void SaveRaw(ZToken token, void*pData, uint32_t lSize);
        void SaveContainer(char const* psToken, uint32_t& lCapacity);
        void SaveContainer(ZToken token, uint32_t& lCapacity);

        template <typename T>
        void SaveArray(ZToken token, const T* pArray, uint32_t lSize)
        {
            ISerializerStream::ExchangeArray(token, pArray, lSize);
        }

        template <typename T>
        void SaveArray(const char* psToken, const T* pArray, uint32_t lSize)
        {
            ZToken token = GetToken(psToken);
            ISerializerStream::ExchangeArray(token, pArray, lSize);
        }

        template <typename T>
        void Save(ZToken token, const T& value)
        {
            ISerializerStream::Exchage(token, value);
        }

        template <typename T>
        void Save(const char* psToken, const T& value)
        {
            ZToken token = GetToken(psToken);
            ISerializerStream::Exchage(token, value);
        }

        template <typename T, size_t N>
        void Save(ZToken token, const float arr[N])
        {
            ISerializerStream::ExchangeArray<T>(token, &arr[0], N);
        }
        
        // NOTE: Need support template spec of T[N] | float[3] as example
    };
}