#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/Render/ZBlurTextureWintelD3D.h>


namespace Glacier
{
    class ZSharedResourcesD3D
    {
    public:
        // constants
        static constexpr int MAX_ENV_TEXTURES_NR = 4;
        static constexpr int MAX_ENV_SURFACES_NR = 6;

        // types
        enum SHADERQUALITY : uint32_t
        {
            SHADERQUALITY_LOW = 0x0,
            SHADERQUALITY_MEDIUM = 0x1,
            SHADERQUALITY_HIGH = 0x2,
            SHADERQUALITY_VERYHIGH = 0x3
        };

        enum SHADOWMODE : uint32_t
        {
            SHADOWMODE_STENCIL = 0x0,
            SHADOWMODE_COLOR = 0x1,
            SHADOWMODE_DEPTH = 0x2
        };

        // static
        STATIC_CLASS_VAR(ZSharedResourcesD3D, ZSharedResourcesD3D*, g_pInstance);
        STATIC_CLASS_VAR(ZSharedResourcesD3D, bool, g_bSVBLockAcquired);

        // static methods
        static void Create();
        static void Release();

        // methods
        ZSharedResourcesD3D();
        ~ZSharedResourcesD3D();

        void Free();
        void Allocate();
        void BlurTexture(IDirect3DTexture9* pTexture, float fBlur1, float fBlur2, int lFlags, bool);
        void LockBuffersForCopy();

        // members
        IDirect3DVertexDeclaration9* m_pVDNull;
        ZRenderMaterialResourceD3DFX* m_pResourceFX;
        ZRenderDrawD3D* m_pRenderDrawShared;
        ZRD3DStaticVB* m_pSVB;
        ZRD3DStaticIB* m_pSIB;
        ZRX86AllocIf* m_pVertexAllocator;
        ZRX86AllocIf* m_pIndexAllocator;
        ZRD3DDynamicVB* m_pDVB;
        ZRD3DDynamicIB* m_pDIB;
        ZRD3DDynamicIB32* m_pDIB32;
        int m_lShaderQuality;
        bool m_bUseVertexShader;
        bool m_bUsePixelShader;
        bool m_bDebugVertexShader;
        bool m_bDebugPixelShader;
        bool m_bUseSoftwareVertexProcessing;
        bool m_bUsePartialPrecisionShader;
        RE_ADD_PADDING(2);
        int m_lDepthBufferPrecision;
        bool m_bBlurDropShadow;
        RE_ADD_PADDING(3);
        int m_iShaderResolution;
        ZSharedResourcesD3D::SHADOWMODE m_eShadowMode;
        D3DFORMAT m_lShadowColorFormat;
        int m_lShadowMapPassesNr;
        IDirect3DCubeTexture9* m_pEnvTextures[MAX_ENV_TEXTURES_NR]; // Count approved in PC
        IDirect3DSurface9* m_pEnvSurface[MAX_ENV_TEXTURES_NR][MAX_ENV_SURFACES_NR]; // Verified PC
        IDirect3DSurface9* m_pEnvDepth;
        bool m_bAllocated;
        RE_ADD_PADDING(3);
        int m_field00C4; // m_pBlurredSrcTexture?
        ZBlurTextureWintelD3D m_BlurTexture;
    };
    RE_VERIFY_SIZE(ZSharedResourcesD3D, 0x140); // Verified PC
    RE_VERIFY_OFFSET(ZSharedResourcesD3D, m_pResourceFX, 0x4); // Verified PC
    RE_VERIFY_OFFSET(ZSharedResourcesD3D, m_pRenderDrawShared, 0x8); // Verified PC
    RE_VERIFY_OFFSET(ZSharedResourcesD3D, m_BlurTexture, 0xC8); // Verified PC
    RE_VERIFY_OFFSET(ZSharedResourcesD3D, m_pEnvTextures, 0x4C); // Verified PC
}