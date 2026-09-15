#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/CBaseEvent.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Geom/ZGEOM.h>


namespace Hitman::BloodMoney
{
    struct ZBits
    {
        bool bPulledIn;
        bool bSofaLeftSideUser;
        bool bSofaRightSideUser;
        bool bSofaCenterSideUser;
    };
    RE_VERIFY_SIZE(ZBits, 4);

    enum EFurnitureType 
    {
        CHAIR = 0,
        MOVEABLECHAIR = 1,
        BARCHAIR = 2,
        SOFA_2PERS = 3,
        SOFA_3PERS = 4,
        BED = 5,
    };

    class ZFurniture : public Glacier::CBaseEvent<Glacier::ZGEOM>
    {
    public:
        // vtbl
        // methods
        void SetPulledIn(bool bPulledIn)
        {
            ZASSERT(m_iType == EFurnitureType::MOVEABLECHAIR);
            m_Bits.bPulledIn = bPulledIn;
        }

        // data
        EFurnitureType m_iType;
        uint8_t m_iMaxNumUser;
        Glacier::ZVector3 m_vResetPos;
        uint8_t m_iNumUsers;
        ZBits m_Bits;
    };
    RE_VERIFY_SIZE(ZFurniture, 0x4C);
}