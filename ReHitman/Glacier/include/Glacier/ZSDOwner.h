#pragma once

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

    static_assert(sizeof(ZSDOwner) == 16, "Bad size of ZSDOwner");
}