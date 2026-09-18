#include <Glacier/GUI/XMLInterface/Elements/ZButtonGraphic.h>
#include <Glacier/GUI/XMLInterface/Elements/ZButtonGraphicPart.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <cstring>


namespace Glacier
{
    ZButtonGraphic::ZButtonGraphic()
    {
        m_iNumOfGraphicElements = 0;
        m_v2TextOffset.x = 0.0f;
        m_v2TextOffset.y = 0.0f;
        m_v2Size.x = 0.0f;
        m_v2Size.y = 0.0f;
        m_eTextAlignment = ELEFT;
        m_eCheckStatus = eCHECK_HALF;
        m_bGraphicsOnly = false;
    }

    void ZButtonGraphic::readParams(const char** ppParams, ZMenuElements* pElems)
    {
        ZGUIBase::readParams(ppParams, pElems);

        GuiOption::readBool(&m_bGraphicsOnly, ppParams, "GraphicsOnly", false);
        ReadElementAlignment(m_eTextAlignment, ppParams, "TextAlignment");
        ReadV2(reinterpret_cast<float(&)[2]>(m_v2TextOffset), ppParams, "TextOffset");
        GuiOption::readInt2(m_v2Size.x, ppParams, "W");
        GuiOption::readInt2(m_v2Size.y, ppParams, "H");
    }

    void ZButtonGraphic::addElement(const char* pName, ZGUIBase* pEntry)
    {
        if (strcmp(pName, "GraphicElement") == 0)
        {
            if (m_iNumOfGraphicElements < 8)
            {
                m_apGraphicPart[m_iNumOfGraphicElements] = static_cast<ZButtonGraphicPart*>(pEntry);
                ++m_iNumOfGraphicElements;
            }
        }
    }

    void ZButtonGraphic::endElement(const char* pTag)
    {
        if (strcmp(pTag, "GraphicElement") == 0)
        {
            ++m_iNumOfGraphicElements;
            return;
        }

        if (strcmp(pTag, "Unchecked") == 0)
        {
            ZASSERT(m_eCheckStatus == eCHECK_ON);
            m_eCheckStatus = eCHECK_HALF;
            return;
        }

        if (strcmp(pTag, "Checked") == 0)
        {
            ZASSERT(m_eCheckStatus == eCHECK_OFF);
            m_eCheckStatus = eCHECK_HALF;
        }
    }

    ZButtonGraphicPart* ZButtonGraphic::GetGraphicPart(int iIndex)
    {
        return m_apGraphicPart[iIndex];
    }

    void ZButtonGraphic::GetTextOffSet(Glacier::Vector2* pOffset)
    {
        pOffset->x = m_v2TextOffset.x;
        pOffset->y = m_v2TextOffset.y;
    }

    void ZButtonGraphic::GetButtonSize(Glacier::Vector2* pSize)
    {
        pSize->x = m_v2Size.x;
        pSize->y = m_v2Size.y;
    }

    int32_t ZButtonGraphic::GetNumOfGraphicElements()
    {
        return m_iNumOfGraphicElements;
    }

    EAlignment ZButtonGraphic::GetTextAlignment()
    {
        return m_eTextAlignment;
    }

    bool ZButtonGraphic::GraphcisOnly() const
    {
        return m_bGraphicsOnly;
    }
}
