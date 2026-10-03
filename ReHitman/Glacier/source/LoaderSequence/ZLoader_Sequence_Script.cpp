#include <Glacier/LoaderSequence/ZLoader_Sequence_Script.h>
#include <Glacier/LoaderSequence/ZLoader_Sequence_Script_Reader.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>

#include <cstring>

namespace Glacier
{
    namespace
    {
        // 'Progress' position-interpolation mode, see XML attribute Position_Interpolation.
        constexpr uint32_t INTERP_PROGRESS = 1;
    }

    ZLoader_Sequence_Script::ZLoader_Sequence_Script()
        // rev @ 0x4675F0
        : m_fLast_KeyFrame_Time(0.0f)
        , m_fScreen_Size_X(640.0f)
        , m_fScreen_Size_Y(400.0f)
        , m_fFull_Progress_Time(-1.0f)
        , m_fProgress(0.0f)
        , m_fTime_Adjustment(0.0f)
    {
        m_Loader_Sequence_Info.m_iNr_Key_Frames = 0;
        m_Loader_Sequence_Info.m_iPicture_Name_Count = 0;
        m_Loader_Sequence_Info.m_pPicture_Name_Buffer = nullptr;
        m_Loader_Sequence_Info.m_pPicture_Name_Offsets_Buffer = nullptr;
        m_Loader_Sequence_Info.m_pKey_Frame_Info_Table = nullptr;
        m_Loader_Sequence_Info.m_pPicture_Info_Table = nullptr;
    }

    ZLoader_Sequence_Script::~ZLoader_Sequence_Script()
    {
        // rev from XBOX_KL1 @ 0x8227CD50; inlined into PC ZLoader_Sequence_Wintel_D3D::Free_All @ 0x4AEE30
        ZUniMemory::Free(m_Loader_Sequence_Info.m_pPicture_Name_Buffer);
        ZUniMemory::Free(m_Loader_Sequence_Info.m_pPicture_Name_Offsets_Buffer);
        ZUniMemory::Free(m_Loader_Sequence_Info.m_pKey_Frame_Info_Table);
        ZUniMemory::Free(m_Loader_Sequence_Info.m_pPicture_Info_Table);
    }

    void ZLoader_Sequence_Script::Load_Script(const char* pScript, uint32_t uiScriptSize)
    {
        // rev @ 0x468020: parse through a stack reader, then copy the
        // script-level settings (the reader owns no persistent state).
        ZLoader_Sequence_Script_Reader reader;
        reader.Load_Script(&m_Loader_Sequence_Info, pScript, uiScriptSize);
        m_fLast_KeyFrame_Time = reader.m_fLast_KeyFrame_Time;
        m_fScreen_Size_X = reader.m_fScreen_Size_X;
        m_fScreen_Size_Y = reader.m_fScreen_Size_Y;
        m_fFull_Progress_Time = reader.m_fFull_Progress_Time;
        reader.Clear();
    }

    void ZLoader_Sequence_Script::Set_Progress(float fProgress)
    {
        // rev @ 0x699100
        m_fProgress = fProgress;
    }

    const char* ZLoader_Sequence_Script::Get_Picture_Name(uint32_t iPicture_Nr) const
    {
        // No standalone body in the PC build; rev from XBOX_KL1 @ 0x8227CE18.
        ZASSERT(iPicture_Nr < m_Loader_Sequence_Info.m_iPicture_Name_Count);
        return &m_Loader_Sequence_Info.m_pPicture_Name_Buffer[m_Loader_Sequence_Info.m_pPicture_Name_Offsets_Buffer[iPicture_Nr]];
    }

    bool ZLoader_Sequence_Script::Is_Finished(float fTime)
    {
        // rev @ 0x4679A0
        if (fTime >= m_fFull_Progress_Time && m_fProgress < 1.0f)
        {
            m_fTime_Adjustment = fTime - m_fFull_Progress_Time;
        }
        return fTime - m_fTime_Adjustment > m_fLast_KeyFrame_Time;
    }

    uint32_t ZLoader_Sequence_Script::Get_End_Key_Frame(float fTime)
    {
        // rev @ 0x467660: index of the first key frame whose time is strictly
        // greater than fTime (clamped into [0, m_iNr_Key_Frames - 1]).
        ZASSERT(m_Loader_Sequence_Info.m_iNr_Key_Frames != 0);
        if (fTime < 0.0f)
        {
            fTime = 0.0f;
        }

        const uint32_t nrKeys = m_Loader_Sequence_Info.m_iNr_Key_Frames;
        const SKeyFrame_Info* pKeys = m_Loader_Sequence_Info.m_pKey_Frame_Info_Table;
        if (pKeys[nrKeys - 1].m_fTime < fTime)
        {
            fTime = pKeys[nrKeys - 1].m_fTime;
        }

        uint32_t i = 0;
        while (i < nrKeys && fTime >= pKeys[i].m_fTime)
        {
            ++i;
        }
        if (i == nrKeys)
        {
            return nrKeys - 1;
        }
        return i;
    }

