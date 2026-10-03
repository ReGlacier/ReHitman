#include <Glacier/Render/Entry/ZRenderEntryReflectorD3D.h>
#include <Glacier/Render/Entry/ZRenderEntryCameraD3D.h>
#include <Glacier/Render/Cmd/ZRenderCommands.h>
#include <Glacier/Render/Draw/ZDrawBufferSimple.h>
#include <Glacier/Render/Draw/ZRenderDraw.h>
#include <Glacier/Render/Entry/ZRenderEntryLists.h>
#include <Glacier/Render/Prim/SPrimObjectHeader.h>
#include <Glacier/Render/Prim/SPrimObjectHeaderReflection2DData.h>
#include <Glacier/Render/Prim/SPrimHeader.h>
#include <Glacier/Render/Prim/ZPrimHandle.h>
#include <Glacier/Render/View/ZRenderView.h>
#include <Glacier/Render/ZRenderWintelD3D.h>
#include <Glacier/Render/ZViewSpace.h>
#include <Glacier/Geom/ZCAMERA.h>
#include <Glacier/Geom/ZSTDOBJ.h>
#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/Camera/ZCameraSpace.h>
#include <Glacier/ZUniMemory.h>

namespace Glacier
{
    namespace
    {
        void AddCommand(ZCmdList* pCmdList, ZRenderView* pView, ZCmdList::CMD eType)
        {
            auto* pCmd = pCmdList->Current();
            pCmd->m_lType = eType;
            pCmd->m_pRenderEntryGeom = nullptr;
            pCmd->m_pCmdList = pCmdList;
            pCmd->m_pRenderView = pView;
            pCmd->m_lLayer = 0;
            pCmd->m_lNrObjects = 0;
            pCmdList->NextCommand();
        }

        void AddViewport(ZCmdList* pCmdList, ZRenderView* pView)
        {
            const uint32_t* pViewport = pView->Viewport();
            AddCommand(pCmdList, pView, ZCmdList::CMD_END_ALPHABLEND_MAX);
            auto* pCmd = pCmdList->Current() - 1;
            const uint32_t viewport[4] = {
                pViewport[0], pViewport[1],
                pViewport[2] - pViewport[0], pViewport[3] - pViewport[1]
            };
            pCmd->AddData(const_cast<uint32_t*>(viewport), sizeof(viewport));
        }

        uint32_t CollectEntries(ZRenderEntryReflectorD3D* pReflector, ZRenderEntryGeom* pCameraEntry,
            ZViewSpace* pViewSpace, ZRenderEntryLists* pEntryList, ZRenderEntry** apEntries,
            uint32_t lCapacity, float fLODScale, const ZVector3& vObserver)
        {
            auto* pDraw = IDraw::Instance<ZRenderDraw>();
            ZVolumeList volumeList;
            pViewSpace->GetVisibleVolumesIncludingBackdrop(&volumeList, pReflector, true);
            ZRenderEntryLists lists;
            pEntryList->m_pViewList = &lists;
            const uint32_t lCount = pDraw->CreateRenderEntries(apEntries, lCapacity,
                &volumeList, pEntryList, pCameraEntry, const_cast<float*>(&vObserver.x), fLODScale);

            auto* pDrawUpdates = lists.GetList(ZRenderEntryLists::LT_DRAWUPDATE);
            ZDrawBufferSimple drawBuffer(apEntries + lCount, 1, 1);
            ZCameraSpace cameraSpace;
            auto* pCamera = static_cast<ZCAMERA*>(pCameraEntry->GetBaseGeom()->GetGeom());
            cameraSpace = pCamera;
            for (uint32_t i = 0; i < pDrawUpdates->Count(); ++i)
            {
                auto* pEntry = *pDrawUpdates->Get(i);
                pEntry->GetBaseGeom()->GetGeom()->DrawBufferViewUpdate(&drawBuffer, &cameraSpace);
            }
            pEntryList->m_pViewList = nullptr;
            return lCount + drawBuffer.m_lNumRenderEntries;
        }
    }

