#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Render/Fwd.h>
#include <cstdint>


namespace Glacier
{
    // NOT FINISHED YET
    // TODO: Finish me
    class ZRenderContext
    {
    public:
        // types
        enum CAMERA_VIEW_DIRECTION : int32_t
        {
            POSITIVE_X = 0x0,
            NEGATIVE_X = 0x1,
            POSITIVE_Y = 0x2,
            NEGATIVE_Y = 0x3,
            POSITIVE_Z = 0x4,
            NEGATIVE_Z = 0x5,
        };

        // methods
        
        // members
        IView* m_pRenderView; // Verified by mem analysis
        ZRender* m_pRender; // Verified by ZRenderMaterialBinderSpriteD3DFX::Execute
        ZRenderObjectInstance *m_pRenderObjectInstance;
        uint32_t m_UnkC;
        uint32_t m_Unk10;
        const SPrimLight* m_pLight[4];
        uint32_t m_lLightDrawEntryId;
        CAMERA_VIEW_DIRECTION m_CameraViewDirection;
        ZMatrix m_ObjectToWorldMatrix; // 0x2C Verified in ZRenderEntryGeom::SetRenderContext
        ZMatrix m_WorldToViewMatrix; // 0x5C Verified in ZRenderEntryGeom::SetRenderContext
        ZMatrix m_ObjectToViewMatrix; // 0x8C
        // ~~~~~~~~~~~~~ Need confirm this block ~~~~~~~~~~~~~
        ZMatrix m_WorldToLightMatrix[3]; // 0xBC
        float m_vFogNearPlane[4]; // 0x14C
        uint32_t m_lShadowId[4]; // 0x15C
        const float* m_pDeformBones; // 0x16C (Verified in SetRenderContext)
        const float* m_pBonesLight; // 0x170 (Verified in SetRenderContext)
        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        uint32_t m_nCurrentPass; // 0x174 - Verified in ZRenderEntryGeom::SetRenderContext
        uint32_t m_lRenderFlags; // 0x178 - Verified in ZRenderEntryGeom::SetRenderContext
        float m_fObjectFade; // 0x17C Verified in ZRenderEntryGeom::SetRenderContext
        // Maybe more fields?
        // ...
    };
    // Size: TBD

    RE_VERIFY_OFFSET(ZRenderContext, m_pRenderView, 0x0); // Verified
    RE_VERIFY_OFFSET(ZRenderContext, m_pRender, 0x4); // Verified
    RE_VERIFY_OFFSET(ZRenderContext, m_pRenderObjectInstance, 0x08); // Verified
    RE_VERIFY_OFFSET(ZRenderContext, m_ObjectToWorldMatrix, 0x2C); // Verified
    RE_VERIFY_OFFSET(ZRenderContext, m_WorldToViewMatrix, 0x5C); // Verified
    RE_VERIFY_OFFSET(ZRenderContext, m_pDeformBones, 0x16C);
    RE_VERIFY_OFFSET(ZRenderContext, m_pBonesLight, 0x170);
    RE_VERIFY_OFFSET(ZRenderContext, m_nCurrentPass, 0x174);
    RE_VERIFY_OFFSET(ZRenderContext, m_lRenderFlags, 0x178);
    RE_VERIFY_OFFSET(ZRenderContext, m_fObjectFade, 0x17C); // Verified
}