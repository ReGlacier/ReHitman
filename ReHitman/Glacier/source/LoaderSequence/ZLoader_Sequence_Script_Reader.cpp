#include <Glacier/LoaderSequence/ZLoader_Sequence_Script_Reader.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZUniAssert.h>

#include <cstdlib>
#include <cstring>

namespace Glacier
{
    namespace
    {
        // XML element and attribute names of the Loader_Sequence script format.
        constexpr const char* ROOT_ELEMENT = "Loader_Sequence_Script";
        constexpr const char* KEY_FRAME_ELEMENT = "Key_Frame";
        constexpr const char* PICTURE_SETTINGS_ELEMENT = "Picture_Settings";
        constexpr const char* SCREEN_ELEMENT = "Screen";

        constexpr const char* ATTR_TIME = "Time";
        constexpr const char* ATTR_PICTURE = "Picture";
        constexpr const char* ATTR_POSITION = "Position";
        constexpr const char* ATTR_OPACITY = "Opacity";
        constexpr const char* ATTR_MULTIPLY = "Multiply";
        constexpr const char* ATTR_POSITION_INTERPOLATION = "Position_Interpolation";
        constexpr const char* ATTR_SCREEN_SIZE = "Screen_Size";
        constexpr const char* ATTR_FULL_PROGRESS_TIME = "Full_Progress_Time";

        // Position_Interpolation attribute values.
        constexpr const char* INTERP_MODE_TIME = "Time";
        constexpr const char* INTERP_MODE_PROGRESS = "Progress";
        constexpr uint32_t INTERP_TIME = 0;
        constexpr uint32_t INTERP_PROGRESS = 1;

        // m_fCurrent_Time sentinel set when a Key_Frame element has no Time attribute.
        constexpr float NO_TIME = -10.0f;

        // Defaults applied to unset picture fields: 0xFFFFFFFF stored by memset.
        constexpr uint32_t UNSET_BITS = 0xFFFFFFFFu;

        bool Is_Unset(float fValue)
        {
            uint32_t bits;
            std::memcpy(&bits, &fValue, sizeof(bits));
            return bits == UNSET_BITS;
        }

        // Clean rewrite of the per-field fill loops of PC 0x466CB0: interpolate the
        // 0xFFFFFFFF "not set" values between the surrounding defined key frames and
        // carry the last defined value forward when nothing is defined below.
        void Interpolate_Undefined_Column(SPicture_Info* pTable, const SKeyFrame_Info* pKeys, uint32_t nrPics, uint32_t nrKeys, uint32_t iPicture_Nr, float SPicture_Info::*pField)
        {
            float fLastValue = pTable[iPicture_Nr].*pField; // row 0 is already default-filled
            float fLastTime = 0.0f;

            for (uint32_t k = 0; k < nrKeys; ++k)
            {
                SPicture_Info* pEntry = &pTable[iPicture_Nr + k * nrPics];
                float& rValue = pEntry->*pField;
                const float fCurTime = pKeys[k].m_fTime;

                if (!Is_Unset(rValue))
                {
                    fLastValue = rValue;
                    fLastTime = fCurTime;
                    continue;
                }

                bool bFoundNext = false;
                float fNextValue = 0.0f;
                float fNextTime = 0.0f;
                for (uint32_t n = k + 1; n < nrKeys; ++n)
                {
                    const float v = pTable[iPicture_Nr + n * nrPics].*pField;
                    if (!Is_Unset(v))
                    {
                        fNextValue = v;
                        fNextTime = pKeys[n].m_fTime;
                        bFoundNext = true;
                        break;
                    }
                }

                if (bFoundNext)
                {
                    const float fSpan = fNextTime - fLastTime;
                    rValue = ((fNextTime - fCurTime) * fLastValue + (fCurTime - fLastTime) * fNextValue) / fSpan;
                    fLastValue = rValue;
                    fLastTime = fCurTime;
                }
                else
                {
                    rValue = fLastValue;
                }
            }
        }
    }

