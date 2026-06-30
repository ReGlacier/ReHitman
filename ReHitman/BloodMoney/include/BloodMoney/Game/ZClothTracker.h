#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZArray.h>
#include <Glacier/CBaseEvent.h>
#include <Glacier/Geom/ZGEOM.h>

namespace Hitman::BloodMoney 
{
    struct SClothInfo 
    {
        Glacier::ZREF rHitmanAs;
        float fDisguiseQuality;
        bool bDisguiseBlown;
        bool m_bPad[3];
    };

    struct SInspector 
    {
        Glacier::ZREF rSeerActor;
        float fQuality;
        struct ZHM3ActorProperties* rt_pActorProperty;
        unsigned char iClothIdx;

        union {
            unsigned char mask;
            struct {
                unsigned char bInvestigating : 1;
                unsigned char bKnownCloth : 1;
                unsigned char bNotorietyBlowCover : 1;
            };
        };
        RE_ADD_PADDING(2);
    };

    struct ZClothTracker : public Glacier::CBaseEvent<Glacier::ZGEOM>
    {
        Glacier::ZArray<SClothInfo> m_Clothes;
        Glacier::ZArray<SInspector> m_Inspectors;

        float m_fInspectTime;
        float m_fDiffSpeedMult;
        float m_fDiffForgetMult;
        float m_fDiffSeeThroughDisguiseDistance;

        int m_iNotoriety;
        float m_fNotorietySpeedMult;
        float m_fNotorietyDistMult;
        int m_iCurrInspectCloth;
    };
    RE_VERIFY_SIZE(ZClothTracker, 0x88);
}