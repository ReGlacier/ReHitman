#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/CBaseEvent.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/PF4/ZLocation.h>
#include <Glacier/PF4/ZInterface.h>

namespace Glacier {
    class ZGEOM;

    // NOTE: Move to BloodMony, it's game specific code
    class ZActorHeroCheckInside : public Glacier::CBaseEvent<Glacier::ZGEOM>
    {
    public:
        //consts
        static constexpr Glacier::ZMSGID kActivateFrameUpdateMsg = 0x83A;
        //vftable (no changes)
        //api
        bool IsInside(Glacier::ZGEOM* pGeom);
     
        //data (total size is 0x15C, base size is 0x30)
        Glacier::ZREF m_rReceiver;
        float m_fCheckDist;
        PF4::ZLocation m_MapLocation;
        PF4::ZInterface::ZResult m_aResults[32];
        uint32_t m_iNumberOfResults;
        uint32_t m_iCurrentCheckIndex;
        uint32_t m_iActorType;
        uint32_t m_iHitmanType;
    };
    RE_VERIFY_SIZE(ZActorHeroCheckInside, 0x15C); // Verified
}