    ZLoader_Sequence_Script_Reader::ZLoader_Sequence_Script_Reader()
        // rev @ 0x4679F0
        : m_fLast_KeyFrame_Time(0.0f)
        , m_fScreen_Size_X(640.0f)
        , m_fScreen_Size_Y(400.0f)
        , m_fFull_Progress_Time(-1.0f)
        , m_eRead_Mode(eNOTHING)
        , m_iNr_Key_Frames(0)
        , m_iPicture_Name_Count(0)
        , m_pPicture_Name_Buffer(nullptr)
        , m_iPicture_Name_Buffer_Size_Total(0)
        , m_iPicture_Name_Buffer_Size_Used(0)
        , m_pPicture_Name_Offsets_Buffer(nullptr)
        , m_iPicture_Name_Offsets_Buffer_Size(0)
        , m_fCurrent_Time(-1.0f)
        , m_iCurrent_Key_Frame_Nr(0)
        , m_pLoader_Sequence_Info(nullptr)
    {
    }

    void ZLoader_Sequence_Script_Reader::Clear()
    {
        // rev @ 0x466C20
        ZUniMemory::Free(m_pPicture_Name_Buffer);
        ZUniMemory::Free(m_pPicture_Name_Offsets_Buffer);

        m_eRead_Mode = eNOTHING;
        m_iNr_Key_Frames = 0;
        m_iPicture_Name_Count = 0;
        m_pPicture_Name_Buffer = nullptr;
        m_iPicture_Name_Buffer_Size_Total = 0;
        m_iPicture_Name_Buffer_Size_Used = 0;
        m_pPicture_Name_Offsets_Buffer = nullptr;
        m_iPicture_Name_Offsets_Buffer_Size = 0;
        m_fCurrent_Time = -1.0f;
        m_iCurrent_Key_Frame_Nr = 0;
        m_pLoader_Sequence_Info = nullptr;
        m_fLast_KeyFrame_Time = 0.0f;
        m_fScreen_Size_X = 640.0f;
        m_fScreen_Size_Y = 400.0f;
        m_fFull_Progress_Time = -1.0f;
    }

    void ZLoader_Sequence_Script_Reader::Load_Script(ZLoader_Sequence_Info* pLoader_Sequence_Info, const char* pScript_Text, uint32_t iScript_Text_Size)
    {
        // rev @ 0x467A80 (cross-checked with XBOX_KL1 @ 0x8227D490):
        // gather pass first, then build the output tables and fill them.
        Clear();
        m_pLoader_Sequence_Info = pLoader_Sequence_Info;

        char* pBuffer = static_cast<char*>(ZUniMemory::Allocate(iScript_Text_Size));
        std::memcpy(pBuffer, pScript_Text, iScript_Text_Size);

        m_eRead_Mode = eGATHER_INFO;
        m_fCurrent_Time = -1.0f;
        XML_Parse(pBuffer, iScript_Text_Size, 1);

        m_pLoader_Sequence_Info->m_iNr_Key_Frames = m_iNr_Key_Frames;
        m_pLoader_Sequence_Info->m_iPicture_Name_Count = m_iPicture_Name_Count;

        m_pLoader_Sequence_Info->m_pKey_Frame_Info_Table = static_cast<SKeyFrame_Info*>(ZUniMemory::Allocate(4 * m_iNr_Key_Frames));
        std::memset(m_pLoader_Sequence_Info->m_pKey_Frame_Info_Table, 0xFF, 4 * m_iNr_Key_Frames);

        m_pLoader_Sequence_Info->m_pPicture_Info_Table = static_cast<SPicture_Info*>(ZUniMemory::Allocate(sizeof(SPicture_Info) * m_iNr_Key_Frames * m_iPicture_Name_Count));
        std::memset(m_pLoader_Sequence_Info->m_pPicture_Info_Table, 0xFF, sizeof(SPicture_Info) * m_iNr_Key_Frames * m_iPicture_Name_Count);

        m_pLoader_Sequence_Info->m_pPicture_Name_Buffer = static_cast<char*>(ZUniMemory::Allocate(m_iPicture_Name_Buffer_Size_Used));
        std::memcpy(m_pLoader_Sequence_Info->m_pPicture_Name_Buffer, m_pPicture_Name_Buffer, m_iPicture_Name_Buffer_Size_Used);

        m_pLoader_Sequence_Info->m_pPicture_Name_Offsets_Buffer = static_cast<uint32_t*>(ZUniMemory::Allocate(4 * m_iPicture_Name_Count));
        std::memcpy(m_pLoader_Sequence_Info->m_pPicture_Name_Offsets_Buffer, m_pPicture_Name_Offsets_Buffer, 4 * m_iPicture_Name_Count);

        m_eRead_Mode = eFILL_STRUCTS;
        m_fCurrent_Time = -1.0f;
        std::memcpy(pBuffer, pScript_Text, iScript_Text_Size);
        XML_Parse(pBuffer, iScript_Text_Size, 1);

        Fill_Keys_Not_Entered();

        ZUniMemory::Free(pBuffer);
    }

