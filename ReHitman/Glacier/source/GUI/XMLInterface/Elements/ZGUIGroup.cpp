#include <Glacier/GUI/XMLInterface/Elements/ZGUIGroup.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <Glacier/GUI/XMLInterface/ZResourceManager.h>
#include <cstring>


namespace Glacier
{
    ZGUIGroup::ZGUIGroup()
    {
        m_v2Size.x = 0.0f;
        m_v2Size.y = 0.0f;
        m_eNavigation = EVERTICAL;
    }

    ZGUIGroup::~ZGUIGroup()
    {
    }

    void ZGUIGroup::readParams(const char** ppParams, ZMenuElements* pElems)
    {
        IGUIElement::readParams(ppParams, pElems);

        ReadV2(reinterpret_cast<float(&)[2]>(m_v2Size), ppParams, "Size");
        ReadNavigation(m_eNavigation, ppParams);
    }

    ZGUIElementLink ZGUIGroup::Setup(float* pfPos, ZResourceManager* pResourceManager, ZWINGROUP* pGroup)
    {
        for (uint32_t i = 0; i < m_Group.size(); ++i)
        {
            IGUIElement* pElement = m_Group[i];

            float aPos[2] = { pfPos[0], pfPos[1] };

            if (pElement->HasAbsolutePos())
            {
                ZVector2 vPos;
                pElement->GetPos(&vPos);
                aPos[0] += vPos.x;
                aPos[1] += vPos.y;
            }

            pElement->Setup(aPos, pResourceManager, pGroup);
        }

        return ZGUIElementLink();
    }

    void ZGUIGroup::ReleaseResources(ZResourceManager* pResourceManager)
    {
        for (uint32_t i = 0; i < m_Group.size(); ++i)
            m_Group[i]->ReleaseResources(pResourceManager);
    }

    void ZGUIGroup::addElement(const char* pName, ZGUIBase* pEntry)
    {
        if (strcmp(pName, "GuiElement") != 0)
            return;

        if (m_Group.size() < m_Group.capacity())
            m_Group.push_back(static_cast<IGUIElement*>(pEntry));
    }

    void ZGUIGroup::Clear()
    {
        m_Group.clear();
    }

    const ZStaticVector<IGUIElement*, 16>& ZGUIGroup::GetGroup()
    {
        return m_Group;
    }

    ENavigation ZGUIGroup::GetNavigation()
    {
        return m_eNavigation;
    }

    void ZGUIGroup::GetSize(ZVector2& rSize)
    {
        rSize = m_v2Size;
    }
}
