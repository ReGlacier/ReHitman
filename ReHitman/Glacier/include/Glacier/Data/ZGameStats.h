#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    class ZGameStats
    {
    public:
        ZGameStats();
        virtual ~ZGameStats();
        virtual uint8_t GetIntensity() const;

        void IncreaseCurrentShotCount();
        void DecreaseCurrentShotCount();
        int16_t GetCurrentShotCount() const { return m_iStats_CurrentShotCount; }

        int16_t m_iStats_CurrentShotCount;
        uint16_t m_pad06;
        float m_iStats_LastShotTime;
    };
    RE_VERIFY_SIZE(ZGameStats, 0xC);
    RE_VERIFY_OFFSET(ZGameStats, m_iStats_CurrentShotCount, 0x4);
    RE_VERIFY_OFFSET(ZGameStats, m_iStats_LastShotTime, 0x8);
}
