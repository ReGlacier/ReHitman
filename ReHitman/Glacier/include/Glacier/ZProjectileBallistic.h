#pragma once

#include <Glacier/EventBase/ZBaseConRout.h>
#include <Glacier/ZProjectileBase.h>
#include <Glacier/CProjectileActivate.h>
#include <Glacier/ZProjectileBase.h>
#include <Glacier/Fysix/COLI.h>

namespace Glacier {
    class ZProjectileBallistic : public ZProjectileBase {
        // size (total size is 0xE0, base size is 0x30)
        COLI m_LastHitColi;
        float m_fTimer;
        bool m_bDeactivate;
        RE_ADD_PADDING(3);
        float m_fActivationTime;
        float m_fLastHitTime;
        ZVector3 m_vColiNormal;
        void* m_pParticle;
    };
    RE_VERIFY_SIZE(ZProjectileBallistic, 0xE0);
}