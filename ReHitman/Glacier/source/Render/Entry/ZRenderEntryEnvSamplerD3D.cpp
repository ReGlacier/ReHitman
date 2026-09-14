#include <Glacier/Geom/ZEnvSampler.h>
#include <Glacier/Render/Entry/ZRenderEntryEnvSamplerD3D.h>
#include <Glacier/Render/Cmd/ZRenderCommands.h>
#include <Glacier/Render/Draw/IDraw.h>
#include <Glacier/Render/Draw/ZRenderDraw.h>
#include <Glacier/Render/Entry/ZRenderEntryLists.h>
#include <Glacier/Render/View/ZRenderView.h>
#include <Glacier/Render/ZSharedResourcesD3D.h>
#include <Glacier/Render/ZViewSpace.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Render/D3D9.h>
#include <Glacier/ZUniMemory.h>
#include <cstring>

namespace Glacier
{
    ZRenderEntryEnvSamplerD3D::ZRenderEntryEnvSamplerD3D(const ZRenderEntryGeomCreateInfo& info)
        : ZRenderEntryGeomD3D(info),
          m_lFacesToUpdate(6),
          m_lCurrentFace(0)
    {
        m_lGeomListsControl |= 8;
        m_lIdentifier = 0;
        m_fCameraFar = 1000.0f;
        m_fFogNear = 0.0f;
        m_fFogFar = 1.0f;
        m_lFogColor.m_Value = 0;
        m_lUpdateFrequency = 0;
        m_bCurrentRoomOnly = false;

        if (info.m_pBaseGeom)
        {
            const auto* pGeom = info.m_pBaseGeom->GetGeom();
            if (pGeom && pGeom->IsDerivedFrom<ZEnvSampler>())
            {
                const auto* pSampler = static_cast<const ZEnvSampler*>(pGeom);
                m_lIdentifier = pSampler->m_lIdentifier;
                m_fCameraFar = pSampler->m_fCameraFar;
                m_fFogNear = pSampler->m_fFogNear;
                m_fFogFar = pSampler->m_fFogFar;
                m_lFogColor = pSampler->m_lFogColor;
                m_bCurrentRoomOnly = pSampler->m_bCurrentRoomOnly;
                m_lUpdateFrequency = static_cast<uint32_t>(pSampler->m_eUpdateFrequency);
            }
        }

        if (m_lIdentifier < 4 && m_lNumSamplers[m_lIdentifier] < 16)
        {
            m_pSamplers[m_lIdentifier * 16 + m_lNumSamplers[m_lIdentifier]++] = this;
        }
    }

    ZRenderEntryEnvSamplerD3D::~ZRenderEntryEnvSamplerD3D()
    {
        if (m_lIdentifier < 4)
        {
            const uint32_t lCount = m_lNumSamplers[m_lIdentifier];
            for (uint32_t i = 0; i < lCount; ++i)
            {
                if (m_pSamplers[m_lIdentifier * 16 + i] == this)
                {
                    m_pSamplers[m_lIdentifier * 16 + i] = m_pSamplers[m_lIdentifier * 16 + lCount - 1];
                    --m_lNumSamplers[m_lIdentifier];
                    break;
                }
            }
        }
    }

    ZRenderEntryEnvSamplerD3D::RENDERENTRY_BASETYPE ZRenderEntryEnvSamplerD3D::GetType() const
    {
        return RT_ENVSAMPLER;
    }

