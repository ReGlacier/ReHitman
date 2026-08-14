#include <Glacier/Render/Globals.h>


namespace Glacier
{
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(PrimHandleToPointerTable_t, g_apPrimHandleToPointerTable, 0x008C3E08, {});
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(uint32_t, g_lPrimHandleToPointerCount, 0x008EBE0C, 0);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(float, g_fFogFar, 0x007F6F3C, 1.0f);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(bool, g_bd3dDeviceLost, 0x0090AF04, false);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(IDirect3D9*, g_pd3dInterface, 0x0090AF08, nullptr);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(ZDirect3DDevice*, g_pd3dDevice, 0x0090AF0C, nullptr);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(uint32_t, g_dwTextureUnits, 0x0090AF10, 0);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(uint32_t, g_dwTextureStages, 0x0090AF14, 0);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(uint32_t, g_dwFogColor, 0x0090AF18, 0);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(float, g_fFogNear, 0x0090AF1C, 0.1f);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(ZTextureD3D, g_texWhite, 0x0090ADC8, {});
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(ZPostFilter*, g_pPostFilter, 0x0090DC68, nullptr);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(IDirect3DVertexBuffer9*, g_pVBPDT1, 0x0090DC78, nullptr);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(IDirect3DVertexBuffer9*, g_pVBPDT2, 0x0090DC7C, nullptr);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(IDirect3DVertexBuffer9*, g_pVBPDT3, 0x0090DC80, nullptr);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(IDirect3DVertexBuffer9*, g_pVBPDT4, 0x0090DC84, nullptr);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(IDirect3DSurface9*, g_pShadowMapDepthBuffers, 0x0090AA70, nullptr);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(SpotMapArray_t, g_pSpotAttenuationMap, 0x0090AA14, {});
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(SpotMapArray_t, g_pSpotAttenuationMap2, 0x0090A98C, {});
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(ShadowColorMapArray_t, g_texShadowMapColor, 0x0090AB34, {});
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(bool, g_bIsResettingDevice, 0x0090D58C, false);

    // stuff
    using SMapper = ZRenderMaterialBinderParser::SMapper;
    using RT_TYPE = ZRenderMaterialBinderParser::RT_TYPE;
    using RT_NAME = ZRenderMaterialBinderParser::RT_NAME;

