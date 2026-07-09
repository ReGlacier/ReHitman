#pragma once

#include <Glacier/ReGlacier.h>
#include <BloodMoney/Game/UI/ZWINOBJ.h>

namespace Hitman::BloodMoney
{
    class ZFRAME : public ZWINOBJ
    {
    public:
        // vtbl
        virtual void SetSize(int, int);
        virtual void SetOuterSize(int, int);
        virtual void CreateGeometry();

        // data
        uint32_t m_aPieces[9];
        uint32_t m_lTextSize;
        int32_t m_lWidth;
        int32_t m_lHeight;
    };
    RE_VERIFY_SIZE(ZFRAME, 0xB8); // Verified
}