#pragma once

#include <Glacier/ReGlacier.h>
#include <BloodMoney/Game/UI/ZTTFONT.h>

namespace Hitman::BloodMoney 
{
    class ZKerningFont : public ZTTFONT 
    {
    public:
        // vtbl (no changes)
        // methods
        static ZKerningFont* Create();

        // data (size is 0x9C, base size is 0x98)
        int32_t m_iExtraKerning;
    };
    RE_VERIFY_SIZE(ZKerningFont, 0x9C); // Verified
}