    SMapper g_TranslatorMapper[68] = {
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_MODEL_VIEW_PROJ,                 "m44ModelViewProj" },
        { RT_TYPE::RTT_MATERIAL, RT_NAME::RTN_VIEW_PROJ,                       "m44ViewProj" },
        { RT_TYPE::RTT_MATERIAL, RT_NAME::RTN_VIEW,                            "m44View" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_WORLD,                           "m44World" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_MODEL_VIEW,                      "m44ModelView" },
        { RT_TYPE::RTT_MATERIAL, RT_NAME::RTN_PROJ,                            "m44Proj" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_WORLD_VIEW_PROJ_SCALE,           "m44WorldViewProjScale" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_VIEW_PROJ_SCALE,                 "m44ViewProjScale" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_WORLD_LIGHT,                     "m44WorldLight" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_WORLD_LIGHT2,                    "m44WorldLight2" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_WORLD_LIGHT_PROJ,                "m44WorldLightProj" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_WORLD_LIGHT_CLIP_PROJ,           "m44WorldLightClipProj" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_WORLD_SHADOW_PROJ,               "m44WorldShadowProj" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_WORLD_SHADOW_LIGHT_PROJ,         "m44WorldShadowLightProj" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_WORLD_SHADOW_CLIP,               "m44WorldShadowClip" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_WORLD_SHADOW,                    "m44WorldShadow" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_OBJ_TO_CUBE_SPACE,               "m34ObjToCubeSpace" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_EYE_POS_OBJECT,                  "v4EyePosObject" },
        { RT_TYPE::RTT_MATERIAL, RT_NAME::RTN_EYE_POS_WORLD,                   "v4EyePosWorld" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_LIGHT_POS_OBJECT,                "v4LightPosObject" },
        { RT_TYPE::RTT_MATERIAL, RT_NAME::RTN_LIGHT_BOUNDS_OBJECT,             "v4LightBoundsObject" },
        { RT_TYPE::RTT_MATERIAL, RT_NAME::RTN_LIGHT_ATTRIBUTES,                "v4LightAttributes" },
        { RT_TYPE::RTT_MATERIAL, RT_NAME::RTN_LIGHT_COLOR,                     "v4LightColor" },
        { RT_TYPE::RTT_MATERIAL, RT_NAME::RTN_LIGHT_AMBIENT_COLOR,             "v4LightAmbientColor" },
        { RT_TYPE::RTT_MATERIAL, RT_NAME::RTN_MAP_CUBE_NORMALIZER,             "mapCubeNormalizer" },
        { RT_TYPE::RTT_MATERIAL, RT_NAME::RTN_MAP_CUBE_NORMALIZER,             "mapCubeNormalizer0" },
        { RT_TYPE::RTT_MATERIAL, RT_NAME::RTN_MAP_CUBE_NORMALIZER,             "mapCubeNormalizer1" },
        { RT_TYPE::RTT_MATERIAL, RT_NAME::RTN_MAP_CUBE_NORMALIZER,             "mapCubeNormalizer2" },
        { RT_TYPE::RTT_MATERIAL, RT_NAME::RTN_MAP_CUBE_NORMALIZER,             "mapCubeNormalizer3" },
        { RT_TYPE::RTT_MATERIAL, RT_NAME::RTN_MAP_SCATTER,                     "mapScatter" },
        { RT_TYPE::RTT_MATERIAL, RT_NAME::RTN_MAP_PHASE,                       "mapPhase" },
        { RT_TYPE::RTT_MATERIAL, RT_NAME::RTN_MAP_REFRACTION,                  "mapRefraction" },
        { RT_TYPE::RTT_MATERIAL, RT_NAME::RTN_MAP_SHADOW_COLOR,                "mapShadowColor" },
        { RT_TYPE::RTT_MATERIAL, RT_NAME::RTN_MAP_SHADOW_COLOR2,               "mapShadowColor2" },
        { RT_TYPE::RTT_MATERIAL, RT_NAME::RTN_MAP_SHADOW_DROP_COLOR,            "mapShadowDropColor" },
        { RT_TYPE::RTT_MATERIAL, RT_NAME::RTN_MAP_CUBE_SHADOW_COLOR,           "mapCubeShadowColor" },
        { RT_TYPE::RTT_MATERIAL, RT_NAME::RTN_MAP_SHADOW_DEPTH,                "mapShadowDepth" },
        { RT_TYPE::RTT_MATERIAL, RT_NAME::RTN_MAP_REFLECTION_2D,               "mapReflection2D" },
        { RT_TYPE::RTT_MATERIAL, RT_NAME::RTN_MAP_REFRACTION_2D,               "mapRefraction2D" },
        { RT_TYPE::RTT_MATERIAL, RT_NAME::RTN_MAP_LIGHT_ATTENUATION,           "mapLightAttenuation" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_V4_BONES,                        "v4Bones" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_V4_BONES2,                       "v4Bones2" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_V4_BONES3,                       "v4Bones3" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_V4_BONES_LIGHTS,                 "v4BonesLights" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_V4_BONES_LIGHTS_AMBIENT,         "v4BonesLightsAmbient" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_V4_BONES_LIGHTS_11,              "v4BonesLights11" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_BONE_SHADOW_MULTIPLIER,          "g_vBoneShadowMultiplier" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_TWEEN_FACTOR,                    "vTweenFactor" },
        { RT_TYPE::RTT_MATERIAL, RT_NAME::RTN_ENGINE_TIME,                     "vEngineTime" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_SPHERICAL_HARMONICS,             "vSphericalHarmonics" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_OBJECT_RECEIVE_SHADOW,           "vObjectReceiveShadow" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_FOG_NEAR_PLANE_OS,               "gi_vFogNearPlaneOS" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_ZBIAS_OFFSET,                    "go_vZBiasOffset" },
        { RT_TYPE::RTT_MATERIAL, RT_NAME::RTN_DEBUG_DISABLE_LIGHTING,          "vDebugDisableLighting" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_DEBUG_WIREFRAME_COLOR,           "vDebugWireFrameColor" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_STATIC_SHADOW_COLOR,             "vStaticShadowColor" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_STATIC_SHADOW_PROJECTION_MATRIX, "m44StaticShadowProjectionMatrix" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_TEXTURE_PROJECTION_MATRIX,       "m44TextureProjectionMatrix" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_TEXTURE_PROJECTION_COLOR,        "v4TextureProjectionColor" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_TEXTURE_PROJECTION_LIGHT_POS,    "v4TextureProjectionLightPos" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_MAP_TEXTURE_PROJECTION_2D,       "mapTextureProjection2D" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_DROP_SHADOW_COLOR,               "g_vDropShadowColor" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_SCATTER_BILLBOARD,               "v4ScatterBillboard" },
        { RT_TYPE::RTT_OBJECT,   RT_NAME::RTN_WATER_PATCH_SINE_WAVES,          "v4WaterPatchSineWaves" },
        { RT_TYPE::RTT_MATERIAL, RT_NAME::RTN_MAP_BLACK_AND_WHITE,             "mapBlackAndWhite" },
        { RT_TYPE::RTT_MATERIAL, RT_NAME::RTN_MAP_BACK_BUFFER,                 "mapBackBuffer" },
        { RT_TYPE::RTT_MATERIAL, RT_NAME::RTN_VIEWPORT,                        "v4Viewport" },
        { RT_TYPE::RTT_NONE,     RT_NAME::RTN_UNKNOWN,                         "" }
    };
}