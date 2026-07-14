#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Serializer/IDictionary.h>
#include <Glacier/Serializer/ZToken.h>


namespace Glacier
{
    struct ZDictionary;
    struct ZOutputStreamBase;

    struct ZPackedDictionary : public IDictionary
    {
        // vtbl
        virtual ~ZPackedDictionary() override;
        ZToken* GetToken(ZToken* result, const char* word) override;

        // methods
        ZPackedDictionary();
        void Setup();
        void Cleanup();

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
        void Save(ZOutputStreamBase*);
    };
}
