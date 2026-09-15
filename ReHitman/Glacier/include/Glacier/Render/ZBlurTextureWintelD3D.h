#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/Render/D3D9.h>
#include <cstdint>


namespace Glacier
{
    class ZBlurTextureWintelD3D
    {
    public:
        // constants
        static constexpr int MAX_BLUR_PASSES_NR = 8;

        // types
        enum 
        {
            eSTANDARD = 0x1,
            eDEPTHBUFFER = 0x2
        };

        struct QUAD_VERTEX
        {
            D3DXVECTOR3 p;
            D3DXVECTOR2 t;
        };

        // vtbl
        virtual ~ZBlurTextureWintelD3D();
        virtual void Init(int lWidth, int lHeight);
        virtual void AllocateDeviceBuffers();
        virtual void FreeDeviceBuffers();

        // methods
        ZBlurTextureWintelD3D();

        void StoreRenderTargets();
        void RestoreRenderTargets();
        void CopyStretch(IDirect3DTexture9* pSrc, IDirect3DTexture9* pDst, int s_w, int s_h, int d_w, int d_h, bool bScissor);
        // fBlurScale is unused in the PC/iOS binaries (literal 0.0 in the tap-offset math).
        IDirect3DTexture9* Blur(IDirect3DTexture9* pTex, float fBlurAmount, float fBlurScale, int iFlags, bool bSinglePass);

        // members
        uint32_t m_lWidth { 0 };
        uint32_t m_lHeight { 0 };
        IDirect3DSurface9* m_pPrevRTColor;
        IDirect3DSurface9* m_pPrevRTDepth;
        D3DVIEWPORT9 m_PrevViewport;
        ZRender* m_pRender { nullptr };
        uint32_t m_iBlurMaxPasses;
        float m_fBlendCol;
        IDirect3DVertexDeclaration9* m_pVDKBLUR { nullptr };
        IDirect3DVertexBuffer9* m_pQuadVB { nullptr };
        IDirect3DPixelShader9* m_pPSBlur { nullptr };
        IDirect3DVertexShader9* m_pVSBlur { nullptr };
        IDirect3DPixelShader9* m_pPixelShader2 { nullptr };
        IDirect3DVertexShader9* m_pVertexShader2 { nullptr };
        IDirect3DPixelShader9* m_pPixelShader3 { nullptr };
        uint32_t m_Unknown54; // m_pBlurSurface ?
        IDirect3DTexture9* m_pTextures[MAX_BLUR_PASSES_NR] {};
    };
    RE_VERIFY_SIZE(ZBlurTextureWintelD3D, 0x78); // Verified PC ctor (8 texture slots, last at +0x74)
    RE_VERIFY_OFFSET(ZBlurTextureWintelD3D, m_lWidth, 4); // Verfied PC ::Init
    RE_VERIFY_OFFSET(ZBlurTextureWintelD3D, m_lHeight, 8); // Verfied PC ::Init
    RE_VERIFY_OFFSET(ZBlurTextureWintelD3D, m_pRender, 0x2C); // Verified PC Dtor, init prc
    RE_VERIFY_OFFSET(ZBlurTextureWintelD3D, m_iBlurMaxPasses, 0x30); // Verified PC AllocateDeviceBuffers
    RE_VERIFY_OFFSET(ZBlurTextureWintelD3D, m_pVDKBLUR, 0x38); // Verified PC AllocateDeviceBuffers
    RE_VERIFY_OFFSET(ZBlurTextureWintelD3D, m_pQuadVB, 0x3C); // Verified PC AllocateDeviceBuffers
    RE_VERIFY_OFFSET(ZBlurTextureWintelD3D, m_pPSBlur, 0x40); // Verified PC AllocateDeviceBuffers
    RE_VERIFY_OFFSET(ZBlurTextureWintelD3D, m_pVSBlur, 0x44); // Verified PC AllocateDeviceBuffers
    RE_VERIFY_OFFSET(ZBlurTextureWintelD3D, m_pPixelShader2, 0x48); // Verified PC AllocateDeviceBuffers
    RE_VERIFY_OFFSET(ZBlurTextureWintelD3D, m_pVertexShader2, 0x4C); // Verified PC AllocateDeviceBuffers
    RE_VERIFY_OFFSET(ZBlurTextureWintelD3D, m_pPixelShader3, 0x50); // Verified PC AllocateDeviceBuffers
    RE_VERIFY_OFFSET(ZBlurTextureWintelD3D, m_pTextures, 0x58); // Verified PC AllocateDeviceBuffers/FreeDeviceBuffers (array of 8)
}