    void ZRenderEntryEnvSamplerD3D::GetVisible(ZCmdList* pCmdList, ZRenderEntryGeom*, ZViewSpace* pViewSpace, ZRenderView* pRenderView, ZRenderEntryLists* pEntryList)
    {
        const uint32_t lFrame = g_pSysInterface->m_lFrameCount;
        if (m_lLastFrameUpdated != lFrame)
        {
            m_lLastFrameUpdated = lFrame;
            ZVector3 vPosition;
            pViewSpace->GetPosition(vPosition);

            for (uint32_t group = 0; group < 4; ++group)
            {
                ZRenderEntryEnvSamplerD3D* pClosest = nullptr;
                float fClosest = 1.0e38f;
                for (uint32_t i = 0; i < m_lNumSamplers[group]; ++i)
                {
                    auto* pSampler = m_pSamplers[group * 16 + i];
                    const ZVector3 delta = pSampler->m_ObjectToWorldMatrix.p0 - vPosition;
                    const float fDistance = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
                    if (fDistance < fClosest)
                    {
                        fClosest = fDistance;
                        pClosest = pSampler;
                    }
                }
                if (m_pActive[group] != pClosest)
                {
                    m_pActive[group] = pClosest;
                    if (pClosest)
                        pClosest->m_lFacesToUpdate = 6;
                }
            }
        }

        if (m_lIdentifier >= 4 || m_pActive[m_lIdentifier] != this || !m_lFacesToUpdate)
            return;

        auto* pShared = ZSharedResourcesD3D::g_pInstance;
        if (!pShared)
            return;

        struct SurfaceCommand
        {
            IDirect3DSurface9* pSurface;
            IDirect3DSurface9* pDepth;
        };
        struct ProjectionCommand
        {
            ZMat4x4 m_Projection;
        };
        auto begin = [&](ZCmdList::CMD eType, uint32_t lLayer = 0u) {
            auto* pCmd = pCmdList->Current();
            pCmd->m_lType = eType;
            pCmd->m_pRenderEntryGeom = nullptr;
            pCmd->m_pCmdList = pCmdList;
            pCmd->m_pRenderView = pRenderView;
            pCmd->m_lLayer = lLayer;
            pCmd->m_lNrObjects = 0;
            pCmdList->NextCommand();
            return pCmd;
        };

        const uint32_t lFace = m_lCurrentFace++;
        if (m_lCurrentFace == 6)
            m_lCurrentFace = 0;
        --m_lFacesToUpdate;
        begin(static_cast<ZCmdList::CMD>(31));
        auto* pSurface = begin(static_cast<ZCmdList::CMD>(33));
        const SurfaceCommand surface { pShared->m_pEnvSurface[m_lIdentifier][lFace], pShared->m_pEnvDepth };
        pSurface->AddData(const_cast<SurfaceCommand*>(&surface), sizeof(SurfaceCommand));

        const ZVector3 vSamplerPosition = m_ObjectToWorldMatrix.p0;
        const ZVector3 aFaceDirection[6] =
        {
            { 1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f },
            { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f }
        };
        const ZVector3 aFaceUp[6] =
        {
            { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, 1.0f },
            { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }
        };
        ZVector3 vDirection = aFaceDirection[lFace];
        ZVector3 vUp = aFaceUp[lFace];
        TransformRootVector(vDirection, m_ObjectToWorldMatrix.m0);
        TransformRootVector(vUp, m_ObjectToWorldMatrix.m0);

        ZVector3 vTarget = vSamplerPosition;
        vTarget += vDirection;
        D3DXMATRIX viewMatrix;
        D3DXMATRIX projectionMatrix;
        D3DXMatrixLookAtLH(
            &viewMatrix,
            reinterpret_cast<const D3DXVECTOR3*>(&vSamplerPosition),
            reinterpret_cast<const D3DXVECTOR3*>(&vTarget),
            reinterpret_cast<const D3DXVECTOR3*>(&vUp));
        D3DXMatrixPerspectiveFovLH(&projectionMatrix, 1.5707964f, 1.0f, 5.0f,
            m_fCameraFar > 5.0f ? m_fCameraFar : 5.0001f);
        D3DXMATRIX viewProjection;
        D3DXMatrixMultiply(&viewProjection, &viewMatrix, &projectionMatrix);

        const ZVector3 vSavedPosition = pViewSpace->m_vPosition;
        const ZVector3 vSavedInnerRoom = pViewSpace->m_vPositionInnerRoom;
        pViewSpace->SetPosition(vSamplerPosition);
        pViewSpace->SetPositionInnerRoom(vSamplerPosition);

        ZVolumeList volumeList;
        pViewSpace->GetVisibleVolumes(&volumeList, false, nullptr);
        ZRenderEntry* apEntries[2048] {};
        const ZVector3 vObserver = m_ObjectToWorldMatrix.p0;
        auto* pDraw = IDraw::Instance<ZRenderDraw>();
        const uint32_t lCount = pDraw->CreateRenderEntries(apEntries, 2048, &volumeList, pEntryList, this,
            const_cast<float*>(&vObserver.x), 0.014994f);

        pSurface = begin(static_cast<ZCmdList::CMD>(2));
        const uint32_t aClear[2] { 3, m_lFogColor.m_Value };
        pSurface->AddData(const_cast<uint32_t*>(aClear), sizeof(aClear));

        struct ViewCommand
        {
            D3DXMATRIX m_View;
            D3DXMATRIX m_Projection;
            float m_fFogNear;
            float m_fFogFar;
            float m_fFogScale;
            float m_fPadding;
        };
        RE_VERIFY_SIZE(ViewCommand, 0x90);
        ViewCommand view { viewMatrix, projectionMatrix, m_fFogNear * m_fCameraFar,
            m_fFogFar * m_fCameraFar, static_cast<float>(m_lFogColor.m_Value), 0.0f };
        pSurface = begin(static_cast<ZCmdList::CMD>(0));
        pSurface->m_pRenderEntryGeom = this;
        pSurface->AddData(&view, sizeof(view));
        CmdDrawEntries(pCmdList, pRenderView, const_cast<const ZRenderEntry**>(apEntries), lCount,
            vObserver, 0.014994f, 7, 1, 7);
        begin(static_cast<ZCmdList::CMD>(32));
        pViewSpace->SetPosition(vSavedPosition);
        pViewSpace->SetPositionInnerRoom(vSavedInnerRoom);

        if (!m_lFacesToUpdate)
        {
            if (m_lUpdateFrequency == ZEnvSampler::UF_FAST)
                m_lFacesToUpdate = 6;
            else if (m_lUpdateFrequency == ZEnvSampler::UF_SLOW)
                m_lFacesToUpdate = 1;
        }
    }

