#pragma once

#include <Glacier/ReGlacier.h>
#include <BloodMoney/Game/UI/ZFRAME.h>


namespace Hitman::BloodMoney
{
    class ZCUSTOMFRAME : public ZFRAME
    {
    public:
        // vtbl (no changes except overrides)
        // data
        int32_t m_aWidths[3];
        int32_t m_aHeights[3];
    };
    RE_VERIFY_SIZE(ZCUSTOMFRAME, 0xD0);
}