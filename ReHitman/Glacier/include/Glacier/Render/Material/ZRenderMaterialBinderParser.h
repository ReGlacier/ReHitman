#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <cstdint>


namespace Glacier
{
    class ZRenderMaterialBinderParser
    {
    public:
        // types
        enum RT_TYPE : uint32_t 
        {
            RTT_NONE     = 0x0,
            RTT_MATERIAL = 0x1,
            RTT_OBJECT   = 0x2
        };

        enum class RT_NAME : uint32_t 
        {
            RTN_UNKNOWN                           = 0,
            RTN_MODEL_VIEW_PROJ                   = 1,   // m44ModelViewProj
            RTN_VIEW_PROJ                         = 2,   // m44ViewProj
            RTN_VIEW                              = 3,   // m44View
            RTN_WORLD                             = 4,   // m44World
            RTN_MODEL_VIEW                        = 5,   // m44ModelView
            RTN_PROJ                              = 6,   // m44Proj
            RTN_WORLD_VIEW_PROJ_SCALE             = 7,   // m44WorldViewProjScale
            RTN_WORLD_LIGHT                       = 8,   // m44WorldLight
            RTN_WORLD_LIGHT2                      = 9,   // m44WorldLight2
            RTN_WORLD_LIGHT_PROJ                  = 10,  // m44WorldLightProj
            RTN_WORLD_SHADOW_PROJ                 = 11,  // m44WorldShadowProj
            RTN_WORLD_SHADOW_CLIP                 = 12,  // m44WorldShadowClip
            RTN_WORLD_SHADOW                      = 13,  // m44WorldShadow
            RTN_WORLD_LIGHT_CLIP_PROJ             = 14,  // m44WorldLightClipProj
            RTN_OBJ_TO_CUBE_SPACE                 = 15,  // m34ObjToCubeSpace
            RTN_EYE_POS_OBJECT                    = 16,  // v4EyePosObject
            RTN_EYE_POS_WORLD                     = 17,  // v4EyePosWorld
            RTN_LIGHT_POS_OBJECT                  = 18,  // v4LightPosObject
            RTN_LIGHT_BOUNDS_OBJECT               = 19,  // v4LightBoundsObject
            RTN_LIGHT_ATTRIBUTES                  = 20,  // v4LightAttributes
            RTN_LIGHT_COLOR                       = 21,  // v4LightColor
            RTN_LIGHT_AMBIENT_COLOR               = 22,  // v4LightAmbientColor
            RTN_MAP_CUBE_NORMALIZER               = 25,  // mapCubeNormalizer / mapCubeNormalizerX
            RTN_MAP_SCATTER                       = 26,  // mapScatter
            RTN_MAP_PHASE                         = 27,  // mapPhase
            RTN_MAP_REFRACTION                    = 28,  // mapRefraction
            RTN_MAP_SHADOW_COLOR                  = 29,  // mapShadowColor
            RTN_MAP_SHADOW_COLOR2                 = 30,  // mapShadowColor2
            RTN_MAP_SHADOW_DROP_COLOR             = 31,  // mapShadowDropColor
            RTN_MAP_CUBE_SHADOW_COLOR             = 32,  // mapCubeShadowColor
            RTN_MAP_SHADOW_DEPTH                  = 33,  // mapShadowDepth
            RTN_MAP_REFLECTION_2D                 = 38,  // mapReflection2D
            RTN_MAP_REFRACTION_2D                 = 39,  // mapRefraction2D
            RTN_MAP_LIGHT_ATTENUATION             = 40,  // mapLightAttenuation
            RTN_V4_BONES                          = 41,  // v4Bones
            RTN_V4_BONES2                         = 42,  // v4Bones2
            RTN_V4_BONES3                         = 43,  // v4Bones3
            RTN_V4_BONES_LIGHTS                   = 44,  // v4BonesLights
            RTN_V4_BONES_LIGHTS_AMBIENT           = 45,  // v4BonesLightsAmbient
            RTN_V4_BONES_LIGHTS_11                = 46,  // v4BonesLights11
            RTN_BONE_SHADOW_MULTIPLIER            = 47,  // g_vBoneShadowMultiplier
            RTN_TWEEN_FACTOR                      = 48,  // vTweenFactor
            RTN_ENGINE_TIME                       = 49,  // vEngineTime
            RTN_SPHERICAL_HARMONICS               = 50,  // vSphericalHarmonics
            RTN_OBJECT_RECEIVE_SHADOW             = 51,  // vObjectReceiveShadow
            RTN_FOG_NEAR_PLANE_OS                 = 52,  // gi_vFogNearPlaneOS
            RTN_ZBIAS_OFFSET                      = 53,  // go_vZBiasOffset
            RTN_DEBUG_DISABLE_LIGHTING            = 54,  // vDebugDisableLighting
            RTN_DEBUG_WIREFRAME_COLOR             = 55,  // vDebugWireFrameColor
            RTN_STATIC_SHADOW_COLOR               = 56,  // vStaticShadowColor
            RTN_STATIC_SHADOW_PROJECTION_MATRIX   = 57,  // m44StaticShadowProjectionMatrix
            RTN_DROP_SHADOW_COLOR                 = 58,  // g_vDropShadowColor
            RTN_SCATTER_BILLBOARD                 = 59,  // v4ScatterBillboard
            RTN_WATER_PATCH_SINE_WAVES            = 60,  // v4WaterPatchSineWaves
            RTN_MAP_BLACK_AND_WHITE               = 61,  // mapBlackAndWhite
            RTN_MAP_BACK_BUFFER                   = 62,  // mapBackBuffer
            RTN_WORLD_SHADOW_LIGHT_PROJ           = 63,  // m44WorldShadowLightProj
            RTN_TEXTURE_PROJECTION_MATRIX         = 64,  // m44TextureProjectionMatrix
            RTN_TEXTURE_PROJECTION_COLOR          = 65,  // v4TextureProjectionColor
            RTN_TEXTURE_PROJECTION_LIGHT_POS      = 66,  // v4TextureProjectionLightPos
            RTN_MAP_TEXTURE_PROJECTION_2D         = 67,  // mapTextureProjection2D
            RTN_VIEW_PROJ_SCALE                   = 68,  // m44ViewProjScale
            RTN_VIEWPORT                          = 69   // v4Viewport
        };

