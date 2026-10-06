#include <Glacier/GUI/XMLInterface/Elements/ZGraphic.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <Glacier/GUI/XMLInterface/ZResourceManager.h>
#include <Glacier/GUI/ZWINGROUP.h>
#include <Glacier/GUI/ZWINPIC.h>
#include <Glacier/Geom/ZGEOM.h>


namespace Glacier
{
    ZGraphic::ZGraphic()
    {
        m_pGraphicGroup = nullptr;
        m_bMirrorY = false;
        m_iPriority = -1;
        m_bUseAsButtons = false;
        m_bAnimateAlpha = false;
    }

    ZGraphic::~ZGraphic()
    {
    }

    void ZGraphic::readParams(const char** ppParams, ZMenuElements* pElems)
    {
        IGUIElement::readParams(ppParams, pElems);

        GuiOption::readBool(&m_bMirrorY, ppParams, "MirrorY", false);
        GuiOption::readInt(m_iPriority, ppParams, "Priority");
        GuiOption::readString(m_sGraphic, "Graphic", ppParams);
        GuiOption::readBool(&m_bUseAsButtons, ppParams, "UseAsButton", false);
        GuiOption::readBool(&m_bAnimateAlpha, ppParams, "AnimateAlpha", false);
    }

    ZGUIElementLink ZGraphic::Setup(float* pfPos, ZResourceManager* pResourceManager, ZWINGROUP* pGroup)
    {
        const ZVector2 vPos { pfPos[0], pfPos[1] };

        const char* pszName = GetName();
        if (!m_sGraphic.empty())
            pszName = m_sGraphic.c_str();

        m_pGraphicGroup = pResourceManager->GetGraphic(vPos, m_pColorSet, pGroup, pszName, m_eAlignment, m_iPriority);
        ZASSERT(m_pGraphicGroup != nullptr);

        float aSize[2];
        GetSize(m_pGraphicGroup, aSize);

        if (m_bUseAsButtons)
        {
            ChangeType(m_pGraphicGroup, 0x29);
            SetAnimateAlpha(m_pGraphicGroup, m_bAnimateAlpha);
        }

        DoMirror();

        return ZGUIElementLink(aSize[0], aSize[1]);
    }

    void ZGraphic::ReleaseResources(ZResourceManager* pResourceManager)
    {
        if (m_pGraphicGroup)
        {
            if (m_bUseAsButtons)
            {
                ChangeType(m_pGraphicGroup, 0x80);
                SetAnimateAlpha(m_pGraphicGroup, false);
            }

            pResourceManager->ReleaseGraphic(m_pGraphicGroup);
            DoMirror();
            m_pGraphicGroup = nullptr;
        }
    }

    void ZGraphic::SetGraphic(const char* pGraphic)
    {
        m_sGraphic = zstring(pGraphic);
    }

    void ZGraphic::DoMirror()
    {
        if (!m_bMirrorY)
            return;

        for (ZBaseGeom* pBaseGeom = m_pGraphicGroup->m_pGroupFirst; pBaseGeom != nullptr; pBaseGeom = pBaseGeom->Next())
        {
            ZGEOM* pGeom = pBaseGeom->GetGeom();
            if (pGeom && pGeom->IsDerivedFrom<ZWINPIC>())
                static_cast<ZWINPIC*>(pGeom)->Mirror(false);
        }
    }
}
