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

        // methods
        ZRenderMaterialBinderTextureD3DFX(
            const char* pszName,
            ZTextureD3D* pTexture,
            ZRenderMaterialEffectD3DFX *pEffect,
            uint32_t hTexture,
            const char* pszMinFilter,
            const char* pszMagFilter,
            const char* pszMipFilter,
            const char* pszTilingU,
            const char* pszTilingV,
            const char* pszTilingW,
            D3DXHANDLE hTechnique
        );

        // members
    };
    RE_VERIFY_SIZE(ZRenderMaterialBinderTextureD3DFX, 0x54); // Verified PC allocation
}


#if 0

#endif