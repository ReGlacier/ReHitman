#include <Glacier/Render/Draw/ZRenderDrawD3D.h>
#include <Glacier/Render/View/ZRenderViewD3D.h>
#include <Glacier/Render/Entry/ZRenderEntryBones.h>
#include <Glacier/Render/Entry/ZRenderEntryLists.h>
#include <Glacier/Render/Cmd/ZCmdList.h>
#include <Glacier/Render/ZDirect3DDevice.h>
#include <Glacier/Render/ZRenderContext.h>
#include <Glacier/Render/ZRenderX86.h>
#include <Glacier/Render/ZViewSpace.h>
#include <Glacier/Render/ZSharedResourcesD3D.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/System/ZSysMem.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/IK/ZBoneModifyBase.h>
#include <Glacier/Geom/ZBaseGeom.h>
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

        // The PC associates entries with a render through an internal owner field which is
        // not present in this source's verified ZRenderEntry layout. Do not reproduce that
        // association with a guessed +0x60/+0x64 cast.
        // TODO: Finish me after the PC render-entry owner association is typed.
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

        // The current tree has no typed equivalent of the PC GetVisible collection call.
        // Keep command execution below live and deterministic rather than inventing entry
        // or view layouts.
        // TODO: Finish me after ZRenderDrawD3D::GetVisible collection is reversed.
        for (uint32_t i = 0; i < m_lRenderEntriesCount; ++i)
        {
            if (m_apRenderEntries[i])
                m_apRenderEntries[i]->Update();
        }

        m_pEntryReuse = nullptr;
        UpdateLightList(&entryLists);

        auto* pDrawUpdate = entryLists.GetList(ZRenderEntryLists::LT_DRAWUPDATE);
        for (int i = 0; i < pDrawUpdate->Count(); ++i)
        {
            ZRenderEntryGeom* pEntry = *pDrawUpdate->Get(i);
            if (pEntry && pEntry->GetBaseGeom())
                pEntry->GetBaseGeom()->FixLightList();
        }
        UpdateBoneModifiers(&entryLists);

        auto* pContext = static_cast<ZRenderX86*>(pRender)->m_pContext;
        D3DVIEWPORT9 savedViewport{};
        SurfaceState surfaceStack[8]{};
        uint32_t surfaceDepth = 0;
        ZMat3x3 unusedMatrix{};
        (void)unusedMatrix;

        for (char* pCurrent = s_CommandArena.m_List.m_Buffer;
             pCurrent < s_CommandArena.m_List.m_pCurrent; )
        {
            auto* pCommand = reinterpret_cast<ZCmdList::ZCmd*>(pCurrent);
            const auto* pData = reinterpret_cast<const uint32_t*>(pCurrent + sizeof(ZCmdList::ZCmd));
            const uint32_t* pNext = pData + pCommand->m_lNrObjects;

            switch (static_cast<uint32_t>(pCommand->m_lType))
            {
            case 0x00:
                // TODO: Finish me after ZRender::PCProjectionUpdate is typed.
                // Expected PC call: PC 0x4B0AF0 invokes the ZRender vtable projection
                // helper at +0x124 with payload words 34..36.
                break;
            case 0x01:
                pRender->SetViewport(static_cast<float>(pData[0]), static_cast<float>(pData[1]),
                                      static_cast<float>(pData[2]), static_cast<float>(pData[3]));
                break;
            case 0x02:
                pRender->ClearViewport(pData[0], pData[1]);
                break;
            case 0x0A:
                pRender->m_bShadowMode = true;
                static_cast<ZRenderX86*>(pRender)->ShadowRenderCasters(0);
                break;
            case 0x0B:
                static_cast<ZRenderX86*>(pRender)->ShadowRenderReceivers();
                break;
            case 0x03:
                // TODO: Finish me after the PC light payload is typed.
                // Expected PC call: 0x4B0AF0 copies the first light payload into
                // ZRenderContext (+0x0C..+0x1D8) and selects its light id.
                break;
            case 0x04:
                // TODO: Finish me after the PC secondary-light payload is typed.
                // Expected PC call: 0x4B0AF0 copies the secondary light payload into
                // ZRenderContext (+0x0C..+0x1D8).
                break;
            case 0x06:
                // TODO: Finish me after the PC shadow-projection payload is typed.
                // Expected PC call: 0x4B0AF0 writes ZRenderContext light clip data
                // and the shadow id from command 0x06.
                break;
            case 0x07:
                // TODO: Finish me after the PC world-to-light payload is typed.
                // Expected PC call: 0x4B0AF0 writes the world-to-light matrix and
                // light id from command 0x07.
                break;
            case 0x08:
                // TODO: Finish me after ZRender::SelectShadowMode is typed.
                // Expected PC call: pRender vtable +0x278 (PC 0x4B0AF0 command 0x08).
                break;
            case 0x09:
                // TODO: Finish me after ZRender::ShadowRenderStart payload is typed.
                // Expected PC call: pRender vtable +0x278 with the primitive light,
                // start flag 1 and pass 0.
                break;
            case 0x14:
                static_cast<ZRenderX86*>(pRender)->MirrorRenderBegin();
                break;
            case 0x15:
                static_cast<ZRenderX86*>(pRender)->MirrorRenderReflectors();
                break;
            case 0x16:
                static_cast<ZRenderX86*>(pRender)->MirrorRenderReflected();
                break;
            case 0x17:
                static_cast<ZRenderX86*>(pRender)->MirrorRenderZHole();
                break;
            case 0x0D:
            case 0x0E:
            case 0x0F:
            case 0x10:
            case 0x11:
                // TODO: Finish me after the PC alpha/scissor payloads are typed.
                // Expected PC calls: 0x4B0AF0 command 0x0D..0x11 surface and
                // alpha-blend helpers.
                break;
            case 0x12:
                // TODO: Finish me after the PC render-object instance traversal is typed.
                // Expected PC call: sub_490430, then the render-object-instance draw
                // virtual with the command's object payload.
                break;
            case 0x13:
                // TODO: Finish me after the PC post-filter API is typed.
                // Expected PC call: pRender vtable +0x1A4.
                break;
            case 0x18:
                // TODO: Finish me after ZRender::MirrorRenderEnd device-state API is typed.
                // Expected PC call: pRender vtable +0x1C0 and the PC mirror depth reset.
                break;
            case 0x19:
            case 0x1A:
            case 0x1B:
            case 0x1C:
                // TODO: Finish me after the optimized-mirror D3D state payload is typed.
                // Expected PC calls: 0x4B0AF0 commands 0x19..0x1C set D3D mirror
                // render states and draw the optimized mirror quad.
                break;
            case 0x1D:
                static_cast<ZRenderX86*>(pRender)->SetCullingReversed(false);
                break;
            case 0x1E:
                static_cast<ZRenderX86*>(pRender)->SetCullingReversed(true);
                break;
            case 0x1F:
                if (surfaceDepth < 8 && g_pd3dDevice)
                {
                    SurfaceState& state = surfaceStack[surfaceDepth++];
                    g_pd3dDevice->GetRenderTarget(0, &state.m_pRenderTarget);
                    g_pd3dDevice->GetDepthStencilSurface(&state.m_pDepthStencil);
                    g_pd3dDevice->GetViewport(&state.m_Viewport);
                    savedViewport = state.m_Viewport;
                }
                break;
            case 0x20:
                if (surfaceDepth && g_pd3dDevice)
                {
                    SurfaceState& state = surfaceStack[--surfaceDepth];
                    g_pd3dDevice->SetRenderTarget(0, state.m_pRenderTarget);
                    g_pd3dDevice->SetDepthStencilSurface(state.m_pDepthStencil);
                    g_pd3dDevice->SetViewport(&state.m_Viewport);
                    if (state.m_pRenderTarget) state.m_pRenderTarget->Release();
                    if (state.m_pDepthStencil) state.m_pDepthStencil->Release();
                    savedViewport = state.m_Viewport;
                }
                break;
            case 0x21:
                if (g_pd3dDevice)
                {
                    g_pd3dDevice->SetRenderTarget(0, reinterpret_cast<IDirect3DSurface9*>(pData[0]));
                    if (pData[1])
                        g_pd3dDevice->SetDepthStencilSurface(reinterpret_cast<IDirect3DSurface9*>(pData[1]));
                }
                break;
            case 0x23:
                if (g_pd3dDevice)
                    g_pd3dDevice->StretchRect(reinterpret_cast<IDirect3DSurface9*>(pData[0]), nullptr,
                                               reinterpret_cast<IDirect3DSurface9*>(pData[1]), nullptr, D3DTEXF_NONE);
                break;
            case 0x24:
                if (g_pd3dDevice)
                    g_pd3dDevice->ColorFill(reinterpret_cast<IDirect3DSurface9*>(pData[0]), nullptr, pData[1]);
                break;
            case 0x27:
                if (ZSharedResourcesD3D::g_pInstance)
                    ZSharedResourcesD3D::g_pInstance->BlurTexture(
                        reinterpret_cast<IDirect3DTexture9*>(pData[0]), 2.0f, 0.0f,
                        static_cast<int>(pData[1]), pData[2] != 0);
                break;
            case 0x28:
                if (pContext)
                    pContext->m_Unk180 = pData[0];
                break;
            case 0x2A:
                if (pContext)
                    memcpy(&pContext->m_ProjectionMatrix, pData, sizeof(pContext->m_ProjectionMatrix));
                break;
            case 0x2B:
                // TODO: Finish me after the PC projection stack owner is typed.
                // Expected PC call: push current ZRenderContext::m_ProjectionMatrix.
                break;
            case 0x2C:
                // TODO: Finish me after the PC projection stack owner is typed.
                // Expected PC call: pop ZRenderContext::m_ProjectionMatrix.
                break;
            case 0x2D:
                // TODO: Finish me after ZBoneModifyBase::UpdateFirstPersonBones gets
                // its exact PC signature. Expected PC call: UpdateFirstPersonBones(
                // pRender, payload[0] != 0).
                break;
            default:
                // TODO: Finish me after this command's PC payload/API is typed.
                // Expected PC call: the corresponding switch case in PC 0x4B0AF0.
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
