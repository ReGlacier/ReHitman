#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/Render/Material/ZRenderMaterialBinderParser.h>
#include <Glacier/Render/ZTextureD3D.h>
#include <Glacier/ZSTL/TIMETYPE.h>
#include <cstdint>


namespace Glacier
{
    using PrimHandleToPointerTable_t = void*[40960];
    using SpotMapArray_t = ZTextureD3D[2];
    using ShadowColorMapArray_t = ZTextureD3D[4];

    STATIC_GLOBAL_CLASS_INSTANCE(PrimHandleToPointerTable_t, g_apPrimHandleToPointerTable);
    STATIC_GLOBAL_CLASS_INSTANCE(uint32_t, g_lPrimHandleToPointerCount);
    STATIC_GLOBAL_CLASS_INSTANCE(float, g_fFogFar);
    STATIC_GLOBAL_CLASS_INSTANCE(bool, g_bd3dDeviceLost);
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
    STATIC_GLOBAL_CLASS_INSTANCE(bool, g_bIsResettingDevice);
    STATIC_GLOBAL_CLASS_INSTANCE(uint32_t, g_dwMinFilter);
    STATIC_GLOBAL_CLASS_INSTANCE(uint32_t, g_dwMagFilter);
    STATIC_GLOBAL_CLASS_INSTANCE(uint32_t, g_dwMipFilter);
    STATIC_GLOBAL_CLASS_INSTANCE(uint32_t, g_lMaxAnisotropy);
    STATIC_GLOBAL_CLASS_INSTANCE(TIMETYPE, g_ttLastVideoEndTime);

    extern ZRenderMaterialBinderParser::SMapper g_TranslatorMapper[68];
}