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
        //vftable
        virtual void LoadSave(Glacier::ZPackedInput* packedInput, bool a2);

        //api
        void CompleteObjective(unsigned int iObjectiveId, bool bShowOSDNotification);
        void PlaySpeech(bool bRegisterSpeechAsSpoken);
        void StopSpeech();

        //data (total size is
        int m_field4;
        int m_field8;
        int m_fieldC;
        int m_field10;
        int m_field14;
        int m_field18;
        int m_field1C;
        int m_field20;
        int m_field24;
        int m_field28;
        int m_field2C;
        int m_field30;
        Glacier::SREF m_rSpeechSoundObjectSREF;
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
        bool m_bIsSpeechPlaying;
        bool m_field65;
        bool m_field66;
        bool m_field67;
        int m_field68;
        int m_field6C;
        char m_bSpoken;
        char m_lObjectiveHiddenFlags;
        char m_lObjectiveCompletedFlags;
        char field_73;
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
    };
}