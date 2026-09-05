#pragma once

#include <Glacier/GlacierFWD.h>

namespace Hitman::BloodMoney
{
    struct SBriefObjective
    {
        int m_field0;
        int m_field4;
        int m_field8;
        int m_fieldC;
        int m_field10;
        int m_field14;
    }; //size 0x18

    class ZHM3BriefingControl
    {
    public:
        virtual ~ZHM3BriefingControl();

    public: //api
        void CompleteObjective(unsigned int iObjectiveId, bool bShowOSDNotification);
        void PlaySpeech(bool bRegisterSpeechAsSpoken);
        void StopSpeech();

    public: // members
        uint8_t m_aszChangedObjectiveTexts[24]; //0x0004
        uint8_t m_aszChangedObjectivePaths[24]; //0x001C
        int32_t m_aiSoundRefs[12]; //0x0034
        bool m_iSpeaking; //0x0064
        int8_t m_iCurrentSpeechIdx; //0x0065
        int8_t m_iCurrentHint; //0x0066
        int8_t m_PAD67; //0x0067
        float m_fCurrentLength; //0x0068
        float m_fStartTime; //0x006C
        bool m_bSpoken; //0x0070
        int8_t m_lObjectiveHiddenFlags; //0x0071
        int8_t m_lObjectiveCompletedFlags; //0x0072
        uint8_t m_PAD73; //0x0073
    };

    static_assert(sizeof(ZHM3BriefingControl) == 116, "Bad size of ZHM3BriefingControl");
}