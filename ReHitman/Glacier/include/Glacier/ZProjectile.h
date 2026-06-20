#pragma once

#include <Glacier/ZBaseConRout.h>
#include <Glacier/ZProjectileBase.h>
#include <Glacier/Fysix/COLI.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/CProjectileActivate.h>
#include <Glacier/ZProjectileBase.h>


namespace Glacier
{
    class ZProjectile : public ZProjectileBase
    {
    public:
        // size is 0x84 (base size is 0x30)
        uint32_t m_rFlyBySound;
        float m_fMetersPerSecond;
        float m_fTravelDistance;
        float m_fMaxTravelDistance;
        uint32_t m_rLastRefHit;
        float m_fRemFrameTime;
        ZVector3 m_vOrigin;
        int32_t m_nHitCount;
        uint16_t m_msgUnderFire;
        RE_ADD_PADDING(2);
    };
    RE_VERIFY_SIZE(ZProjectile, 0x84);
}