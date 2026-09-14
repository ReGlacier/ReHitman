#pragma once

#include <Glacier/STempStripsUniqueId.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ReGlacier.h>

namespace Glacier
{
    struct COLI
    {
        COLI()
            : t(1.0f)
            , ColiRef(0)
            , m_bBothSides(true)
        {
        }

        ZVector3 lp;
        ZVector3 ln;
        ZVector3 cp;
        float t;
        uint32_t m_BoneId;
        RE_ADD_PADDING(4); // KL1: Confirmed padding by 4 bytes due COLI aligned by 8?
        STempStripsUniqueId m_HitCache;
        uint32_t m_iColiMaterialDescId;
        uint32_t ColiRef;
        ZVector3 vP1;
        ZVector3 vP2;
        ZVector3 vP3;
        bool m_bBothSides;
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(COLI, 0x68);
}
