#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/Render/D3D9.h>
#include <Glacier/Render/ZRenderBinderTexture.h>


namespace Glacier
{
    class ZRenderMaterialBinderTextureD3DFX : public ZRenderBinderTexture
    {
    public:
        // vtbl
        ~ZRenderMaterialBinderTextureD3DFX() override;
        void Execute(const ZRenderContext* pContext) override;
        void PrintInfo() override;
        uint32_t GetTextureId() override;

        // methods
        ZRenderMaterialBinderTextureD3DFX(
            const char* pszName,
            ZTextureD3D* pTexture,
            ZRenderMaterialEffectD3DFX *pEffect,
            D3DXHANDLE hTexture,
            const char* pszMinFilter,
            const char* pszMagFilter,
            const char* pszMipFilter,
            const char* pszTilingU,
            const char* pszTilingV,
            const char* pszTilingW,
            D3DXHANDLE hTechnique
        );

        // members
        ZTextureD3D* m_pTexture;                     // +0x0C
        IDirect3DBaseTexture9* m_pD3DTexture;        // +0x10 (overrides m_pTexture->m_pUserData in Execute)
        uint32_t m_lAddressU;                        // +0x14 (D3DTADDRESS_*)
        uint32_t m_lAddressV;                        // +0x18 (D3DTADDRESS_*)
        uint32_t m_lAddressW;                        // +0x1C (D3DTADDRESS_*)
        uint32_t m_lMinFilter;                       // +0x20 (D3DTEXF_*)
        uint32_t m_lMagFilter;                       // +0x24 (D3DTEXF_*)
        uint32_t m_lMipFilter;                       // +0x28 (D3DTEXF_*)
        uint32_t m_lMaxAnisotropy;                   // +0x2C (unused; Execute passes g_lMaxAnisotropy)
        D3DXHANDLE m_hTexture;                       // +0x30
        D3DXHANDLE m_hAddressU;                      // +0x34
        D3DXHANDLE m_hAddressV;                      // +0x38
        D3DXHANDLE m_hAddressW;                      // +0x3C
        D3DXHANDLE m_hMinFilter;                     // +0x40
        D3DXHANDLE m_hMagFilter;                     // +0x44
        D3DXHANDLE m_hMipFilter;                     // +0x48
        D3DXHANDLE m_hMaxAnisotropy;                 // +0x4C
        ZRenderMaterialEffectD3DFX* m_pEffect;       // +0x50
    };
    RE_VERIFY_SIZE(ZRenderMaterialBinderTextureD3DFX, 0x54); // Verified PC allocation
    RE_VERIFY_OFFSET(ZRenderMaterialBinderTextureD3DFX, m_pTexture, 0xC); // Verified PC ctor
    RE_VERIFY_OFFSET(ZRenderMaterialBinderTextureD3DFX, m_pD3DTexture, 0x10); // Verified PC ctor/Execute
    RE_VERIFY_OFFSET(ZRenderMaterialBinderTextureD3DFX, m_lAddressU, 0x14); // Verified PC ctor
    RE_VERIFY_OFFSET(ZRenderMaterialBinderTextureD3DFX, m_lMinFilter, 0x20); // Verified PC ctor
    RE_VERIFY_OFFSET(ZRenderMaterialBinderTextureD3DFX, m_lMaxAnisotropy, 0x2C); // Verified PC ctor
    RE_VERIFY_OFFSET(ZRenderMaterialBinderTextureD3DFX, m_hTexture, 0x30); // Verified PC ctor
    RE_VERIFY_OFFSET(ZRenderMaterialBinderTextureD3DFX, m_hAddressU, 0x34); // Verified PC ctor
    RE_VERIFY_OFFSET(ZRenderMaterialBinderTextureD3DFX, m_hMaxAnisotropy, 0x4C); // Verified PC ctor
    RE_VERIFY_OFFSET(ZRenderMaterialBinderTextureD3DFX, m_pEffect, 0x50); // Verified PC ctor
}
