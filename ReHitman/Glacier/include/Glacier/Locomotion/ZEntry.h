#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Animation/ZAnimVariationHandle.h>


namespace Glacier
{
    struct ZAnimTemplatesNames;
}

namespace Glacier::Locomotion
{
    struct ZEntry
    {
        ZEntry();

        ZEntry* Init(ZAnimTemplatesNames& names, float fMinSpeed, float fMaxSpeed,
            float fMinDistance, float fMaxDistance, const char* pszAnimation);
        ZEntry* SetFlags(int8_t flags);
        ZEntry* SetCompanionEntryTo(int8_t entry);
        ZEntry* SeekFullMoveCycles(bool seek);
        ZEntry* SetCycleCorrectionSpeed(float speed);
        void SetParameters(float fMinSpeed, float fMaxSpeed, float fDirectionX,
            float fDirectionY, float fMinDistance, float fMaxDistance);

        float m_SpeedThreshold[2];
        float m_Direction[2];
        float m_DistanceThreshold[2];
        float m_CycleCorrectionSpeed;
        ZAnimVariationHandle m_AnimHandle;
        int8_t m_Flags;
        int8_t m_CompanionEntryTo;
        bool m_bSeekFullMoveCycles : 1;
        RE_ADD_PADDING(3);
    };

    RE_VERIFY_SIZE(ZEntry, 0x24);
}
