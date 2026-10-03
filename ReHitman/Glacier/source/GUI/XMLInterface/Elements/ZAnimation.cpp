#include <Glacier/GUI/XMLInterface/Elements/ZAnimation.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <Glacier/GUI/XMLInterface/ZResourceManager.h>
#include <Glacier/GUI/ZWINGROUP.h>
#include <Glacier/GUI/ZWINOBJ.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniMemory.h>

#include <cstdlib>
#include <cstring>


namespace Glacier
{
    ZAnimation::ZAnimation()
    {
        m_pGraphicGroup = nullptr;
        m_papWinObjs = nullptr;
        m_iNumOfFrames = 0;
        m_iCurrentFrame = 0;
        m_fDeltaTime = 0.0f;
        m_iFramesPerSecond = 15;
    }

    ZAnimation::~ZAnimation()
    {
    }

    void ZAnimation::readParams(const char** ppParams, ZMenuElements* pElems)
    {
        IGUIElement::readParams(ppParams, pElems);

        GuiOption::readInt(m_iFramesPerSecond, ppParams, "FramesPerSecond");
    }

    ZGUIElementLink ZAnimation::Setup(float* pfPos, ZResourceManager* pResourceManager, ZWINGROUP* pGroup)
    {
        const ZVector2 vPos { pfPos[0], pfPos[1] };

        m_pGraphicGroup = pResourceManager->GetGraphic(vPos, m_pColorSet, pGroup, GetName(), m_eAlignment, -1);
        m_iNumOfFrames = m_pGraphicGroup->m_NrAttachGeom;

        m_papWinObjs = static_cast<ZWINOBJ**>(ZUniMemory::Allocate(4 * m_iNumOfFrames));
        memset(m_papWinObjs, 0, 4 * m_iNumOfFrames);

        for (ZBaseGeom* pBaseGeom = m_pGraphicGroup->m_pGroupFirst; pBaseGeom != nullptr; pBaseGeom = pBaseGeom->Next())
        {
            ZGEOM* pGeom = pBaseGeom->GetGeom();
            ZASSERT(pGeom && pGeom->IsDerivedFrom<ZWINOBJ>());

            ZWINOBJ* pWinObj = static_cast<ZWINOBJ*>(pGeom);
            pWinObj->Hide(true);

            const char* pszName = pWinObj->Name();
            if (!pszName)
                pszName = "<NONAME>";

            const char* pszFrame = strchr(pszName, '#');
            if (pszFrame)
            {
                const int32_t iIndex = atoi(pszFrame + 1);
                ZASSERT(iIndex < m_iNumOfFrames);

                if (iIndex < m_iNumOfFrames)
                    m_papWinObjs[iIndex] = pWinObj;
            }
        }

        m_iCurrentFrame = 0;
        m_fDeltaTime = 0.0f;

        if (m_papWinObjs[0])
            m_papWinObjs[0]->Hide(false);

        float aSize[2];
        GetSize(m_pGraphicGroup, aSize);

        return ZGUIElementLink(aSize[0], aSize[1]);
    }

    void ZAnimation::ReleaseResources(ZResourceManager* pResourceManager)
    {
        pResourceManager->ReleaseGraphic(m_pGraphicGroup);
        ZUniMemory::Free(m_papWinObjs);
        m_papWinObjs = nullptr;
    }

    void ZAnimation::Update(bool)
    {
        const int32_t iOldFrame = m_iCurrentFrame;
        const float fFrameTime = 1.0f / static_cast<float>(m_iFramesPerSecond);

        m_fDeltaTime += g_pSysInterface->m_fActualTimeDelta;

        if (m_fDeltaTime > fFrameTime)
        {
            do
            {
                m_fDeltaTime -= fFrameTime;
                ++m_iCurrentFrame;

                if (m_iCurrentFrame >= m_iNumOfFrames)
                    m_iCurrentFrame = 0;
            }
            while (m_fDeltaTime > fFrameTime);
        }

        if (iOldFrame != m_iCurrentFrame)
        {
            if (m_papWinObjs[iOldFrame])
                m_papWinObjs[iOldFrame]->Hide(true);

            if (m_papWinObjs[m_iCurrentFrame])
                m_papWinObjs[m_iCurrentFrame]->Hide(false);
        }
    }
}
