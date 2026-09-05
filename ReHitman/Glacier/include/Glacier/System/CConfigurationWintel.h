#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/System/CConfiguration.h>
#include <cstdint>


namespace Glacier
{
    class CConfigurationWintel : public CConfiguration
    {
    public:
        // vtbl
        void Load() override;
        virtual void Save();
        
        // methods
        CConfigurationWintel();

        // members
        uint32_t m_lNumSoundBuffers { 16u };
        bool m_bUseEAX { false };
    };
    RE_VERIFY_SIZE(CConfigurationWintel, 0xCC);
}