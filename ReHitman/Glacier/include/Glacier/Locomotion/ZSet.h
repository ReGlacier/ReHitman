#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Animation/ZAnimVariationHandle.h>
#include <Glacier/Locomotion/ZEntry.h>
#include <Glacier/Locomotion/ZTransition.h>


namespace Glacier::Locomotion
{
    struct ZSet
    {
        static constexpr int MaxEntries = 10;
        static constexpr int MaxTransitions = 20;

        ZSet();

        ZEntry& GetEntry(int index);
        ZTransition& GetTransition(int index);
        int NewEntry();
        int NewTransition();
        void SetOpenDoorAnim(ZAnimTemplatesNames& names, const char* pszAnimation);
        void SetOpenDoubleDoorAnim(ZAnimTemplatesNames& names, const char* pszAnimation);
        void SetRunOpenDoorAnim(ZAnimTemplatesNames& names, const char* pszAnimation);
        void SetRunOpenDoubleDoorAnim(ZAnimTemplatesNames& names, const char* pszAnimation);

        int m_iEntries;
        int m_iTransitions;
        float m_fTurnSpeed;
        int m_DefaultState;
        float m_AccelerationFactor;
        float m_DecelerationFactor;
        ZEntry m_Entries[MaxEntries];
        ZTransition m_Transitions[MaxTransitions];
        ZAnimVariationHandle m_OpenDoorAnim;
        ZAnimVariationHandle m_OpenDoubleDoorAnim;
        ZAnimVariationHandle m_RunOpenDoorAnim;
        ZAnimVariationHandle m_RunOpenDoubleDoorAnim;
    };

    RE_VERIFY_SIZE(ZSet, 0x458);
}
