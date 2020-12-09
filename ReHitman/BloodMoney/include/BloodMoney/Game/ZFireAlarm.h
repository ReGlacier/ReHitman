#pragma once

#include <Glacier/ZEventBase.h>
#include <Glacier/GlacierFWD.h>

namespace Hitman::BloodMoney
{
    class ZFireAlarm : public Glacier::ZEventBase
    {
        /// vftable - no custom vftable
        /// data (total size : 0x48, ZEventBase size: 0x30)
        int m_field30;
        int m_field34;
        int m_field38;
        Glacier::REFTAB* m_listeners;
        Glacier::REFTAB* m_reftab40;
        int m_field44;
    };
}