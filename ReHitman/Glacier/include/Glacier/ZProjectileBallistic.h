#pragma once

#include <Glacier/EventBase/ZEventBase.h>
#include <Glacier/CProjectileActivate.h>

namespace Glacier {
    class ZProjectileBallistic : public ZEventBase {
        // vtable
        virtual void SetTarget(ZVector3* pTarget);
        virtual void SetProjectileInfo(CProjectileActivate* pActivationInfo);
        virtual void ShotImpact(COLI* pColi);

        // size (total size is 0xE0, base size is 0x30)
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
        int m_field84;
        int m_field88;
        int m_field8C;
        int m_field90;
        int m_field94;
        int m_field98;
        int m_field9C;
        int m_fieldA0;
        int m_fieldA4;
        int m_fieldA8;
        int m_fieldAC;
        int m_fieldB0;
        int m_fieldB4;
        int m_fieldB8;
        int m_fieldBC;
        int m_fieldC0;
        int m_fieldC4;
        int m_fieldC8;
        int m_fieldCC;
        int m_fieldD0;
        int m_fieldD4;
        int m_fieldD8;
        int m_fieldDC;
    };
}