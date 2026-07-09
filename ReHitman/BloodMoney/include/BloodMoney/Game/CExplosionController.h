#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZEntityLocator.h> // ZBaseGeom
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/CBaseEvent.h>


namespace Hitman::BloodMoney
{
    class CExplosionController : public Glacier::CBaseEvent<Glacier::ZGEOM>
    {
    public:
        // vtbl
        // data
        float m_fMaxDamage;
        float m_fMaxDamageRange;
        float m_fMaxRange;
        Glacier::ZREF m_rEffectGroup;
        Glacier::ZREF m_rExplodedGeom;
        Glacier::REFTAB m_rAfterEffectGeomsToActivate;
        Glacier::REFTAB m_rAfterEffectGeomsToInactivate;
        float m_fHitPoints;
        Glacier::ZMSGID m_msgDirectActivationMessage;
        bool m_bExploded;
        bool m_bExploding;
        uint32_t m_iNumberOfTargets;
        Glacier::ZBaseGeom* m_pTargets[30];
        uint32_t m_iTargetCheck;

    };
    RE_VERIFY_SIZE(CExplosionController, 0x104); // verified
}