    void ZLoader_Sequence_Script_Reader::startElement(const char* name, const char** attrs)
    {
        // rev @ 0x467C70
        if (m_eRead_Mode == eGATHER_INFO)
        {
            if (strcmp(ROOT_ELEMENT, name) == 0)
            {
                return;
            }

            if (strcmp(KEY_FRAME_ELEMENT, name) == 0)
            {
                const char* pTime = GetAttr(attrs, ATTR_TIME);
                if (pTime != nullptr)
                {
                    const float fTime = static_cast<float>(atof(pTime));
                    m_fCurrent_Time = fTime;
                    if (m_iNr_Key_Frames != 0 || fTime == 0.0f)
                    {
                        ++m_iNr_Key_Frames;
                    }
                    else
                    {
                        // An implicit key frame at time 0 is inserted before the first
                        // non-zero key frame (see the FILL pass below).
                        m_iNr_Key_Frames = 2;
                    }
                    return;
                }

                m_fCurrent_Time = NO_TIME;
                return;
            }

            if (strcmp(PICTURE_SETTINGS_ELEMENT, name) == 0 && m_fCurrent_Time != NO_TIME)
            {
                const char* pPicture = GetAttr(attrs, ATTR_PICTURE);
                if (pPicture != nullptr)
                {
                    Add_Picture_Name(pPicture);
                }
            }
            return;
        }

        if (m_eRead_Mode != eFILL_STRUCTS || strcmp(ROOT_ELEMENT, name) == 0)
        {
            return;
        }

        if (strcmp(KEY_FRAME_ELEMENT, name) == 0)
        {
            const char* pTime = GetAttr(attrs, ATTR_TIME);
            if (pTime == nullptr)
            {
                m_fCurrent_Time = NO_TIME;
                return;
            }

            const float fTime = static_cast<float>(atof(pTime));
            m_fCurrent_Time = fTime;
            if (fTime > m_fLast_KeyFrame_Time)
            {
                m_fLast_KeyFrame_Time = fTime;
            }

            if (m_iCurrent_Key_Frame_Nr == 0 && fTime != 0.0f)
            {
                ZASSERT(m_iNr_Key_Frames != 0);
                m_pLoader_Sequence_Info->m_pKey_Frame_Info_Table[m_iCurrent_Key_Frame_Nr++].m_fTime = 0.0f;
            }

            ZASSERT(m_iCurrent_Key_Frame_Nr < m_iNr_Key_Frames);
            m_pLoader_Sequence_Info->m_pKey_Frame_Info_Table[m_iCurrent_Key_Frame_Nr++].m_fTime = m_fCurrent_Time;
            return;
        }

        if (strcmp(PICTURE_SETTINGS_ELEMENT, name) == 0)
        {
            if (m_fCurrent_Time == NO_TIME)
            {
                return;
            }

            ZASSERT(m_iCurrent_Key_Frame_Nr != 0);
            const char* pPicture = GetAttr(attrs, ATTR_PICTURE);
            if (pPicture == nullptr)
            {
                return;
            }

            const uint32_t pictureNr = Get_Picture_Nr(pPicture);
            // The row belongs to the key frame element parsed just above.
            SPicture_Info* pEntry = &m_pLoader_Sequence_Info->m_pPicture_Info_Table[pictureNr + m_iPicture_Name_Count * (m_iCurrent_Key_Frame_Nr - 1)];
            pEntry->m_iPicture_Nr = pictureNr;

            const char* pPosition = GetAttr(attrs, ATTR_POSITION);
            if (pPosition != nullptr)
            {
                Get_Cds(pEntry->m_fPosX, pEntry->m_fPosY, pPosition);
            }

            const char* pOpacity = GetAttr(attrs, ATTR_OPACITY);
            if (pOpacity != nullptr)
            {
                pEntry->m_fOpacity = static_cast<float>(atof(pOpacity));
            }

            const char* pMultiply = GetAttr(attrs, ATTR_MULTIPLY);
            if (pMultiply != nullptr)
            {
                pEntry->m_fMultiply = static_cast<float>(atof(pMultiply));
            }

            const char* pInterp = GetAttr(attrs, ATTR_POSITION_INTERPOLATION);
            if (pInterp != nullptr)
            {
                if (strcmp(pInterp, INTERP_MODE_TIME) == 0)
                {
                    pEntry->m_iPosition_Interpolation = INTERP_TIME;
                }
                else if (strcmp(pInterp, INTERP_MODE_PROGRESS) == 0)
                {
                    pEntry->m_iPosition_Interpolation = INTERP_PROGRESS;
                }
            }
            return;
        }

        if (strcmp(SCREEN_ELEMENT, name) == 0)
        {
            const char* pScreenSize = GetAttr(attrs, ATTR_SCREEN_SIZE);
            if (pScreenSize != nullptr)
            {
                Get_Cds(m_fScreen_Size_X, m_fScreen_Size_Y, pScreenSize);
            }

            const char* pFullProgressTime = GetAttr(attrs, ATTR_FULL_PROGRESS_TIME);
            if (pFullProgressTime != nullptr)
            {
                m_fFull_Progress_Time = static_cast<float>(atof(pFullProgressTime));
            }
        }
    }