    ZRenderEntryEnvSamplerD3D* ZRenderEntryEnvSamplerD3D::Create(const ZRenderEntryGeomCreateInfo& info)
    {
        if (!info.m_pBaseGeom)
        {
            return nullptr;
        }

        const auto* pGeom = info.m_pBaseGeom->GetGeom();
        const bool bEnvSampler = pGeom
            ? pGeom->IsDerivedFrom<ZEnvSampler>()
            : info.m_pBaseGeom->IsDerivedFromStdObj(ZEnvSampler::m_Id);

        if (!bEnvSampler)
            return nullptr;

        return ZUniMemory::New<ZRenderEntryEnvSamplerD3D>(info);
    }

    STATIC_CLASS_VAR_ARRAY_IMPL(ZRenderEntryEnvSamplerD3D, ZRenderEntryEnvSamplerD3D*, m_pActive, 4, 0x90DD38);
    STATIC_CLASS_VAR_ARRAY_IMPL(ZRenderEntryEnvSamplerD3D, uint32_t, m_lNumSamplers, 4, 0x90DD48);
    STATIC_CLASS_VAR_ARRAY_IMPL(ZRenderEntryEnvSamplerD3D, ZRenderEntryEnvSamplerD3D*, m_pSamplers, 64, 0x90DE7C);
    STATIC_CLASS_VAR_IMPL(ZRenderEntryEnvSamplerD3D, uint32_t, m_lLastFrameUpdated, 0x90DE78, 0xFFFFFFFFu);
}
