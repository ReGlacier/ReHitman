#pragma once

#include <Glacier/ZSTL/SimpleXML.h>
#include <BloodMoney/Game/LoaderSequence/ZLoader_Sequence_Info.h>

namespace Hitman::BloodMoney
{
    enum eREAD_MODE {
        eNOTHING = 0,
        eGATHER_INFO = 1,
        eFILL_STRUCTS = 2,
    };

    struct ZLoader_Sequence_Script_Reader : public Glacier::SimpleXML
    {
        float m_fLast_KeyFrame_Time;
        float m_fScreen_Size_X;
        float m_fScreen_Size_Y;
        float m_fFull_Progress_Time;
        eREAD_MODE m_eRead_Mode;
        uint32_t m_iNr_Key_Frames;
        uint32_t m_iPicture_Name_Count;
        char* m_pPicture_Name_Buffer;
        uint32_t m_iPicture_Name_Buffer_Size_Total;
        uint32_t m_iPicture_Name_Buffer_Size_Used;
        uint32_t* m_pPicture_Name_Offsets_Buffer;
        uint32_t m_iPicture_Name_Offsets_Buffer_Size;
        float m_fCurrent_Time;
        uint32_t m_iCurrent_Key_Frame_Nr;
        ZLoader_Sequence_Info* m_pLoader_Sequence_Info;
    }; // 0x1F4 current state, target is 0x1F8
}