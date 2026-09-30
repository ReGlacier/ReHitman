#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/LoaderSequence/ZLoader_Sequence_Info.h>

namespace Glacier
{
    // Runtime side of the loader sequence script. ZLoader_Sequence_Wintel_D3D
    // allocates one of these (0x30 bytes, via ZLoader_Sequence_Wintel_D3D::LoadScript)
    // and stores it in ZLoader_Sequence_Wintel_D3D::pReader; the values are filled
    // by ZLoader_Sequence_Script_Reader through ZLoader_Sequence_Script::Load_Script.
    // Original source: engine\enginedata\loader_sequence_script.cpp (XBOX_KL1 PDB).
    struct ZLoader_Sequence_Script
    {
        // methods
        ZLoader_Sequence_Script();                    // rev @ 0x4675F0 (PC ctor body)
        ~ZLoader_Sequence_Script();                   // rev from XBOX_KL1 @ 0x8227CD50 (inlined into PC Free_All @ 0x4AEE30)

        void Load_Script(const char* pScript, uint32_t uiScriptSize); // rev @ 0x468020

        void Set_Progress(float fProgress);           // rev @ 0x699100
        uint32_t Get_Nr_Pictures() const              { return m_Loader_Sequence_Info.m_iPicture_Name_Count; } // rev @ 0x6FDBF0 (XBOX_KL1 @ 0x8227CDE0)
        const char* Get_Picture_Name(uint32_t iPicture_Nr) const;  // no standalone PC body, rev from XBOX_KL1 @ 0x8227CE18
        float Get_Screen_Size_X() const               { return m_fScreen_Size_X; } // rev @ 0x467980 (misnamed Get_PosX in the PC database)
        float Get_Screen_Size_Y() const               { return m_fScreen_Size_Y; } // rev @ 0x467990 (misnamed Get_PosY in the PC database)

        // Time-interpolated picture values (PC 0x4680C0 / 0x4676E0 / 0x4677C0 / 0x4678A0).
        // NOTE: these mutate m_fTime_Adjustment, so they are not const.
        float Get_PosX(uint32_t iPicture_Nr, float fTime);
        float Get_PosY(uint32_t iPicture_Nr, float fTime);
        float Get_Opacity(uint32_t iPicture_Nr, float fTime);
        float Get_Multiply(uint32_t iPicture_Nr, float fTime);

        bool Is_Finished(float fTime);                // rev @ 0x4679A0

    private:
        uint32_t Get_End_Key_Frame(float fTime);      // rev @ 0x467660; first key frame strictly after fTime
        float Adjust_Script_Time(float fTime);        // shared prologue of the interpolated getters
        float Interpolate_Picture_Field(uint32_t iPicture_Nr, float fAdjTime, float SPicture_Info::*pField);

    public:
        // data
        ZLoader_Sequence_Info m_Loader_Sequence_Info; // +0x00
        float m_fLast_KeyFrame_Time;                  // +0x18 maximum parsed key-frame time
        float m_fScreen_Size_X;                       // +0x1C default 640.0
        float m_fScreen_Size_Y;                       // +0x20 default 400.0
        float m_fFull_Progress_Time;                  // +0x24 default -1.0, then last key-frame time
        float m_fProgress;                            // +0x28 current progress [0, 1]
        float m_fTime_Adjustment;                     // +0x2C time recorded when progress reaches the end
    };
    RE_VERIFY_SIZE(ZLoader_Sequence_Script, 0x30); // XBOX_KL1 PDB; PC Wintel_D3D::LoadScript allocation
    RE_VERIFY_OFFSET(ZLoader_Sequence_Script, m_fProgress, 0x28);
    RE_VERIFY_OFFSET(ZLoader_Sequence_Script, m_fTime_Adjustment, 0x2C);
}
