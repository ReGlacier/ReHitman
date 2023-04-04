#pragma once

#include <Glacier/EventBase/ZEventBase.h>
#include <Glacier/ZSTL/REFTAB.h>

namespace Hitman::BloodMoney
{
    class CMetalDetector : public Glacier::ZEventBase
    {
    public:
        // api
        void DoDetectWeapon();
        void DoAlarm();

        // data (total size is 0xA8, ZEventBase size is 0x30)
        Glacier::REFTAB m_pAttachedActorsRefTab;
        int field_4C;
        Glacier::REFTAB m_reftab1;
        Glacier::REFTAB m_reftab2;
        Glacier::REFTAB m_reftab3;
        char m_bIsAlarming;
        char field_A5;
        char field_A6;
        char field_A7;
    };
}