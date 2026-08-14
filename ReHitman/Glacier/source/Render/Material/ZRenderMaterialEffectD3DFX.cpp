#include <Glacier/Render/Material/ZRenderMaterialEffectD3DFX.h>
#include <Glacier/ZUniMemory.h>
#include <cstring>


namespace Glacier
{
    // ZRenderMaterialEffectD3DFX

    ZRenderMaterialEffectD3DFX::ZRenderMaterialEffectD3DFX(const char* pszFileName, const char* pszDefine, ID3DXEffect* pD3DXEffect, const D3DXMACRO* pMacros, uint32_t uNumMacros)
    {
        m_uNumMacros = uNumMacros;
        m_pD3DXEffect = pD3DXEffect;

        if (pszFileName)
        {
            size_t len = strlen(pszFileName);
            m_pszFileName = (char*)ZUniMemory::Allocate(len+1);
            strcpy(m_pszFileName, pszFileName);
        }
        else
        {
            m_pszFileName = nullptr;
        }

        if (pszDefine)
        {
            size_t len = strlen(pszDefine);
            m_pszDefine = (char*)ZUniMemory::Allocate(len+1);
            strcpy(m_pszDefine, pszDefine);
        }
        else
        {
            m_pszDefine = nullptr;
        }

        if (m_uNumMacros > 0)
        {
            m_pMacros = (D3DXMACRO*)ZUniMemory::Allocate(sizeof(m_pMacros) * m_uNumMacros);
            for (uint32_t i = 0; i < m_uNumMacros; ++i)
            {
                new (&m_pMacros[i]) D3DXMACRO();

                if (pMacros[i].Name)
                {
                    size_t len = strlen(pMacros[i].Name);
                    char* pszName = (char*)ZUniMemory::Allocate(len + 1);
                    strcpy(pszName, pMacros[i].Name);
                    m_pMacros[i].Name = pszName;
                }
                else
                {
                    m_pMacros[i].Name = nullptr;
                }

                if (pMacros[i].Definition)
                {
                    size_t len = strlen(pMacros[i].Definition);
                    char* pszDef = (char*)ZUniMemory::Allocate(len + 1);
                    strcpy(pszDef, pMacros[i].Definition);
                    m_pMacros[i].Definition = pszDef;
                }
                else
                {
                    m_pMacros[i].Definition = nullptr;
                }
            }
        }
        else
        {
            m_pMacros = nullptr;
        }
    }

    ZRenderMaterialEffectD3DFX::~ZRenderMaterialEffectD3DFX()
    {
        if (m_pD3DXEffect)
        {
            m_pD3DXEffect->Release();
            m_pD3DXEffect = nullptr;
        }

        if (m_pszFileName)
        {
            ZUniMemory::Free(m_pszFileName);
            m_pszFileName = nullptr;
        }

        if (m_pszDefine)
        {
            ZUniMemory::Free(m_pszDefine);
            m_pszDefine = nullptr;
        }

        if (m_uNumMacros > 0 && m_pMacros)
        {
            for (uint32_t i = 0; i < m_uNumMacros; ++i)
            {
                if (m_pMacros[i].Name)
                {
                    ZUniMemory::Free((void*)m_pMacros[i].Name);
                }

                if (m_pMacros[i].Definition)
                {
                    ZUniMemory::Free((void*)m_pMacros[i].Definition);
                }

                (&m_pMacros[i])->~D3DXMACRO();
            }

            ZUniMemory::Free(m_pMacros);
            m_pMacros = nullptr;
        }
    }
}