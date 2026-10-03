#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/PostFilter/ZPostFilter.h>
#include <Glacier/Render/D3D9.h>


namespace Glacier
{
    class ZPostFilterWintelD3D : public ZPostFilter
    {
    public:
        // constants
        static constexpr int MAX_PALETTE_TEXTURES_NR = 12;

        // vtbl
        ~ZPostFilterWintelD3D() override;
        void Update(ZRenderViewBase* pView) override;
        void Init() override;
        void SetViewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h) override;
        void FreeDeviceBuffers() override;
        void AllocateDeviceBuffers() override;
        uint32_t* GetRedPalette(uint32_t lIndex) override;
        uint32_t* GetGreenPalette(uint32_t lIndex) override;
        uint32_t* GetBluePalette(uint32_t lIndex) override;
        uint8_t* GetZBufferPalette() override;
        int UnlockRedPalette(uint32_t lIndex) override;
        int UnlockGreenPalette(uint32_t lIndex) override;
        int UnlockBluePalette(uint32_t lIndex) override;
        int UnlockZBufferPalette() override;
        void SetHeatShimmerSpeed(float fSpeed) override;
        void UpdateCurvePalettes() override;
        virtual void UpdateZPassEnable();

        // methods
        ZPostFilterWintelD3D();

        // members
        uint32_t m_lUnknownB8;
        uint32_t m_lUnknownBC;
        uint32_t m_lUnknownC0;
        uint32_t m_lUnknownC4;
        uint32_t m_lUnknownC8;
        uint32_t m_lUnknownCC;
        uint32_t m_lUnknownD0;
        uint32_t m_lUnknownD4;
        uint32_t m_lUnknownD8;
        uint32_t m_lUnknownDC;
        IDirect3DTexture9* m_lUnknownE0;
        IDirect3DTexture9* m_lUnknownE4;
        uint32_t m_lUnknownE8;
        uint32_t m_lUnknownEC;
        IDirect3DPixelShader9* m_lUnknownF0;
        IDirect3DPixelShader9* m_lUnknownF4;
        IDirect3DTexture9* m_lUnknownF8;
        IDirect3DTexture9* m_lUnknownFC;
        IDirect3DTexture9* m_lUnknown100;
        IDirect3DTexture9* m_lUnknown104;
        uint32_t m_lUnknown108;
        IDirect3DTexture9* m_lUnknown10C;
        IDirect3DTexture9* m_pPaletteTexture[MAX_PALETTE_TEXTURES_NR];
        IDirect3DTexture9* m_lUnknown140;
        uint32_t m_lUnknown144;
        IDirect3DTexture9* m_lUnknown148;
        IDirect3DTexture9* m_lUnknown14C;
        IDirect3DTexture9* m_lUnknown150;
        IDirect3DTexture9* m_lUnknown154;
        IDirect3DSurface9* m_pSurface;
        ZRender* m_pRender;
        uint32_t m_lUnknown160;
        uint32_t m_lUnknown164;
        uint32_t m_lUnknown168;
        IDirect3DVertexShader9* m_lUnknown16C;
        IDirect3DPixelShader9* m_lUnknown170;
        IDirect3DPixelShader9* m_lUnknown174;
        IDirect3DPixelShader9* m_lUnknown178;
        IDirect3DPixelShader9* m_lUnknown17C;
        IDirect3DPixelShader9* m_lUnknown180;
        IDirect3DPixelShader9* m_lUnknown184;
        IDirect3DPixelShader9* m_lUnknown188;
        IDirect3DPixelShader9* m_lUnknown18C;
        IDirect3DPixelShader9* m_lUnknown190;
        IDirect3DVertexDeclaration9* m_pVertexDeclaration;
        uint32_t m_bDepthEnabled;
        uint32_t m_bZBPassEnable;
        uint32_t m_lUnknown1A0;
        uint32_t m_bHasBumpEnv;
        float m_fUnknown1A8;
        uint32_t m_lUnknown1AC;
        uint32_t m_Viewport[4];
        float m_fUnknown1C0;
        float m_fUnknown1C4;
    };
    RE_VERIFY_SIZE(ZPostFilterWintelD3D, 0x1C8); // Verified PC ZRenderViewD3D::CreatePostFilter

    RE_VERIFY_OFFSET(ZPostFilterWintelD3D, m_pRender, 0x15C); // Verified by dtor
    RE_VERIFY_OFFSET(ZPostFilterWintelD3D, m_bDepthEnabled, 0x198); // Verified by ZPostFilterWintelD3D::UpdateZPassEnable
    RE_VERIFY_OFFSET(ZPostFilterWintelD3D, m_bZBPassEnable, 0x19C); // Verified by ZPostFilterWintelD3D::UpdateZPassEnable
    RE_VERIFY_OFFSET(ZPostFilterWintelD3D, m_bHasBumpEnv, 0x1A4); // Verified by ZPostFilterWintelD3D::UpdateZPassEnable
}