#include <Glacier/Render/Draw/ZRenderDrawD3D.h>
#include <Glacier/Render/View/ZRenderViewD3D.h>
#include <Glacier/Render/Entry/ZRenderEntryBones.h>
#include <Glacier/Render/Entry/ZRenderEntryLists.h>
#include <Glacier/Render/Cmd/ZCmdList.h>
#include <Glacier/Render/ZDirect3DDevice.h>
#include <Glacier/Render/ZRenderContext.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/ZRenderX86.h>
#include <Glacier/Render/ZRenderWintelD3D.h>
#include <Glacier/Render/ZViewSpace.h>
#include <Glacier/Render/ZSharedResourcesD3D.h>
#include <Glacier/Render/ZWaterManager.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/System/ZSysMem.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/Geom/ZCAMERA.h>
#include <Glacier/IK/ZBoneModifyBase.h>
#include <Glacier/Render/Material/ZRenderMaterialInstance.h>
#include <Glacier/Render/Material/ZRenderMaterialSubClass.h>
#include <Glacier/Render/Object/ZRenderObject.h>
#include <Glacier/Render/Object/ZRenderObjectInstance.h>
#include <Glacier/Render/Prim/SPrimLight.h>
#include <Glacier/Render/Prim/ZPrimHandle.h>
#include <Glacier/Render/PostFilter/ZPostFilter.h>
#include <Glacier/Render/View/ZRenderView.h>
#include <Glacier/ZUniMemory.h>
#include <cstring>


namespace Glacier
{
    namespace
    {
        struct ZCommandArena
        {
            ZCmdList m_List{};
            char* m_pStorage{ nullptr };

            void Init()
            {
                if (!m_pStorage)
                {
                    m_pStorage = static_cast<char*>(ZUniMemory::Allocate(0x40000));
                    m_List.m_pBufferEnd = m_pStorage + 0x40000;
                }
                m_List.m_lPreTransactionNrCmds = 0;
                m_List.m_pPreTransactionCurrent = m_pStorage;
                m_List.m_lNrCmds = 0;
                m_List.m_pCurrent = m_pStorage;
                m_List.m_Buffer = m_pStorage;
            }
        };

        ZCommandArena s_CommandArena;

        struct SurfaceState
        {
            IDirect3DSurface9* m_pRenderTarget;
            IDirect3DSurface9* m_pDepthStencil;
            D3DVIEWPORT9 m_Viewport;
        };

        void DrawCommandObjects(ZRenderObjectInstance** ppObjects, uint32_t lCount,
                                uint32_t lLayer, ZRenderContext* pContext)
        {
            uint32_t first = 0;
            while (first < lCount)
            {
                auto* pSubClass = ppObjects[first]->m_pRenderObject->m_pMaterialInstance->m_pMaterialSubClass;
                uint32_t end = first + 1;
                while (end < lCount && ppObjects[end]->m_pRenderObject->m_pMaterialInstance->m_pMaterialSubClass == pSubClass)
                    ++end;
                const uint32_t lPasses = pSubClass->Begin(lLayer, pContext);
                for (uint32_t lPass = 0; lPass < lPasses; ++lPass)
                {
                    pSubClass->BeginPass(lPass, pContext);
                    uint32_t materialFirst = first;
                    while (materialFirst < end)
                    {
                        auto* pMaterial = ppObjects[materialFirst]->m_pRenderObject->m_pMaterialInstance;
                        uint32_t materialEnd = materialFirst + 1;
                        while (materialEnd < end && ppObjects[materialEnd]->m_pRenderObject->m_pMaterialInstance == pMaterial)
                            ++materialEnd;
                        pMaterial->Begin(pContext);
                        pMaterial->Draw(ppObjects + materialFirst, materialEnd - materialFirst, pContext);
                        pMaterial->End();
                        materialFirst = materialEnd;
                    }
                    pSubClass->EndPass();
                }
                pSubClass->End();
                first = end;
            }
        }
    }

