#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/Render/Material/ZRenderMaterialBinderParser.h>
#include <Glacier/Render/ZTextureD3D.h>
#include <Glacier/Render/D3D9.h>
#include <Glacier/ZSTL/TIMETYPE.h>
#include <cstdint>


namespace Glacier
{
    static constexpr int MAX_NUM_FREEPRIM = 256;

    using PrimHandleToPointerTable_t = void*[40960];
    using SpotMapArray_t = ZTextureD3D[2];
    using ShadowColorMapArray_t = ZTextureD3D[4];
    using SPrimToFreeList_t = uint32_t[MAX_NUM_FREEPRIM];

    STATIC_GLOBAL_CLASS_INSTANCE(PrimHandleToPointerTable_t, g_apPrimHandleToPointerTable);
    STATIC_GLOBAL_CLASS_INSTANCE(uint32_t, g_lPrimHandleToPointerCount);
    STATIC_GLOBAL_CLASS_INSTANCE(float, g_fFogFar);
    STATIC_GLOBAL_CLASS_INSTANCE(bool, g_bd3dDeviceLost);
    STATIC_GLOBAL_CLASS_INSTANCE(bool, g_bForceResetDevice); // 0x0090AF05 (unk_90AF05: set by Clear, forces the Flip device reset)
    STATIC_GLOBAL_CLASS_INSTANCE(IDirect3D9*, g_pd3dInterface);
    STATIC_GLOBAL_CLASS_INSTANCE(ZDirect3DDevice*, g_pd3dDevice);
    STATIC_GLOBAL_CLASS_INSTANCE(uint32_t, g_dwTextureUnits);
    STATIC_GLOBAL_CLASS_INSTANCE(uint32_t, g_dwTextureStages);
    STATIC_GLOBAL_CLASS_INSTANCE(uint32_t, g_dwFogColor);
    STATIC_GLOBAL_CLASS_INSTANCE(float, g_fFogNear);
    STATIC_GLOBAL_CLASS_INSTANCE(ZTextureD3D, g_texWhite);
    STATIC_GLOBAL_CLASS_INSTANCE(ZPostFilter*, g_pPostFilter);
    STATIC_GLOBAL_CLASS_INSTANCE(IDirect3DVertexBuffer9*, g_pVBPDT1);
    STATIC_GLOBAL_CLASS_INSTANCE(IDirect3DVertexBuffer9*, g_pVBPDT2);
    STATIC_GLOBAL_CLASS_INSTANCE(IDirect3DVertexBuffer9*, g_pVBPDT3);
    STATIC_GLOBAL_CLASS_INSTANCE(IDirect3DVertexBuffer9*, g_pVBPDT4);
    STATIC_GLOBAL_CLASS_INSTANCE(SpotMapArray_t, g_pSpotAttenuationMap);
    STATIC_GLOBAL_CLASS_INSTANCE(SpotMapArray_t, g_pSpotAttenuationMap2);
    STATIC_GLOBAL_CLASS_INSTANCE(IDirect3DCubeTexture9*, g_pNormalizedCubeMap);
    STATIC_GLOBAL_CLASS_INSTANCE(IDirect3DTexture9*, g_pAnisotropicMap);
    STATIC_GLOBAL_CLASS_INSTANCE(IDirect3DTexture9*, g_pSpecularMap);
    STATIC_GLOBAL_CLASS_INSTANCE(IDirect3DTexture9*, g_pScatterMap);
    STATIC_GLOBAL_CLASS_INSTANCE(IDirect3DTexture9*, g_pghPhaseMap);
    STATIC_GLOBAL_CLASS_INSTANCE(IDirect3DTexture9*, g_pRefractionMap);
    STATIC_GLOBAL_CLASS_INSTANCE(IDirect3DSurface9*, g_pShadowMapDepthBuffers);
    STATIC_GLOBAL_CLASS_INSTANCE(ShadowColorMapArray_t, g_texShadowMapColor);
    STATIC_GLOBAL_CLASS_INSTANCE(ZTextureD3D, g_texShadowCubeMapColor);       // 0x0090AC60
    STATIC_GLOBAL_CLASS_INSTANCE(ZTextureD3D, g_texNormalizer);              // 0x0090ACA8
    STATIC_GLOBAL_CLASS_INSTANCE(ZTextureD3D, g_texShadowMapDepth);          // 0x0090AD80
    STATIC_GLOBAL_CLASS_INSTANCE(ZTextureD3D, g_texCubeShadowColor);         // 0x0090AAC0
    STATIC_GLOBAL_CLASS_INSTANCE(ZTextureD3D, g_texShadowDepth);            // 0x0090AA78
    STATIC_GLOBAL_CLASS_INSTANCE(ZTextureD3D, g_texRefractionMap);          // 0x0090AC18
    STATIC_GLOBAL_CLASS_INSTANCE(bool, g_bIsResettingDevice);
    STATIC_GLOBAL_CLASS_INSTANCE(bool, g_bDisablePostEffects);    // 0x0090AEE4
    STATIC_GLOBAL_CLASS_INSTANCE(bool, g_lShowBufferAllocators);  // 0x0090AF24
    STATIC_GLOBAL_CLASS_INSTANCE(int, g_lOverrideSLI);            // 0x0090AF30
    STATIC_GLOBAL_CLASS_INSTANCE(bool, g_bD3DViewportInited);     // 0x0090AF34
    STATIC_GLOBAL_CLASS_INSTANCE(D3DVIEWPORT9, g_sD3DViewport);   // 0x0090A940
    STATIC_GLOBAL_CLASS_INSTANCE(bool, g_lShowShadowVolumes);     // 0x0090AF28
    STATIC_GLOBAL_CLASS_INSTANCE(uint32_t, g_dwMinFilter);
    STATIC_GLOBAL_CLASS_INSTANCE(uint32_t, g_dwMagFilter);
    STATIC_GLOBAL_CLASS_INSTANCE(uint32_t, g_dwMipFilter);
    STATIC_GLOBAL_CLASS_INSTANCE(uint32_t, g_lMaxAnisotropy);
    STATIC_GLOBAL_CLASS_INSTANCE(TIMETYPE, g_ttLastVideoEndTime);
    STATIC_GLOBAL_CLASS_INSTANCE(int32_t, g_iCurrentDynamicPrimBuffersCount);
    STATIC_GLOBAL_CLASS_INSTANCE(uint32_t, g_lPrimHandleToPointerFreeBack);
    STATIC_GLOBAL_CLASS_INSTANCE(uint32_t, g_lPrimToFreeCount);
    STATIC_GLOBAL_CLASS_INSTANCE(SPrimToFreeList_t, g_lPrimToFreeList);
    STATIC_GLOBAL_CLASS_INSTANCE(SHandleTableEntry*, g_pPrimHandleTable);
    STATIC_GLOBAL_CLASS_INSTANCE(float, g_fStaticShadowSampleHeightLimit);
    STATIC_GLOBAL_CLASS_INSTANCE(float, g_fTrisPerSec);         // 0x008C39F4 - triangles/sec stat (updated by ZRender::Update)
    STATIC_GLOBAL_CLASS_INSTANCE(TIMETYPE, g_ttLastTime);       // 0x008C39F8 - last update timestamp of the FPS counter
    STATIC_GLOBAL_CLASS_INSTANCE(uint32_t, g_lTrisPerSecFlags); // 0x008C39FC - FPS counter init flag (bit 0)
    STATIC_GLOBAL_CLASS_INSTANCE(uint32_t, g_lNumTris);         // 0x008C3A00 - accumulated triangle counter

    extern ZRenderMaterialBinderParser::SMapper g_TranslatorMapper[68];
}