    // Common prologue: once the current time passes Full_Progress_Time and the
    // progress is not complete yet, remember how far past the end we are so the
    // animation keeps looping from the last key frame.
    float ZLoader_Sequence_Script::Adjust_Script_Time(float fTime)
    {
        if (fTime >= m_fFull_Progress_Time && m_fProgress < 1.0f)
        {
            m_fTime_Adjustment = fTime - m_fFull_Progress_Time;
        }
        return fTime - m_fTime_Adjustment;
    }

    // Linear interpolation of one SPicture_Info field between the surrounding
    // key frames. Common body of PC 0x4680C0 / 0x4676E0 / 0x4677C0 / 0x4678A0.
    float ZLoader_Sequence_Script::Interpolate_Picture_Field(uint32_t iPicture_Nr, float fAdjTime, float SPicture_Info::*pField)
    {
        const uint32_t nrPics = m_Loader_Sequence_Info.m_iPicture_Name_Count;
        const SKeyFrame_Info* pKeys = m_Loader_Sequence_Info.m_pKey_Frame_Info_Table;
        const SPicture_Info* pTable = m_Loader_Sequence_Info.m_pPicture_Info_Table;

        const uint32_t next = Get_End_Key_Frame(fAdjTime);

        float fPrevValue = 0.0f;
        float fPrevTime = 0.0f;
        if (next != 0)
        {
            fPrevValue = pTable[iPicture_Nr + (next - 1) * nrPics].*pField;
            fPrevTime = pKeys[next - 1].m_fTime;
        }

        const float fNextTime = pKeys[next].m_fTime;
        float t = fAdjTime;
        if (t < fPrevTime)
        {
            t = fPrevTime;
        }
        if (fNextTime < t)
        {
            t = fNextTime;
        }
        return ((t - fPrevTime) * (pTable[iPicture_Nr + next * nrPics].*pField) + (fNextTime - t) * fPrevValue)
             / (fNextTime - fPrevTime);
    }

    float ZLoader_Sequence_Script::Get_PosX(uint32_t iPicture_Nr, float fTime)
    {
        // rev @ 0x4680C0
        float fAdjTime = Adjust_Script_Time(fTime);

        // The 'Progress' position-interpolation mode remaps the lookup time from
        // the current progress value instead of from the real time.
        const uint32_t nrPics = m_Loader_Sequence_Info.m_iPicture_Name_Count;
        const SKeyFrame_Info* pKeys = m_Loader_Sequence_Info.m_pKey_Frame_Info_Table;
        uint32_t first = Get_End_Key_Frame(0.0f);
        if (first != 0)
        {
            first -= 1;
        }
        if (m_Loader_Sequence_Info.m_pPicture_Info_Table[iPicture_Nr + first * nrPics].m_iPosition_Interpolation == INTERP_PROGRESS)
        {
            const uint32_t last = Get_End_Key_Frame(1e38f); // PC uses 9.9999997e37
            fAdjTime = pKeys[last].m_fTime * m_fProgress + (1.0f - m_fProgress) * pKeys[0].m_fTime;
        }
        return Interpolate_Picture_Field(iPicture_Nr, fAdjTime, &SPicture_Info::m_fPosX);
    }

    float ZLoader_Sequence_Script::Get_PosY(uint32_t iPicture_Nr, float fTime)
    {
        // rev @ 0x4676E0
        const float fAdjTime = Adjust_Script_Time(fTime);
        return Interpolate_Picture_Field(iPicture_Nr, fAdjTime, &SPicture_Info::m_fPosY);
    }

    float ZLoader_Sequence_Script::Get_Opacity(uint32_t iPicture_Nr, float fTime)
    {
        // rev @ 0x4677C0 (misnamed Get_Picture_Name in the PC database)
        const float fAdjTime = Adjust_Script_Time(fTime);
        return Interpolate_Picture_Field(iPicture_Nr, fAdjTime, &SPicture_Info::m_fOpacity);
    }

    float ZLoader_Sequence_Script::Get_Multiply(uint32_t iPicture_Nr, float fTime)
    {
        // rev @ 0x4678A0 (misnamed Get_Opacity in the PC database)
        const float fAdjTime = Adjust_Script_Time(fTime);
        return Interpolate_Picture_Field(iPicture_Nr, fAdjTime, &SPicture_Info::m_fMultiply);
    }
}