    ZRenderDrawD3D::ZRenderDrawD3D()
        : ZRenderDraw()
    {
        // Do nothing
    }

    void ZRenderDrawD3D::InitAllocation()
    {
        memset(m_apRenderEntryLookup, 0, sizeof(m_apRenderEntryLookup));
    }

    void ZRenderDrawD3D::BeginFrame()
    {
        m_DecalMarks.BeginFrame();
        ZRenderDraw::BeginFrame();
    }

    void ZRenderDrawD3D::Update(ZRender* pRender)
    {
        m_pEntryReuse = nullptr;

        ZRenderEntryMap entryReuse;
        for (uint32_t i = 0; i < m_lRenderEntriesCount; ++i)
        {
            ZRenderEntry* pEntry = m_apRenderEntries[i];
            if (pEntry && (pEntry->m_lControl & ZRenderEntry::RE_ADDTOREUSE) != 0)
                entryReuse.Add(pEntry->GetPrim(), pEntry);
        }
        m_pEntryReuse = &entryReuse;

        ZRenderEntryLists entryLists;
        ZViewSpace viewSpace;
        s_CommandArena.Init();

        for (uint32_t i = 0; i < m_Views.Count(); ++i)
        {
            ZRenderViewBase* pViewBase = *m_Views.Get(i);
            if (!pViewBase || pViewBase->m_pRender != pRender)
                continue;
            auto* pView = static_cast<ZRenderView*>(pViewBase);
            for (uint32_t cameraIndex = 0; cameraIndex < pView->m_Cameras.Count(); ++cameraIndex)
            {
                ZCAMERA* pCamera = *pView->m_Cameras.Get(cameraIndex);
                ZBaseGeom* pCameraGeom = pCamera ? pCamera->BaseGeom() : nullptr;
                if (!pCamera || !pCamera->IsActive() || !pCameraGeom || !pCameraGeom->m_lDrawId)
                    continue;
                ZRenderEntry* pEntry = m_apRenderEntryLookup[pCameraGeom->m_lDrawId];
                if (pEntry)
                {
                    pEntry->GetVisible(&s_CommandArena.m_List, nullptr, &viewSpace, pView, &entryLists);
                    viewSpace.Reset();
                }
            }
        }

        if (g_pWaterManager && g_pSysInterface)
            g_pWaterManager->FrameUpdate(
                static_cast<float>(g_pSysInterface->FrameTime.secs) * (1.0f / 1024.0f));

        for (uint32_t i = 0; i < m_lRenderEntriesCount; ++i)
        {
            if (m_apRenderEntries[i])
                m_apRenderEntries[i]->Update();
        }

        m_pEntryReuse = nullptr;
        UpdateLightList(&entryLists);

        for (uint32_t i = 0; i < m_lRenderEntriesCount; ++i)
        {
            auto* pEntry = m_apRenderEntries[i];
            auto* pGeomEntry = pEntry && pEntry->GetType() == ZRenderEntry::RT_GEOM
                ? static_cast<ZRenderEntryGeom*>(pEntry) : nullptr;
            if (pGeomEntry && pGeomEntry->GetBaseGeom()
                && (pGeomEntry->GetBaseGeom()->m_lControl & ZCUPDATELIGHT) != 0)
                pGeomEntry->m_lControl |= ZRenderEntry::RE_NEEDUPDATE;
        }

        auto* pDrawUpdate = entryLists.GetList(ZRenderEntryLists::LT_DRAWUPDATE);
        for (int i = 0; i < pDrawUpdate->Count(); ++i)
        {
            ZRenderEntryGeom* pEntry = *pDrawUpdate->Get(i);
            if (pEntry && pEntry->GetBaseGeom())
                pEntry->GetBaseGeom()->FixLightList();
        }
        UpdateBoneModifiers(&entryLists);

        auto* pContext = static_cast<ZRenderX86*>(pRender)->m_pContext;
        if (pContext)
            pContext->m_pRender = pRender;
        D3DVIEWPORT9 savedViewport{};
        SurfaceState surfaceStack[8]{};
        uint32_t surfaceDepth = 0;
        ZMat4x4 projectionStack[8]{};
        uint32_t projectionDepth = 0;
        ZMatrix currentViewMatrix{};

        for (char* pCurrent = s_CommandArena.m_List.m_Buffer;
             pCurrent < s_CommandArena.m_List.m_pCurrent; )
        {
            auto* pCommand = reinterpret_cast<ZCmdList::ZCmd*>(pCurrent);
            const auto* pData = reinterpret_cast<const uint32_t*>(pCurrent + sizeof(ZCmdList::ZCmd));
            const uint32_t* pNext = pData + pCommand->m_lNrObjects;

            switch (pCommand->m_lType)
            {
            case ZCmdList::CMD_SET_VIEW:
                if (pContext && pCommand->m_lNrObjects >= 36)
                {
                    memcpy(&currentViewMatrix, pData, sizeof(currentViewMatrix));
                    memcpy(&pContext->m_ProjectionMatrix, pData + 12,
                           sizeof(pContext->m_ProjectionMatrix));
                    pContext->m_WorldToViewMatrix = currentViewMatrix;
                    pContext->m_pRenderStats = reinterpret_cast<SRenderStats*>(pCommand->m_pRenderView);
                    memcpy(&pContext->m_vFogNearPlane[2], pData + 32, 0x10);
                }
                if (pCommand->m_lNrObjects >= 31)
                    pRender->SetFog(*reinterpret_cast<const float*>(pData + 28),
                                    *reinterpret_cast<const float*>(pData + 29), pData[30]);
                if (pCommand->m_lNrObjects >= 32)
                {
                    auto* pRenderX86 = static_cast<ZRenderX86*>(pRender);
                    pRenderX86->m_lDrawConForbidCamera = pData[31];
                    pRenderX86->m_lDrawMask = 3;
                    if ((pData[31] & 1) != 0)
                        pRenderX86->m_lDrawMask = 2;
                    if ((pData[31] & 2) != 0)
                        pRenderX86->m_lDrawMask &= ~2u;
                    if (g_pd3dDevice)
                        g_pd3dDevice->SetRenderState(D3DRS_FILLMODE,
                            pRenderX86->m_lDrawMask ? D3DFILL_SOLID : D3DFILL_WIREFRAME);
                }
                break;
            case ZCmdList::CMD_VIEWPORT_SETUP:
                pRender->SetViewport(static_cast<float>(pData[0]), static_cast<float>(pData[1]),
                                      static_cast<float>(pData[2]), static_cast<float>(pData[3]));
                break;
            case ZCmdList::CMD_VIEWPORT_CLEAR:
                pRender->ClearViewport(pData[0], pData[1]);
                break;
            case ZCmdList::CMD_SHADOW_CASTERS_START_2:
                if (pContext)
                    pContext->m_lShadowId[2] = 4;
                pRender->m_bShadowMode = true;
                static_cast<ZRenderX86*>(pRender)->ShadowRenderCasters(0);
                break;
            case ZCmdList::CMD_SHADOW_RECEIVERS_START:
                static_cast<ZRenderX86*>(pRender)->ShadowRenderReceivers();
                break;
            case ZCmdList::CMD_LIGHT_SETUP:
                if (pContext && pCommand->m_lNrObjects >= 36)
                {
                    pContext->m_pCurrentLight = reinterpret_cast<const SPrimLight*>(pData[1]);
                    pContext->m_lShadowId[3] = pData[0] != 0;
                    memcpy(&pContext->m_pRenderView, pData + 2, 0x18);
                    memcpy(&pContext->m_ObjectToViewMatrix, pData + 8, sizeof(pContext->m_ObjectToViewMatrix));
                    memcpy(&pContext->m_aLightClipMatrix[0], pData + 20, sizeof(pContext->m_aLightClipMatrix[0]));
                }
                break;
            case ZCmdList::CMD_LIGHT_SETUP_2:
                if (pContext && pCommand->m_lNrObjects >= 36)
                {
                    pContext->m_pCurrentLight = reinterpret_cast<const SPrimLight*>(pData[1]);
                    pContext->m_lShadowId[3] = pData[0] != 0;
                    memcpy(&pContext->m_pRenderView, pData + 2, 0x18);
                    memcpy(&pContext->m_WorldToLightMatrix[0], pData + 8, sizeof(pContext->m_WorldToLightMatrix[0]));
                    memcpy(&pContext->m_aLightClipMatrix[1], pData + 20, sizeof(pContext->m_aLightClipMatrix[1]));
                }
                break;
            case ZCmdList::CMD_LIGHT_CAST_SHADOWS:
                if (pContext && pCommand->m_lNrObjects >= 29)
                {
                    memcpy(&pContext->m_WorldToLightMatrix[1], pData, sizeof(pContext->m_WorldToLightMatrix[1]));
                    memcpy(&pContext->m_aLightClipMatrix[2], pData + 12, sizeof(pContext->m_aLightClipMatrix[2]));
                    memcpy(&pContext->m_vFogNearPlane[1], pData + 28, sizeof(pContext->m_vFogNearPlane[1]));
                }
                break;
            case ZCmdList::CMD_SHADOW_SETUP:
                if (pContext && pCommand->m_lNrObjects >= 13)
                {
                    memcpy(&pContext->m_WorldToLightMatrix[2], pData, sizeof(pContext->m_WorldToLightMatrix[2]));
                    memcpy(&pContext->m_vFogNearPlane[0], pData + 12, sizeof(pContext->m_vFogNearPlane[0]));
                }
                break;
            case ZCmdList::CMD_DROP_SHADOW_SETUP:
                if (pCommand->m_lNrObjects >= 2 && pData[0])
                {
                    const auto* pLight = reinterpret_cast<const SPrimLight*>(pData[0]);
                    const uint32_t lMode = static_cast<ZRenderWintelD3D*>(pRender)->ShadowRenderCasterStart(
                        pLight->lLightType,
                        pContext ? *reinterpret_cast<const uint32_t*>(&pContext->m_vFogNearPlane[1]) : 0,
                        static_cast<float>(static_cast<uint8_t>(pData[1])));
                    static constexpr uint32_t aModeMap[6] = { 4, 5, 2, 3, 0, 1 };
                    if (pContext)
                        pContext->m_lShadowId[2] = lMode < 6 ? aModeMap[lMode] : lMode;
                }
                break;
            case ZCmdList::CMD_SHADOW_CASTERS_START:
                if (pCommand->m_pRenderEntryGeom)
                {
                    const auto* pLight = ZPrimHandle{ pCommand->m_pRenderEntryGeom->GetPrim() }.Get<SPrimLight>();
                    if (pLight)
                        static_cast<ZRenderWintelD3D*>(pRender)->ShadowRenderCasterStart(
                            pLight->lLightType, 1, 0.0f);
                }
                break;
            case ZCmdList::CMD_MIRROR_RENDER_REFLECTORS:
                static_cast<ZRenderX86*>(pRender)->MirrorRenderReflectors();
                break;
            case ZCmdList::CMD_MIRROR_RENDER_REFLECTED:
                static_cast<ZRenderX86*>(pRender)->MirrorRenderReflected();
                break;
            case ZCmdList::CMD_MIRROR_RENDER_ZHOLE:
                static_cast<ZRenderX86*>(pRender)->MirrorRenderZHole();
                break;
            case ZCmdList::CMD_MIRROR_END:
                static_cast<ZRenderX86*>(pRender)->MirrorRenderEnd();
                break;
            case ZCmdList::CMD_CLEAR_AND_BEGIN_ALPHABLEND_MAX:
                if (static_cast<ZRenderX86*>(pRender)->m_lDrawMask && g_pd3dDevice)
                {
                    g_pd3dDevice->GetViewport(&savedViewport);
                    auto* pWintel = static_cast<ZRenderWintelD3D*>(pRender);
                    g_pd3dDevice->SetRenderTarget(0, g_pRenderDll->m_lAntialias
                        ? pWintel->m_pAntialiasSurface : pWintel->m_pSurface16BC);
                    g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
                    g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
                    g_pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 5);
                    g_pd3dDevice->SetViewport(&savedViewport);
                }
                break;
            case ZCmdList::CMD_BEGIN_ALPHABLEND_MAX:
                if (static_cast<ZRenderX86*>(pRender)->m_lDrawMask && g_pd3dDevice)
                {
                    g_pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 1);
                    g_pd3dDevice->SetRenderTarget(0, static_cast<ZRenderWintelD3D*>(pRender)->m_pSurface16A4);
                    g_pd3dDevice->SetViewport(&savedViewport);
                }
                break;
            case ZCmdList::CMD_END_ALPHABLEND_MAX:
                if (static_cast<ZRenderX86*>(pRender)->m_lDrawMask && g_pd3dDevice && pCommand->m_lNrObjects >= 4)
                {
                    g_pd3dDevice->SetPixelShader(nullptr);
                    g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
                    g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
                    g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
                    g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
                    g_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
                    g_pd3dDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
                    auto* pWintel = static_cast<ZRenderWintelD3D*>(pRender);
                    if (g_pRenderDll->m_lAntialias && pWintel->m_pAntialiasSurface && pWintel->m_pSurface16BC)
                        g_pd3dDevice->StretchRect(pWintel->m_pAntialiasSurface, nullptr,
                                                 pWintel->m_pSurface16BC, nullptr, D3DTEXF_NONE);
                    g_pd3dDevice->SetTexture(0, pWintel->m_pTexture16B8);
                    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
                    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_CURRENT);
                    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
                    g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
                    g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
                    g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
                    g_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
                    g_pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
                    g_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
                    IDirect3DSurface9* pTarget = nullptr;
                    D3DSURFACE_DESC desc{};
                    g_pd3dDevice->GetRenderTarget(0, &pTarget);
                    if (pTarget) { pTarget->GetDesc(&desc); pTarget->Release(); }
                    const float fWidth = static_cast<float>(desc.Width);
                    const float fHeight = static_cast<float>(desc.Height);
                    ZDirect3DDevice::DrawPlaneUV(g_pd3dDevice, static_cast<float>(pData[0]),
                        static_cast<float>(pData[1]), static_cast<float>(pData[2]),
                        static_cast<float>(pData[3]), 0,
                        static_cast<float>(pData[0]) / fWidth,
                        static_cast<float>(pData[1]) / fHeight,
                        static_cast<float>(pData[0] + pData[2]) / fWidth,
                        static_cast<float>(pData[1] + pData[3]) / fHeight);
                    g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
                    g_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
                }
                break;
            case ZCmdList::CMD_SCISSOR_SETUP:
                if (pCommand->m_lNrObjects >= 5)
                    static_cast<ZRenderX86*>(pRender)->SetScissorRect(
                        *reinterpret_cast<const float*>(pData),
                        *reinterpret_cast<const float*>(pData + 1),
                        *reinterpret_cast<const float*>(pData + 2),
                        *reinterpret_cast<const float*>(pData + 3), pData[4] != 0);
                break;
            case ZCmdList::CMD_OBJECT_DRAW:
                if (pCommand->m_lNrObjects && pContext)
                    DrawCommandObjects(reinterpret_cast<ZRenderObjectInstance**>(const_cast<uint32_t*>(pData)),
                                       pCommand->m_lNrObjects, pCommand->m_lLayer, pContext);
                break;
            case ZCmdList::CMD_APPLY_POSTFILTER:
                if (pCommand->m_pRenderView && g_pd3dDevice)
                {
                    g_pd3dDevice->SynchronizeStateCaches();
                    if (auto* pPostFilter = pCommand->m_pRenderView->GetPostFilter())
                    {
                        pPostFilter->m_mViewMatrix = currentViewMatrix;
                        pPostFilter->Update(pCommand->m_pRenderView);
                    }
                }
                break;
            case ZCmdList::CMD_MIRROR_BEGIN:
                static_cast<ZRenderX86*>(pRender)->MirrorRenderBegin();
                break;
            case ZCmdList::CMD_OPTIMIZED_MIRROR_CLEAR_ZBUFFER:
                if (static_cast<ZRenderWintelD3D*>(pRender)->field_16AE && g_pd3dDevice)
                {
                    g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
                    g_pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 7);
                }
                break;
            case ZCmdList::CMD_OPTIMIZED_MIRROR_RENDER_REFLECTORS:
                if (static_cast<ZRenderWintelD3D*>(pRender)->field_16AE && g_pd3dDevice)
                {
                    g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
                    g_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
                    g_pd3dDevice->SetRenderState(D3DRS_STENCILENABLE, TRUE);
                    g_pd3dDevice->SetRenderState(D3DRS_STENCILWRITEMASK, 0xFFFFFFFF);
                    g_pd3dDevice->SetRenderState(D3DRS_STENCILMASK, 0xFFFFFFFF);
                    g_pd3dDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
                    g_pd3dDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);
                    g_pd3dDevice->SetRenderState(D3DRS_STENCILREF, 0);
                    g_pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
                }
                break;
            case ZCmdList::CMD_OPTIMIZED_MIRROR_SET_STENCIL_REF:
                if (static_cast<ZRenderWintelD3D*>(pRender)->field_16AE && g_pd3dDevice && pCommand->m_lNrObjects)
                    g_pd3dDevice->SetRenderState(D3DRS_STENCILREF, pData[0]);
                break;
            case ZCmdList::CMD_OPTIMIZED_MIRROR_RENDER_REFLECTED:
                if (static_cast<ZRenderWintelD3D*>(pRender)->field_16AE && g_pd3dDevice && pCommand->m_lNrObjects)
                {
                    g_pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
                    g_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
                    g_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
                    g_pd3dDevice->SetRenderState(D3DRS_STENCILENABLE, TRUE);
                    g_pd3dDevice->SetRenderState(D3DRS_STENCILREF, pData[0]);
                    g_pd3dDevice->SetRenderState(D3DRS_STENCILWRITEMASK, 0xFFFFFFFF);
                    g_pd3dDevice->SetRenderState(D3DRS_STENCILMASK, 0xFFFFFFFF);
                    g_pd3dDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
                    g_pd3dDevice->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
                    g_pd3dDevice->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
                    g_pd3dDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
                    IDirect3DSurface9* pTarget = nullptr;
                    D3DSURFACE_DESC desc{};
                    g_pd3dDevice->GetRenderTarget(0, &pTarget);
                    if (pTarget) { pTarget->GetDesc(&desc); pTarget->Release(); }
                    for (uint32_t i = 0; i < 4; ++i)
                        g_pd3dDevice->SetTexture(i, nullptr);
                    g_pd3dDevice->SetStreamSource(0, nullptr, 0, 0);
                    g_pd3dDevice->SetIndices(nullptr);
                    g_pd3dDevice->SetPixelShader(nullptr);
                    g_pd3dDevice->SetVertexShader(nullptr);
                    ZDirect3DDevice::DrawPlane(g_pd3dDevice, 0.0f, 0.0f,
                        static_cast<float>(desc.Width), static_cast<float>(desc.Height), 0, 0.0f, 1.0f, 1.0f);
                    g_pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 7);
                    g_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
                }
                break;
            case ZCmdList::CMD_OPTIMIZED_MIRROR_END:
                if (g_pd3dDevice)
                    g_pd3dDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
                break;
            case ZCmdList::CMD_CULL_NORMAL:
                static_cast<ZRenderX86*>(pRender)->SetCullingReversed(false);
                break;
            case ZCmdList::CMD_CULL_REVERSED:
                static_cast<ZRenderX86*>(pRender)->SetCullingReversed(true);
                break;
            case ZCmdList::CMD_PUSH_SURFACE:
                if (surfaceDepth < 8 && g_pd3dDevice)
                {
                    SurfaceState& state = surfaceStack[surfaceDepth++];
                    g_pd3dDevice->GetRenderTarget(0, &state.m_pRenderTarget);
                    g_pd3dDevice->GetDepthStencilSurface(&state.m_pDepthStencil);
                    g_pd3dDevice->GetViewport(&state.m_Viewport);
                    savedViewport = state.m_Viewport;
                }
                break;
            case ZCmdList::CMD_POP_SURFACE:
                if (surfaceDepth && g_pd3dDevice)
                {
                    SurfaceState& state = surfaceStack[--surfaceDepth];
                    g_pd3dDevice->SetRenderTarget(0, state.m_pRenderTarget);
                    g_pd3dDevice->SetDepthStencilSurface(state.m_pDepthStencil);
                    g_pd3dDevice->SetViewport(&state.m_Viewport);
                    if (state.m_pRenderTarget) state.m_pRenderTarget->Release();
                    if (state.m_pDepthStencil) state.m_pDepthStencil->Release();
                    savedViewport = state.m_Viewport;
                    for (uint32_t i = 0; i < g_dwTextureUnits; ++i)
                        g_pd3dDevice->SetTexture(i, nullptr);
                }
                break;
            case ZCmdList::CMD_SET_SURFACE:
                if (g_pd3dDevice)
                {
                    g_pd3dDevice->SetRenderTarget(0, reinterpret_cast<IDirect3DSurface9*>(pData[0]));
                    if (pData[1])
                        g_pd3dDevice->SetDepthStencilSurface(reinterpret_cast<IDirect3DSurface9*>(pData[1]));
                    for (uint32_t i = 0; i < g_dwTextureUnits; ++i)
                        g_pd3dDevice->SetTexture(i, nullptr);
                }
                break;
            case ZCmdList::CMD_COPY_SURFACE:
                if (g_pd3dDevice)
                    g_pd3dDevice->StretchRect(reinterpret_cast<IDirect3DSurface9*>(pData[0]), nullptr,
                                               reinterpret_cast<IDirect3DSurface9*>(pData[1]), nullptr, D3DTEXF_NONE);
                break;
            case ZCmdList::CMD_FILL_SURFACE:
                if (g_pd3dDevice)
                    g_pd3dDevice->ColorFill(reinterpret_cast<IDirect3DSurface9*>(pData[0]), nullptr, pData[1]);
                break;
            case ZCmdList::CMD_SET_COLOR_WRITE_MASK:
                if (g_pd3dDevice)
                    g_pd3dDevice->SetRenderState(static_cast<D3DRENDERSTATETYPE>(0xA8), pData[0]);
                break;
            case ZCmdList::CMD_BLUR_SURFACE:
                if (ZSharedResourcesD3D::g_pInstance)
                {
                    ZSharedResourcesD3D::g_pInstance->m_field00C4 = static_cast<int>(pData[0]);
                    ZSharedResourcesD3D::g_pInstance->BlurTexture(
                        reinterpret_cast<IDirect3DTexture9*>(pData[0]), 2.0f, 0.0f,
                        2, pData[2] != 0);
                }
                break;
            case ZCmdList::CMD_TEXTURE_PROJECTION_INDEX:
                if (pContext)
                    pContext->m_Unk180 = pData[0];
                break;
            case ZCmdList::CMD_RENDER_FULLSCREEN_QUAD:
                if (g_pd3dDevice && pCommand->m_lNrObjects >= 5)
                {
                    IDirect3DSurface9* pTarget = nullptr;
                    D3DSURFACE_DESC desc{};
                    g_pd3dDevice->GetRenderTarget(0, &pTarget);
                    if (pTarget) { pTarget->GetDesc(&desc); pTarget->Release(); }
                    DWORD oldZEnable = 0, oldZWrite = 0, oldColorMask = 0;
                    g_pd3dDevice->m_pDevice->GetRenderState(D3DRS_ZENABLE, &oldZEnable);
                    g_pd3dDevice->m_pDevice->GetRenderState(D3DRS_ZWRITEENABLE, &oldZWrite);
                    g_pd3dDevice->m_pDevice->GetRenderState(D3DRS_COLORWRITEENABLE, &oldColorMask);
                    g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
                    g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
                    g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, pData[3]);
                    g_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, pData[2]);
                    g_pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, pData[1]);
                    g_pd3dDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
                    g_pd3dDevice->SetTexture(0, nullptr);
                    g_pd3dDevice->SetPixelShader(nullptr);
                    ZDirect3DDevice::DrawPlane(g_pd3dDevice, 0.0f, 0.0f,
                        static_cast<float>(desc.Width), static_cast<float>(desc.Height), pData[0],
                        *reinterpret_cast<const float*>(pData + 4), 1.0f, 1.0f);
                    g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, oldZEnable);
                    g_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, oldZWrite);
                    g_pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, oldColorMask);
                }
                break;
            case ZCmdList::CMD_SET_PROJECTION:
                if (pContext)
                    memcpy(&pContext->m_ProjectionMatrix, pData, sizeof(pContext->m_ProjectionMatrix));
                break;
            case ZCmdList::CMD_PUSH_PROJECTION:
                if (pContext && projectionDepth < 8)
                    memcpy(&projectionStack[projectionDepth++], &pContext->m_ProjectionMatrix,
                           sizeof(pContext->m_ProjectionMatrix));
                break;
            case ZCmdList::CMD_POP_PROJECTION:
                if (pContext && projectionDepth)
                    memcpy(&pContext->m_ProjectionMatrix, &projectionStack[--projectionDepth],
                           sizeof(pContext->m_ProjectionMatrix));
                break;
            case ZCmdList::CMD_SET_FIRST_PERSON_BONES:
                if (pCommand->m_lNrObjects)
                    ZBoneModifyBase::UpdateFirstPersonBones(pRender, pData[0] != 0);
                break;
            default:
                // PC 0x004B0AF0 has no cases for 0x05, 0x0C, 0x22 or 0x26.
                break;
            }

            pCurrent = const_cast<char*>(reinterpret_cast<const char*>(pNext));
        }

        viewSpace.Reset();
        CleanupUnused();
    }

    ZRenderViewBase* ZRenderDrawD3D::NewView(ZRender* pRender, uint32_t lViewNumber, uint32_t lViewId)
    {
        return ZUniMemory::New<ZRenderViewD3D>(pRender, this, lViewNumber, lViewId);
    }

    void ZRenderDrawD3D::CalcBoneLightSources(ZBaseGeom* pBaseGeom, float* pDirectLights)
    {
        if (!pBaseGeom || !pBaseGeom->m_lDrawId)
            return;

        auto* pEntry = m_apRenderEntryLookup[pBaseGeom->m_lDrawId & 0x7FFFu];
        if (pEntry && (pEntry->m_lControl & ZRenderEntry::RE_HASBONES) != 0)
            static_cast<ZRenderEntryBones*>(pEntry)->m_pLightData = pDirectLights;
    }
}