        struct SMapper 
        {
            RT_TYPE Type;
            RT_NAME Name;
            const char* pszName;
        };

        // vtbl
        virtual ~ZRenderMaterialBinderParser();
        virtual void CreateBinders(ZRenderMaterialBinderList* pMaterialBinderList, ZRenderMaterialInstance* pMaterialInstance);
        virtual void CreatePropertyBinders(ZRenderMaterialBinderList* pMaterialBinderList, ZRenderMaterialInstance* pMaterialInstance);
        virtual void CreateBuiltInBinders(ZRenderMaterialBinderList* pMaterialBinderList, ZRenderMaterialInstance* pMaterialInstance);
        virtual void CreatePropertyBinderTexture(
            ZRenderBinderList* pBinderList, 
            const char* pszBinderName, 
            uint32_t lTextureId, 
            const char* pszMinFilter, 
            const char* pszMagFilter, 
            const char* pszMipFilter, 
            const char* pszTilingU, 
            const char* pszTilingV, 
            const char* pszTilingW) = 0;
        virtual void CreatePropertyBinderFloat(ZRenderBinderList* pMaterialBinderList, const char* pszBinderName, const float* pFloats, uint32_t lNumFloats) = 0;
        virtual void CreatePropertyBinderBool(ZRenderBinderList* pMaterialBinderList, const char* pszBinderName, const uint32_t* pBools, uint32_t lNumBools) = 0;
        virtual void CreatePropertyBinderEnum(ZRenderBinderList* pMaterialBinderList, const char* pszBinderName, const char* pszEnum) = 0;
        virtual void CreatePropertyBinderColor(ZRenderBinderList* pMaterialBinderList, const char* pszBinderName, const float* pColor) = 0;
        virtual void CreatePropertyBinderContext(ZRenderBinderList* pMaterialBinderList, const char* pszBinderName, uint32_t lContextType) = 0;
        virtual void CreatePropertyBinderSprite(ZRenderBinderList* pMaterialBinderList) = 0;
        virtual void CreatePropertyBinderRenderState(
            ZRenderBinderList* pMaterialBinderList, 
            bool bBlendEnabled,
            const char* pszBlendMode, 
            float fOpacity, 
            bool bAlphaTestEnabled, 
            uint32_t lAlphaTestRef,
            bool bFogEnabled,
            const char* pszCullMode, 
            uint32_t lZBias,
            float fZOffset) = 0;
        virtual void CreatePropertyBinderScroll(ZRenderBinderList* pMaterialBinderList, const char* pszBinderName, const float* pfScrollSpeed) = 0;
        virtual void VerifyBinders(int lLayer, ZRenderMaterialBinderList* pMaterialBinderList, ZRenderMaterialInstance* pMaterialInstance);

        // methods
        ZRenderMaterialBinderParser();

        // members
        const ZRenderMaterialBinderParser::SMapper* m_pTranslatorMapper { nullptr };
    };
}