    void ZLoader_Sequence_Script_Reader::endElement(const char* /*name*/)
    {
        // Empty body (PC vtable slot 1 -> nullsub_4).
    }

    void ZLoader_Sequence_Script_Reader::Add_Picture_Name(const char* psName)
    {
        // rev @ 0x4672F0
        for (uint32_t i = 0; i < m_iPicture_Name_Count; ++i)
        {
            if (stricmp(&m_pPicture_Name_Buffer[m_pPicture_Name_Offsets_Buffer[i]], psName) == 0)
            {
                return; // already registered
            }
        }

        // Grow the offsets buffer (first block 128 entries, then doubling).
        if (m_iPicture_Name_Offsets_Buffer_Size == m_iPicture_Name_Count)
        {
            if (m_iPicture_Name_Offsets_Buffer_Size != 0)
            {
                const uint32_t newSize = 2 * m_iPicture_Name_Offsets_Buffer_Size;
                uint32_t* pNewBuffer = static_cast<uint32_t*>(ZUniMemory::Allocate(8 * m_iPicture_Name_Offsets_Buffer_Size));
                std::memcpy(pNewBuffer, m_pPicture_Name_Offsets_Buffer, 4 * m_iPicture_Name_Offsets_Buffer_Size);
                ZUniMemory::Free(m_pPicture_Name_Offsets_Buffer);
                m_pPicture_Name_Offsets_Buffer = pNewBuffer;
                m_iPicture_Name_Offsets_Buffer_Size = newSize;
            }
            else
            {
                ZASSERT(m_pPicture_Name_Buffer == nullptr);
                m_iPicture_Name_Offsets_Buffer_Size = 128;
                m_pPicture_Name_Offsets_Buffer = static_cast<uint32_t*>(ZUniMemory::Allocate(512));
            }
        }

        m_pPicture_Name_Offsets_Buffer[m_iPicture_Name_Count++] = m_iPicture_Name_Buffer_Size_Used;

        // Grow the name buffer (first block 1024 bytes, then doubling).
        const uint32_t iNameSize = static_cast<uint32_t>(strlen(psName)) + 1;
        if (m_iPicture_Name_Buffer_Size_Total < m_iPicture_Name_Buffer_Size_Used + iNameSize)
        {
            if (m_iPicture_Name_Buffer_Size_Total != 0)
            {
                const uint32_t newTotal = 2 * m_iPicture_Name_Buffer_Size_Total;
                char* pNewBuffer = static_cast<char*>(ZUniMemory::Allocate(newTotal));
                std::memcpy(pNewBuffer, m_pPicture_Name_Buffer, m_iPicture_Name_Buffer_Size_Used);
                ZUniMemory::Free(m_pPicture_Name_Buffer);
                m_pPicture_Name_Buffer = pNewBuffer;
                m_iPicture_Name_Buffer_Size_Total = newTotal;
            }
            else
            {
                ZASSERT(m_pPicture_Name_Buffer == nullptr);
                m_iPicture_Name_Buffer_Size_Total = 1024;
                m_pPicture_Name_Buffer = static_cast<char*>(ZUniMemory::Allocate(1024));
            }
        }

        strcpy(&m_pPicture_Name_Buffer[m_iPicture_Name_Buffer_Size_Used], psName);
        m_iPicture_Name_Buffer_Size_Used += iNameSize;
    }

