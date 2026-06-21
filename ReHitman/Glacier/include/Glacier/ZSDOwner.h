#pragma once

#include <Glacier/ReGlacier.h>

namespace Glacier
{
    struct ZSDOwner
    {
        int m_iSoundDefinitionIndex;
        bool m_bEnsureOneChannel;
        char pad[3];
        float m_Time_AllowedToPlay;
        unsigned int m_rLastPlayed;
    };
    RE_VERIFY_SIZE(ZSDOwner, 0x10);
}