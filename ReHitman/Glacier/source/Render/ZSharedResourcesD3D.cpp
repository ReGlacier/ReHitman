#include <Glacier/Render/Material/ZRenderMaterialResourceD3DFX.h>
#include <Glacier/Render/PostFilter/ZPostFilter.h>
#include <Glacier/Render/ZSharedResourcesD3D.h>
#include <Glacier/Render/ZDirect3DDevice.h>
#include <Glacier/Render/ZRD3DStaticVB.h>
#include <Glacier/Render/ZRD3DStaticIB.h>
#include <Glacier/Render/ZRD3DDynamicVB.h>
#include <Glacier/Render/ZRD3DDynamicIB.h>
#include <Glacier/Render/ZRD3DDynamicIB32.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/Render/D3D9.h>
#include <Glacier/System/ZRX86AllocIf.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    namespace
    {
        void AllocateBigQuadVB()
        {
            constexpr DWORD kUsage = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;
            static_assert(kUsage == 0x208u);

            g_pd3dDevice->CreateVertexBuffer(0x70u, kUsage, 0, D3DPOOL_DEFAULT, &g_pVBPDT1, nullptr);
            g_pd3dDevice->CreateVertexBuffer(0x90u, kUsage, 0, D3DPOOL_DEFAULT, &g_pVBPDT2, nullptr);
            g_pd3dDevice->CreateVertexBuffer(0xB0u, kUsage, 0, D3DPOOL_DEFAULT, &g_pVBPDT3, nullptr);
            g_pd3dDevice->CreateVertexBuffer(0xD0u, kUsage, 0, D3DPOOL_DEFAULT, &g_pVBPDT4, nullptr);
        }
        
        void FreeBigQuadVB()
        {
            if (g_pVBPDT1 && !g_pVBPDT1->Release())
            {
                g_pVBPDT1 = nullptr;
            }

            if (g_pVBPDT2 && !g_pVBPDT2->Release())
            {
                g_pVBPDT2 = nullptr;
            }
            
            if (g_pVBPDT3 && !g_pVBPDT3->Release())
            {
                g_pVBPDT3 = nullptr;
            }

            if (g_pVBPDT4 && !g_pVBPDT4->Release())
            {
                g_pVBPDT4 = nullptr;
            }
        }
    }

    void ZSharedResourcesD3D::Create()
    {
        ZASSERT(!ZSharedResourcesD3D::g_pInstance);
        
        ZSharedResourcesD3D::g_pInstance = ZUniMemory::New<ZSharedResourcesD3D>();
    }

    void ZSharedResourcesD3D::Release()
    {
        ZASSERT(ZSharedResourcesD3D::g_pInstance);
        ZSharedResourcesD3D::g_pInstance->Free();

        ZUniMemory::Delete(ZSharedResourcesD3D::g_pInstance);
        ZSharedResourcesD3D::g_pInstance = nullptr;
    }

    ZSharedResourcesD3D::ZSharedResourcesD3D()
    {
        // TODO: Finish me
    }

    ZSharedResourcesD3D::~ZSharedResourcesD3D()
    {
        // TODO: Finish me
    }

    void ZSharedResourcesD3D::Free()
    {
        if (m_bAllocated)
        {
            m_bAllocated = false;

            if (m_pVertexAllocator)
            {
                ZUniMemory::Delete(m_pVertexAllocator);
                m_pVertexAllocator = nullptr;
            }

            if (m_pIndexAllocator)
            {
                ZUniMemory::Delete(m_pIndexAllocator);
                m_pIndexAllocator = nullptr;
            }

            if (m_pSVB)
            {
                ZUniMemory::Delete(m_pSVB);
                m_pSVB = nullptr;
            }

            if (m_pSIB)
            {
                ZUniMemory::Delete(m_pSIB);
                m_pSIB = nullptr;
            }

            if (m_pDVB)
            {
                ZUniMemory::Delete(m_pDVB);
                m_pDVB = nullptr;
            }

            if (m_pDIB)
            {
                ZUniMemory::Delete(m_pDIB);
                m_pDIB = nullptr;
            }

            if (m_pDIB32)
            {
                ZUniMemory::Delete(m_pDIB32);
                m_pDIB32 = nullptr;
            }

            m_pResourceFX->FreeResources();

            if (m_pVDNull && !m_pVDNull->Release())
            {
                m_pVDNull = nullptr;
            }

            // TODO: Finish me
            // dword_90AE3C - not initialized - skipped until usage found
            // dword_90AECC - not initialized - skipped until usage found
            // dword_90AE84 - not initialized - skipped until usage found
            // dword_90ADF4 - used at sub_491390 but type unrecognized - skipped
            // 

            if (g_pNormalizedCubeMap)
            {
                g_pNormalizedCubeMap->Release();
                g_pNormalizedCubeMap = nullptr;
            }
            
            if (g_pAnisotropicMap)
            {
                g_pAnisotropicMap->Release();
                g_pAnisotropicMap = nullptr;
            }
            
            if (g_pSpecularMap)
            {
                g_pSpecularMap->Release();
                g_pSpecularMap = nullptr;
            }

            if (g_pScatterMap)
            {
                g_pScatterMap->Release();
                g_pScatterMap = nullptr;
            }

            if (g_pghPhaseMap)
            {
                g_pghPhaseMap->Release();
                g_pghPhaseMap = nullptr;
            }

            if (g_pRefractionMap)
            {
                g_pRefractionMap->Release();
                g_pRefractionMap = nullptr;
            }

            for (int i = 0; i < 2; ++i)
            {
                auto* pTex0 = reinterpret_cast<IDirect3DTexture9*>(g_pSpotAttenuationMap[i].m_pUserData);
                auto* pTex1 = reinterpret_cast<IDirect3DTexture9*>(g_pSpotAttenuationMap2[i].m_pUserData);

                pTex0->Release();
                pTex1->Release();

                g_pSpotAttenuationMap[i].m_pUserData = nullptr;
                g_pSpotAttenuationMap2[i].m_pUserData = nullptr;
            }

            for (int lPassId = 0; lPassId < m_lShadowMapPassesNr; ++lPassId)
            {
                if (!g_texShadowMapColor[lPassId].m_pUserData)
                    continue;
                
                reinterpret_cast<IDirect3DTexture9*>(g_texShadowMapColor[lPassId].m_pUserData)->Release();
                g_texShadowMapColor[lPassId].m_pUserData = nullptr;
            }

            if (g_pShadowMapDepthBuffers)
            {
                g_pShadowMapDepthBuffers->Release();
                g_pShadowMapDepthBuffers = nullptr;
            }

            for (int i = 0; i < MAX_ENV_TEXTURES_NR; ++i)
            {
                for (int j = 0; j < MAX_ENV_SURFACES_NR; ++j)
                {
                    m_pEnvSurface[i][j]->Release();
                }

                m_pEnvTextures[i]->Release();
            }

            if (m_pEnvDepth)
            {
                m_pEnvDepth->Release();
                m_pEnvDepth = nullptr;
            }
            
            FreeBigQuadVB();

            if (g_pPostFilter)
            {
                g_pPostFilter->FreeDeviceBuffers();
            }

            if (m_bBlurDropShadow)
            {
                m_BlurTexture.FreeDeviceBuffers();
            }
        }
    }

    void ZSharedResourcesD3D::Allocate()
    {
        if (!m_bAllocated)
        {
            m_bAllocated = true;
            
            // TODO: Finish me
        }
    }

    void ZSharedResourcesD3D::BlurTexture(IDirect3DTexture9* pTexture, float fBlur1, float fBlur2, int lFlags, bool)
    {
        // TODO: Finish me
    }

    void ZSharedResourcesD3D::LockBuffersForCopy()
    {
        // iOS locks the current and next copies of its triple-buffered
        // static VB/IB sets; the PC layout has a single SVB/SIB pair.
        m_pSVB->LockedData();
        m_pSIB->LockedData();
    }

    STATIC_CLASS_VAR_IMPL(ZSharedResourcesD3D, ZSharedResourcesD3D*, g_pInstance, 0x0090AF20, nullptr);
    STATIC_CLASS_VAR_IMPL(ZSharedResourcesD3D, bool, g_bSVBLockAcquired, 0x0090AEF1, false);
}