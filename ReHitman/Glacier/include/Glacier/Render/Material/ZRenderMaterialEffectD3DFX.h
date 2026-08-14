#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/Render/D3D9.h>
#include <cstdint>


namespace Glacier
{
    class ZRenderMaterialEffectD3DFX
    {
    public:
        // methods
        ZRenderMaterialEffectD3DFX(const char* pszFileName, const char* pszDefine, ID3DXEffect* pD3DXEffect, const D3DXMACRO* pMacros, uint32_t uNumMacros);
        ~ZRenderMaterialEffectD3DFX();
        bool IsSame(const char* pszFileName, const D3DXMACRO* pDefined, uint32_t lNumDefines) const;

        // members
        char* m_pszFileName { nullptr }; // 0x0
        char* m_pszDefine { nullptr }; // 0x04
        D3DXMACRO* m_pMacros { nullptr }; // 0x08
        uint32_t m_uNumMacros { 0 };       // 0x0C
        ID3DXEffect* m_pD3DXEffect { nullptr }; // 0x10
    };
    RE_VERIFY_SIZE(ZRenderMaterialEffectD3DFX, 0x14);
}