#pragma once

#include <Glacier/EventBase/ZEventBase.h>
#include <Glacier/Fysix/COLI.h>
#include <Glacier/ZSTL/ZMath.h>

#include <Glacier/CProjectileActivate.h>

namespace Glacier
{
    class ZProjectile : public ZEventBase
    {
    public:
        // vtable
        virtual void SetTarget(ZVector3* pTarget);
        virtual void SetProjectileInfo(CProjectileActivate* pActivationInfo);
        virtual void ShotImpact(COLI* pColi);

        // size is 0x84 (base size is 0x30)
        int m_field30;
        int m_field34;
        int m_field38;
        int m_field3C;
        int m_field40;
        int m_field44;
        int m_field48;
        int m_field4C;
        int m_field50;
        int m_field54;
        int m_field58;
        int m_field5C;
        int m_field60;
        int m_field64;
        int m_field68;
        int m_field6C;
        int m_field70;
        int m_field74;
        int m_field78;
        int m_field7C;
        int m_field80;
    };
}