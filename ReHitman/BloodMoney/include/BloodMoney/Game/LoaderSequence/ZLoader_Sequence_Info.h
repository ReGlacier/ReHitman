#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>

namespace Hitman::BloodMoney
{
    struct SKeyFrame_Info 
    {
        float m_fTime;
    };
    RE_VERIFY_SIZE(SKeyFrame_Info, 0x4); // verified

    struct SPicture_Info 
    {
        const char* m_Picture_Name;
        uint32_t m_iPicture_Nr;
        float m_fPosX;
        float m_fPosY;
        float m_fOpacity;
        float m_fMultiply;
        uint32_t m_iPosition_Interpolation;
    };
    RE_VERIFY_SIZE(SPicture_Info, 0x1C); // verified

    struct ZLoader_Sequence_Info 
    {
        uint32_t m_iNr_Key_Frames;
        uint32_t m_iPicture_Name_Count;
        char* m_pPicture_Name_Buffer;
        uint32_t* m_pPicture_Name_Offsets_Buffer;
        SKeyFrame_Info* m_pKey_Frame_Info_Table;
        SPicture_Info* m_pPicture_Info_Table;
        uint32_t m_field18;
        uint32_t m_field1C;
        uint32_t m_field20;
        uint32_t m_field24;
        uint32_t m_field28;
        float m_field2C;
    };
    RE_VERIFY_SIZE(ZLoader_Sequence_Info, 0x30); // verified
}