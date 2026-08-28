#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/Render/Prim/SPrimLight.h>
#include <cstdint>


namespace Glacier
{
    class ZRenderContext
    {
    public:
        // methods

        // members
        IView* m_pRenderView; // +0x0 Verified by mem analysis
        ZRender* m_pRender; // +0x4 Verified by ZRenderMaterialBinderSpriteD3DFX::Execute
        ZRenderObjectInstance* m_pRenderObjectInstance; // +0x8
        uint32_t m_UnkC; // +0xC
        uint32_t m_Unk10; // +0x10
        // TODO: Finish this place - light parameters written by render commands 3/4
        const SPrimLight* m_pLight[4]; // +0x14 (only m_pLight[0] confirmed as dword write)
        uint32_t m_lLightDrawEntryId; // +0x24
        const SPrimLight* m_pCurrentLight; // +0x28 Pointer to the current light data (was mislabelled as CAMERA_VIEW_DIRECTION)
        ZMatrix m_ObjectToWorldMatrix; // +0x2C Verified in ZRenderEntryGeom::SetRenderContext
        ZMatrix m_WorldToViewMatrix; // +0x5C Verified in ZRenderEntryGeom::SetRenderContext
        ZMatrix m_ObjectToViewMatrix; // +0x8C
        ZMatrix m_WorldToLightMatrix[3]; // +0xBC Verified in ZRenderDrawD3D::Update (commands 4/6/7)
        float m_vFogNearPlane[4]; // +0x14C Verified in ZRenderDrawD3D::Update
        uint32_t m_lShadowId[4]; // +0x15C Verified in ZRenderDrawD3D::Update
        const float* m_pDeformBones; // +0x16C (Verified in SetRenderContext)
        const float* m_pBonesLight; // +0x170 (Verified in SetRenderContext)
        uint32_t m_nCurrentPass; // +0x174 - Verified in ZRenderEntryGeom::SetRenderContext
        uint32_t m_lRenderFlags; // +0x178 - Verified in ZRenderEntryGeom::SetRenderContext
        float m_fObjectFade; // +0x17C Verified in ZRenderEntryGeom::SetRenderContext

        // === Newly reversed fields (PC) ===
        uint32_t m_Unk180; // +0x180 Set by render command 0x28
        float m_fZBias; // +0x184 Written by ZRenderMaterialBinderRenderStateD3DFX::Execute (XBOX name: m_fZBias)
        RE_ADD_PADDING(0x10); // +0x188..0x198 (unknown)
        ZMat4x4 m_ProjectionMatrix; // +0x198 Main projection matrix (render command 0 / push-pop commands 0x2A-0x2C)
        ZMat4x4 m_aLightClipMatrix[3]; // +0x1D8 Per-light clip/projection matrices (render commands 3/4/6)
    };
    RE_VERIFY_SIZE(ZRenderContext, 0x298);

    RE_VERIFY_OFFSET(ZRenderContext, m_pRenderView, 0x0); // Verified
    RE_VERIFY_OFFSET(ZRenderContext, m_pRender, 0x4); // Verified
    RE_VERIFY_OFFSET(ZRenderContext, m_pRenderObjectInstance, 0x08); // Verified
    RE_VERIFY_OFFSET(ZRenderContext, m_pCurrentLight, 0x28); // Verified in ZRenderDrawD3D::Update
    RE_VERIFY_OFFSET(ZRenderContext, m_ObjectToWorldMatrix, 0x2C); // Verified
    RE_VERIFY_OFFSET(ZRenderContext, m_WorldToViewMatrix, 0x5C); // Verified
    RE_VERIFY_OFFSET(ZRenderContext, m_ObjectToViewMatrix, 0x8C); // Verified
    RE_VERIFY_OFFSET(ZRenderContext, m_WorldToLightMatrix, 0xBC); // Verified
    RE_VERIFY_OFFSET(ZRenderContext, m_vFogNearPlane, 0x14C); // Verified
    RE_VERIFY_OFFSET(ZRenderContext, m_lShadowId, 0x15C); // Verified
    RE_VERIFY_OFFSET(ZRenderContext, m_pDeformBones, 0x16C);
    RE_VERIFY_OFFSET(ZRenderContext, m_pBonesLight, 0x170);
    RE_VERIFY_OFFSET(ZRenderContext, m_nCurrentPass, 0x174);
    RE_VERIFY_OFFSET(ZRenderContext, m_lRenderFlags, 0x178);
    RE_VERIFY_OFFSET(ZRenderContext, m_fObjectFade, 0x17C); // Verified
    RE_VERIFY_OFFSET(ZRenderContext, m_Unk180, 0x180); // Verified in ZRenderDrawD3D::Update
    RE_VERIFY_OFFSET(ZRenderContext, m_fZBias, 0x184); // Verified in ZRenderMaterialBinderRenderStateD3DFX::Execute
    RE_VERIFY_OFFSET(ZRenderContext, m_ProjectionMatrix, 0x198); // Verified in ZRenderDrawD3D::Update
    RE_VERIFY_OFFSET(ZRenderContext, m_aLightClipMatrix, 0x1D8); // Verified in ZRenderDrawD3D::Update
}
