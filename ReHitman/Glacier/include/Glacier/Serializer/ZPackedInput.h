#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/Serializer/ZInputStreamBase.h>
#include <Glacier/Serializer/ZPackedDictionary.h>
#include <Glacier/Serializer/ZPackedStream.h>
#include <Glacier/Serializer/ZToken.h>
#include <Glacier/Serializer/ZTokenTable.h>


namespace Glacier
{
    struct ZPackedInput : public IInputSerializerStream
    {
        // vtbl
        ~ZPackedInput() override;
        bool Visit(ISerializerVisitor* pVisitor) override;
        void ExchangeContainer(const ZToken token, unsigned int* count) override;
        void ExchangeRaw(const ZToken token, void* data, const unsigned int size) override;
        ZToken* GetToken(ZToken* result, const char* word) override;
        void BeginArray(const ZToken token, const unsigned int count) override;
        void EndArray() override;
        void ExchangeHeader(const ZToken token, const EPropertyType propertyType) override;
        void ExchangeFooter(const EPropertyType propertyType) override;
        void ExchangeData(ZSerializable* serializable) override;
        void ExchangeData(ZBitfieldBase* data, const ZEnumInfo* description) override;
        void ExchangeData(void* data, const ZEnumInfo* description) override;
        void ExchangeData(zstring& data) override;
        void ExchangeData(const char*& data) override;
        void ExchangeData(double& data) override;
        void ExchangeData(float& data) override;
        void ExchangeData(uint8_t& data) override;
        void ExchangeData(int8_t& data) override;
        void ExchangeData(uint16_t& data) override;
        void ExchangeData(int16_t& data) override;
        void ExchangeData(uint32_t& data) override;
        void ExchangeData(int32_t& data) override;
        void ExchangeData(bool& data) override;

        // methods
        ZPackedInput(ZInputStreamBase* pInput, ZInputStreamBase* pDataStream = nullptr);
        void Init();
        void CheckSignature();
        ZSuccess CheckTag(ZPackedStream::ETag eTag);
        ZSuccess CheckName(ZToken token);

        // members
        ZInputStreamBase* m_Input;
        ZInputStreamBase* m_DataStream;
        ZPackedDictionary_Serializerlib m_Dictionary;
        ZTokenTable_Serializerlib m_StringTable;
        bool m_BigEndian;
        bool m_Finishing;
        RE_ADD_PADDING(2);
    };
    RE_VERIFY_SIZE(ZPackedInput, 0x54);
}
