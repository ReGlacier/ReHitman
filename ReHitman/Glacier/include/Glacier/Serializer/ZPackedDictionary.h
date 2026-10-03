#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Serializer/IDictionary.h>
#include <Glacier/Serializer/ZToken.h>
#include <Glacier/ZSTL/TDynamicArray.h>
#include <Glacier/Serializer/ZDictionary.h>


namespace Glacier
{
    struct ZFastDictionary;
    struct ZOutputStreamBase;
    struct ZInputStreamBase;

    struct ZPackedDictionary : public IDictionary
    {
        // vtbl
        virtual ~ZPackedDictionary() override;
        ZToken GetToken(const char* word) override;

        // methods
        ZPackedDictionary();
        ZPackedDictionary(ZDictionary& sDict);
        ZPackedDictionary(ZFastDictionary& sDict);
        void Setup();
        void Cleanup();

        static uint32_t CalculatePackedSize(const TDynamicArray<ZDictionary::ZTokenizedString>& tokenTable,
                                             int low, int high, int depth);
        uint32_t PackRecursive(const TDynamicArray<ZDictionary::ZTokenizedString>& tokenTable,
                                int& count, int low, int high, int depth);

        // members
        unsigned int m_Size;
        char *m_Letters;
        unsigned int *m_From;
        unsigned int *m_To;
        ZToken *m_Tokens;
    };
    RE_VERIFY_SIZE(ZPackedDictionary, 0x18);

    struct ZPackedDictionary_Serializerlib : public ZPackedDictionary
    {
        // vtbl
        ~ZPackedDictionary_Serializerlib() override;

        // methods
        ZPackedDictionary_Serializerlib();
        ZPackedDictionary_Serializerlib(ZDictionary&);
        ZPackedDictionary_Serializerlib(ZFastDictionary&);
        void Save(ZOutputStreamBase&);
        void Load(ZInputStreamBase&);
    };
    RE_VERIFY_SIZE(ZPackedDictionary_Serializerlib, 0x18);
}