    ZRenderEntryReflectorD3D::ZRenderEntryReflectorD3D(const ZRenderEntryGeomCreateInfo& sInfo)
        : ZRenderEntryGeomD3D(sInfo),
          m_bReflectionEnabled(false),
          m_bRefractionEnabled(false),
          m_bTwoSided(false),
          m_bCurrentRoomOnly(false),
          m_bCustomFog(false),
          m_PAD_A1 {},
          m_fFogNear(0.0f),
          m_fFogFar(0.0f),
          m_lFogColor(0u)
    {
        m_lGeomListsControl |= 1u << ZRenderEntryLists::LT_ENVIRONMENT;
        const auto* pHeader = ZPrimHandle{ m_lPrimId }.Get<SPrimObjectHeader>();
        if (!pHeader || (pHeader->lPropertyFlags & (SPrimObjectHeader::HAS_REFLECTION2D | SPrimObjectHeader::HAS_REFRACTION2D)) == 0)
            return;

        m_bReflectionEnabled = (pHeader->lPropertyFlags & SPrimObjectHeader::HAS_REFLECTION2D) != 0;
        m_bRefractionEnabled = (pHeader->lPropertyFlags & SPrimObjectHeader::HAS_REFRACTION2D) != 0;
        m_bTwoSided = (pHeader->lPropertyFlags & SPrimObjectHeader::IS_TWOSIDED) != 0;
        const auto* pProperties = ZPrimHandle{ pHeader->lPropertyData }.Get<uint32_t>();
        const auto* pData = pProperties
            ? ZPrimHandle{ pProperties[2] }.Get<SPrimObjectHeaderReflection2DData>()
            : nullptr;
        if (pData)
        {
            m_bCurrentRoomOnly = pData->m_lOnlyCurrentRoom != 0;
            m_lFogColor = pData->m_lFogColor;
            m_fFogNear = pData->m_fFogNear;
            m_fFogFar = pData->m_fFogFar;
            m_bCustomFog = m_fFogFar != 0.0f;
        }
    }

    ZRenderEntryReflectorD3D::~ZRenderEntryReflectorD3D() = default;

    ZRenderEntryReflectorD3D::RENDERENTRY_BASETYPE ZRenderEntryReflectorD3D::GetType() const
    {
        return RT_REFLECTOR;
    }

    void ZRenderEntryReflectorD3D::GetVisible(ZCmdList* pCmdList, ZRenderEntryGeom* pGeomEntry,
        ZViewSpace* pViewSpace, ZRenderView* pView, ZRenderEntryLists* pEntryList)
    {
        if (!pCmdList || !pGeomEntry || !pViewSpace || !pView || !pEntryList || !m_bReflectionEnabled)
            return;

        auto* pBaseGeom = pGeomEntry->GetBaseGeom();
        auto* pCamera = pBaseGeom ? pBaseGeom->GetGeom() : nullptr;
        if (!pCamera || !pCamera->IsDerivedFrom<ZCAMERA>())
            return;

        ZCameraSpace cameraSpace;
        cameraSpace = static_cast<ZCAMERA*>(pCamera);
        pViewSpace->SetClipPlanesFromCameraSpace(&cameraSpace);
        const ZVector3 vObserver = pGeomEntry->m_ObjectToWorldMatrix.p0;
        const float fLODScale = pGeomEntry->GetType() == RT_CAMERA
            ? static_cast<ZRenderEntryCameraD3D*>(pGeomEntry)->m_matProjection._11
            : 0.0f;

        ZRenderEntry* apEntries[2048] {};
        const uint32_t lCount = CollectEntries(this, pGeomEntry, pViewSpace, pEntryList,
            apEntries, 2048, fLODScale, vObserver);

        AddCommand(pCmdList, pView, ZCmdList::CMD_MIRROR_BEGIN);
        CmdDrawEntries(pCmdList, pView, const_cast<const ZRenderEntry**>(apEntries), lCount,
            vObserver, fLODScale, 6, 1, 7, true);
        AddCommand(pCmdList, pView, ZCmdList::CMD_MIRROR_RENDER_REFLECTORS);
        const ZRenderEntry* pReflectorEntry = this;
        CmdDrawEntries(pCmdList, pView, &pReflectorEntry, 1,
            vObserver, fLODScale, 7, 1, 7);
        AddViewport(pCmdList, pView);
        AddCommand(pCmdList, pView, ZCmdList::CMD_MIRROR_RENDER_REFLECTED);
        CmdDrawEntries(pCmdList, pView, const_cast<const ZRenderEntry**>(apEntries), lCount,
            vObserver, fLODScale, 8, 1, 7, true);
        AddCommand(pCmdList, pView, ZCmdList::CMD_MIRROR_END);
    }

    ZRenderEntryReflectorD3D* ZRenderEntryReflectorD3D::Create(const ZRenderEntryGeomCreateInfo& sInfo)
    {
        if (!sInfo.m_pBaseGeom)
            return nullptr;

        const auto* pGeom = sInfo.m_pBaseGeom->GetGeom();
        const bool bStandard = pGeom
            ? pGeom->IsDerivedFrom<ZSTDOBJ>()
            : sInfo.m_pBaseGeom->IsDerivedFromStdObj(ZSTDOBJ::m_Id);
        const bool bLinked = pGeom
            ? pGeom->IsDerivedFrom<ZLNKOBJ>()
            : sInfo.m_pBaseGeom->IsDerivedFromStdObj(ZLNKOBJ::m_Id);
        const auto* pHeader = ZPrimHandle{ sInfo.m_pBaseGeom->Prim() }.Get<SPrimObjectHeader>();
        if (!bStandard || bLinked || !pHeader || (pHeader->lPropertyFlags &
            (SPrimObjectHeader::HAS_REFLECTION2D | SPrimObjectHeader::HAS_REFRACTION2D)) == 0)
            return nullptr;

        return ZUniMemory::New<ZRenderEntryReflectorD3D>(sInfo);
    }
}
