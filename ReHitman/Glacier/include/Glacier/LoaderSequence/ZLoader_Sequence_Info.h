#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>

namespace Glacier
{
    // Loader-sequence script data types.
    // Original source: engine\enginedata\loader_sequence_script.cpp (XBOX_KL1 PDB).

    struct SKeyFrame_Info
    {
        float m_fTime; // +0x00
    };
    RE_VERIFY_SIZE(SKeyFrame_Info, 0x4); // verified

    // One picture row at one key frame. Rows for picture P are strided by
    // m_iPicture_Name_Count; a 0xFFFFFFFF bit pattern means "not set in XML".
    struct SPicture_Info
    {
        const char* m_Picture_Name;           // +0x00
        uint32_t    m_iPicture_Nr;            // +0x04
        float       m_fPosX;                  // +0x08
        float       m_fPosY;                  // +0x0C
        float       m_fOpacity;               // +0x10
        float       m_fMultiply;              // +0x14
        uint32_t    m_iPosition_Interpolation; // +0x18 0 = Time, 1 = Progress
    };
    RE_VERIFY_SIZE(SPicture_Info, 0x1C); // verified (PC allocations: 4 * kf, 28 * kf * pics)

    // Raw parsed script data, owned by ZLoader_Sequence_Script and produced by
    // ZLoader_Sequence_Script_Reader::Load_Script. All buffers are ZSysMem blocks.
    struct ZLoader_Sequence_Info
    {
        uint32_t        m_iNr_Key_Frames;               // +0x00
        uint32_t        m_iPicture_Name_Count;          // +0x04
        char*           m_pPicture_Name_Buffer;         // +0x08
        uint32_t*       m_pPicture_Name_Offsets_Buffer; // +0x0C
        SKeyFrame_Info* m_pKey_Frame_Info_Table;        // +0x10
        SPicture_Info*  m_pPicture_Info_Table;          // +0x14
    };
    RE_VERIFY_SIZE(ZLoader_Sequence_Info, 0x18); // XBOX_KL1 PDB; PC ZLoader_Sequence_Script is 0x30 = Info + 6 floats
}
