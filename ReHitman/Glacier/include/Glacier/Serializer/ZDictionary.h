#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Serializer/IDictionary.h>
#include <Glacier/Serializer/ZToken.h>
#include <Glacier/ZSTL/TDynamicArray.h>
#include <Glacier/ZSTL/ZRTStringObject.h>


namespace Glacier
{
    struct ZDictionary : public IDictionary 
    {
        // types
        struct ZTokenizedString 
        {
            // methods
            ZTokenizedString();
            ZTokenizedString(const ZTokenizedString& copy);
            ZTokenizedString& operator=(const ZTokenizedString& copy);

            // members
            ZRTString m_String;
            ZToken m_Token;
        };
        RE_VERIFY_SIZE(ZTokenizedString, 0x8);
        
        // vtbl
        ~ZDictionary() override;
        ZToken* GetToken(ZToken* result, const char* word) override;

        // methods
        ZDictionary();

        // members
        TDynamicArray<ZRTString> m_StringTable;
        TDynamicArray<ZDictionary::ZTokenizedString> m_TokenTable;
    };
}
