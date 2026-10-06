#include <Glacier/GUI/XMLInterface/Elements/ZColorSet.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <cstdio>


namespace Glacier
{
    ZColorSet::ZColorSet()
    {
        m_aiColors[0] = 0xFFFFFFFFu;
        m_aiColors[1] = 0xFFFFFFFFu;
        m_aiColors[2] = 0xFFFFFFFFu;
    }

    uint32_t ZColorSet::GetColor(EColorIndex colorIndex)
    {
        ZASSERT(colorIndex < 3);
        return m_aiColors[colorIndex];
    }

    void ZColorSet::readParams(const char** ppParams, ZMenuElements* pElems)
    {
        static const char* s_aColorNames[3] = { "NormalColor", "FocusColor", "DisableColor" };

        ZGUIBase::readParams(ppParams, pElems);

        for (int i = 0; i < 3; ++i)
        {
            const char* pAttr = GUI::GetAttr(ppParams, s_aColorNames[i], false);
            if (!pAttr)
                continue;

            int iR = 0;
            int iG = 0;
            int iB = 0;
            int iA = 0;
            if (sscanf(pAttr, "%d,%d,%d,%d", &iR, &iG, &iB, &iA) == 4)
            {
                m_aiColors[i] = (static_cast<uint32_t>(iA) << 24)
                    | (static_cast<uint32_t>(iR) << 16)
                    | (static_cast<uint32_t>(iG) << 8)
                    | static_cast<uint32_t>(iB);
            }
            else
            {
                GUI::ReadUIntHex(m_aiColors[i], ppParams, s_aColorNames[i]);
            }
        }
    }
}
