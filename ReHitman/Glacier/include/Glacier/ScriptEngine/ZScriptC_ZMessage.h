#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    struct ZScriptC_Indexes
    {
        uint16_t m_lNextIndex;
        uint16_t m_lUniqueText;
        uint16_t m_lZMsgID;
    };
    RE_VERIFY_SIZE(ZScriptC_Indexes, 0x6);

    struct ZScriptC_ZMessage 
    {
        const char* m_pKeys;
        ZScriptC_Indexes* m_pIndexes;
    };
    RE_VERIFY_SIZE(ZScriptC_ZMessage, 0x8);
}