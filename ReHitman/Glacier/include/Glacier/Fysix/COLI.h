#pragma once

#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ReGlacier.h>

namespace Glacier
{
    struct STempStripsUniqueId
    {
        uint32_t id1;
        uint32_t id2;
    };
    RE_VERIFY_SIZE(STempStripsUniqueId, 0x8);

    struct COLI
    {
        ZVector3 lp;
        ZVector3 ln;
        ZVector3 cp;
        float t;
        uint32_t m_BoneId;
        uint32_t m_Unknown2C; // Really idk, this is padding in PS2
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
    RE_VERIFY_OFFSET(COLI, m_Unknown2C, 0x2C);
}