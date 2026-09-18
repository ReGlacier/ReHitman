#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Serializer/ZOutputStream.h>
#include <Glacier/Serializer/ZPackedStream.h>
#include <Glacier/Serializer/ZFastDictionary.h>
#include <Glacier/Serializer/IOutputSerializerStream.h>
#include <cstdint>


namespace Glacier
{
    //fwds
    struct IDictionary;
    struct ZOutputStreamBase;
    struct ZMemoryOutputStream;
    struct IDictionary_Serializerlib;
    
    struct ZPackedOutput : public IOutputSerializerStream, public ZPackedStream
    {
        // vtbl
        ~ZPackedOutput() override;

        void Skip() override;
		void End() override;
		void ExchangeContainer(const ZToken, unsigned int*) override;
		void ExchangeRaw(const ZToken, void*, const unsigned int) override;
        void ExchangeREF(const ZToken, IREFConverter *) override;
        void ExchangeReftab(const ZToken, REFTAB*) override;
        ZToken GetToken(const char* psToken) override;
		void BeginArray(const ZToken, const unsigned int) override;
		void EndArray() override;
		void ExchangeHeader(const ZToken, const EPropertyType) override;
		void ExchangeFooter(const EPropertyType) override;
		void ExchangeData(ZSerializable*) override;
		void ExchangeData(ZBitfieldBase * data, const ZEnumInfo * description) override;
		void ExchangeData(void * data, const ZEnumInfo * description) override;
		void ExchangeData(zstring&) override;	
		void ExchangeData(const char *&) override;
		void ExchangeData(double &) override;
		void ExchangeData(float &) override;
		void ExchangeData(uint8_t &) override;
		void ExchangeData(int8_t &) override;
		void ExchangeData(uint16_t &) override;
		void ExchangeData(int16_t &) override;
		void ExchangeData(uint32_t &) override;
		void ExchangeData(int32_t &) override;
		void ExchangeData(bool &) override;

        // methods
        ZPackedOutput(IOutputStream& iStream, IOutputStream* pTempStream, ISerializerStream::EType eType, ISerializerStream::EContent eContent, bool bIsBigEndian);

        /// @brief A shortcut to write tag into stream if tags supported by stream options
        /// @param tag 
        void Tag(ZPackedStream::ETag tag);

        // members
        ZOutputStream m_Output;
        ZOutputStreamBase* m_DataStream;
        ZMemoryOutputStream* m_TemporaryStream;
        uint32_t m_NumberOfObjects;
        ZFastDictionary m_Dictionary;
        IDictionary* m_StringTable;
        IDictionary_Serializerlib *m_StringTableSave;
    };
}