    uint32_t ZLoader_Sequence_Script_Reader::Get_Picture_Nr(const char* psName)
    {
        // rev @ 0x467500
        if (m_iPicture_Name_Count != 0)
        {
            for (uint32_t i = 0; i < m_iPicture_Name_Count; ++i)
            {
                if (stricmp(&m_pPicture_Name_Buffer[m_pPicture_Name_Offsets_Buffer[i]], psName) == 0)
                {
                    return i;
                }
            }
        }

        ZASSERT(false); // the name was never registered through Add_Picture_Name
        return 0;
    }

    void ZLoader_Sequence_Script_Reader::Fill_Keys_Not_Entered()
    {
        // rev @ 0x466CB0 (clean rewrite; the PC body unrolls every field loop)
        if (m_pLoader_Sequence_Info->m_iNr_Key_Frames == 0 || m_pLoader_Sequence_Info->m_iPicture_Name_Count == 0)
        {
            return;
        }

        if (m_fFull_Progress_Time < 0.0f)
        {
            m_fFull_Progress_Time = m_fLast_KeyFrame_Time;
        }

        const uint32_t nrPics = m_pLoader_Sequence_Info->m_iPicture_Name_Count;
        const uint32_t nrKeys = m_pLoader_Sequence_Info->m_iNr_Key_Frames;
        SPicture_Info* pTable = m_pLoader_Sequence_Info->m_pPicture_Info_Table;
        const SKeyFrame_Info* pKeys = m_pLoader_Sequence_Info->m_pKey_Frame_Info_Table;

        for (uint32_t p = 0; p < nrPics; ++p)
        {
            SPicture_Info& rFirst = pTable[p];
            if (Is_Unset(rFirst.m_fPosX))     { rFirst.m_fPosX = 0.0f; }
            if (Is_Unset(rFirst.m_fPosY))     { rFirst.m_fPosY = 0.0f; }
            if (Is_Unset(rFirst.m_fOpacity))  { rFirst.m_fOpacity = 1.0f; }
            if (Is_Unset(rFirst.m_fMultiply)) { rFirst.m_fMultiply = 1.0f; }
            if (rFirst.m_iPosition_Interpolation == UNSET_BITS) { rFirst.m_iPosition_Interpolation = 0; }

            // Propagate the interpolation mode down the column.
            uint32_t interp = rFirst.m_iPosition_Interpolation;
            for (uint32_t k = 0; k < nrKeys; ++k)
            {
                SPicture_Info* pEntry = &pTable[p + k * nrPics];
                if (pEntry->m_iPosition_Interpolation == UNSET_BITS)
                {
                    pEntry->m_iPosition_Interpolation = interp;
                }
                else
                {
                    interp = pEntry->m_iPosition_Interpolation;
                }
            }

            Interpolate_Undefined_Column(pTable, pKeys, nrPics, nrKeys, p, &SPicture_Info::m_fPosX);
            Interpolate_Undefined_Column(pTable, pKeys, nrPics, nrKeys, p, &SPicture_Info::m_fPosY);
            Interpolate_Undefined_Column(pTable, pKeys, nrPics, nrKeys, p, &SPicture_Info::m_fOpacity);
            Interpolate_Undefined_Column(pTable, pKeys, nrPics, nrKeys, p, &SPicture_Info::m_fMultiply);
        }
    }

    void ZLoader_Sequence_Script_Reader::Get_Cds(float& fX, float& fY, const char* psValue)
    {
        // rev @ 0x467560: split "X,Y" into two floats. A missing comma keeps both
        // halves equal to the full string (as in the PC body).
        char szBuffer[256];
        strncpy(szBuffer, psValue, sizeof(szBuffer) - 1);
        szBuffer[sizeof(szBuffer) - 1] = '\0';

        char* pSecond = szBuffer;
        char* pComma = strchr(szBuffer, ',');
        if (pComma != nullptr)
        {
            *pComma = '\0';
            pSecond = pComma + 1;
        }

        fX = static_cast<float>(atof(szBuffer));
        fY = static_cast<float>(atof(pSecond));
    }
}
