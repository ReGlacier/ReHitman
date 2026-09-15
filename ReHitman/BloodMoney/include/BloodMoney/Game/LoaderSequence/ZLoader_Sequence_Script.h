#pragma once

#include <Glacier/ReGlacier.h>
#include <BloodMoney/Game/LoaderSequence/ZLoader_Sequence_Info.h>


namespace Hitman::BloodMoney
{
    struct ZLoader_Sequence_Script
    {
        ZLoader_Sequence_Info m_Loader_Sequence_Info;
        float m_fLast_KeyFrame_Time;
        float m_fScreen_Size_X;
        float m_fScreen_Size_Y;
        float m_fFull_Progress_Time;
        float m_fProgress;
        float m_fTime_Adjustment;
    };
}