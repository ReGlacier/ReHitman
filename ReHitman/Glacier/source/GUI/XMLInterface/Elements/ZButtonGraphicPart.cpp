#include <Glacier/GUI/XMLInterface/Elements/ZButtonGraphicPart.h>
#include <Glacier/GUI/XMLInterface/Elements/ZButtonGraphicPartType.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <cstring>


namespace Glacier
{
    ZButtonGraphicPart::ZButtonGraphicPart()
    {
        m_v2Size.x = 0.0f;
        m_v2Size.y = 0.0f;
        m_iType = 0;
        m_iPriority = 8;
        m_bAnimateAlpha = false;
        m_bFrame = false;
        m_eCheckStatus = eCHECK_HALF;
    }

    void ZButtonGraphicPart::readParams(const char** ppParams, ZMenuElements* pElems)
    {
        ZGUIBase::readParams(ppParams, pElems);

        GuiOption::readBool(&m_bAnimateAlpha, ppParams, "AnimateAlpha", false);
        GuiOption::readInt(m_iPriority, ppParams, "Priority");
        ReadStatus(m_eCheckStatus, ppParams, "Status");
        GuiOption::readBool(&m_bFrame, ppParams, "Frame", false);
        GuiOption::readInt2(m_v2Size.x, ppParams, "W");
        GuiOption::readInt2(m_v2Size.y, ppParams, "H");
    }

    void ZButtonGraphicPart::addElement(const char* pName, ZGUIBase* pEntry)
    {
        if (strcmp(pName, "Type") == 0)
            m_iType |= static_cast<ZButtonGraphicPartType*>(pEntry)->m_iType;
    }

    void ZButtonGraphicPart::ReadStatus(ECheckStatus& rStatus, const char** ppParams, const char* pAttrName)
    {
        rStatus = eCHECK_HALF;

        const char* pAttr = GUI::GetAttr(ppParams, pAttrName, false);
        if (!pAttr)
            return;

        if (strcmp(pAttr, "Unchecked") == 0)
            rStatus = eCHECK_ON;
        else if (strcmp(pAttr, "Checked") == 0)
            rStatus = eCHECK_OFF;
    }
}
