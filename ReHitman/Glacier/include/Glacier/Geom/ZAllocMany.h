#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZGROUP.h>

namespace Glacier
{
    struct ZAllocMany : public ZGROUP
    {
        int16_t m_iNumObjects;
        int16_t m_iNumFree;
        int16_t m_iFreeIndex;
        bool m_bNeedClean;
        RE_ADD_PADDING(2);
        struct ZAllocMany *m_pOriginal;
        uint16_t m_msgActivate;
        bool m_bClone;
        RE_ADD_PADDING(1);
        uint32_t m_rCloneGroup;
        bool m_bInitClones;
        RE_ADD_PADDING(3);
        struct ZAllocMany **m_pFreeObjects;
    }; // size 0x68
    RE_VERIFY_SIZE(ZAllocMany, 0x68); // Verified
}