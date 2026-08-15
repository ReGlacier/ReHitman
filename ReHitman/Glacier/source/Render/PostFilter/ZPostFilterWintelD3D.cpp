#include <Glacier/Render/PostFilter/ZPostFilterWintelD3D.h>
#include <Glacier/Render/ZDirect3DDevice.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/ZGameData.h>


namespace Glacier
{
    ZPostFilterWintelD3D::ZPostFilterWintelD3D()
        : ZPostFilter()
    {
        m_pPostfilterName = nullptr;
        // TODO: Finish me
        m_pRender = g_pSysInterface->WindowFirst;
        m_bZBPassEnable = 0;
        m_bDepthEnabled = 0;
        // TODO: Finish me
    }

    ZPostFilterWintelD3D::~ZPostFilterWintelD3D()
    {
        FreeDeviceBuffers();
        g_pPostFilter = nullptr;
        m_pRender = nullptr;
    }

    void ZPostFilterWintelD3D::Update(ZRenderViewBase* pView)
    {
        // TODO: Finish me, but IDA part is broken
    }

    void ZPostFilterWintelD3D::Init()
    {
        m_bHasBumpEnv = true;

        D3DCAPS9 caps;
        g_pd3dDevice->GetDeviceCaps(&caps);

        const DWORD kRequiredBumpCaps = D3DTEXOPCAPS_BUMPENVMAP | D3DTEXOPCAPS_BUMPENVMAPLUMINANCE;
        if ((caps.TextureOpCaps & kRequiredBumpCaps) == 0)
        {
            m_bHasBumpEnv = false;
            return;
        }

        UINT lWidth = 32;
        UINT lHeight = 32;
        UINT mipLevels = 1;
        D3DFORMAT lFormat = D3DFMT_V8U8; // 60 = 0x3C

        HRESULT hr = D3DXCheckTextureRequirements(
            g_pd3dDevice->m_pDevice,
            &lWidth,
            &lHeight,
            &mipLevels,
            0,                  // Usage
            &lFormat,
            D3DPOOL_MANAGED     // Pool = 1
        );

        if (FAILED(hr) || lWidth != 32 || lHeight != 32 || lFormat != D3DFMT_V8U8)
        {
            m_bHasBumpEnv = false;
        }
    }

    void ZPostFilterWintelD3D::SetViewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
    {
        if (m_Viewport[0] != x || m_Viewport[1] != y || m_Viewport[2] != w || m_Viewport[3] != h)
        {
            m_Viewport[0] = x;
            m_Viewport[1] = y;
            m_Viewport[2] = w;
            m_Viewport[3] = h;

            FreeDeviceBuffers();
            AllocateDeviceBuffers();
        }
    }

    void ZPostFilterWintelD3D::FreeDeviceBuffers()
    {
        if (!m_pRender) return;

        // TODO: Finish me
        for (int i = 0; i < MAX_PALETTE_TEXTURES_NR; ++i)
        {
            D3D_SAFE_RELEASE(m_pPaletteTexture[i]);
        }
        // TODO: Finish me
    }

    void ZPostFilterWintelD3D::AllocateDeviceBuffers()
    {
        // TODO: Finish me
    }

    uint32_t* ZPostFilterWintelD3D::GetRedPalette(uint32_t lIndex)
    {
        // TODO: Finish me
        return nullptr;
    }

    uint32_t* ZPostFilterWintelD3D::GetGreenPalette(uint32_t lIndex)
    {
        // TODO: Finish me
        return nullptr;
    }

    uint32_t* ZPostFilterWintelD3D::GetBluePalette(uint32_t lIndex)
    {
        // TODO: Finish me
        return nullptr;
    }

    uint8_t* ZPostFilterWintelD3D::GetZBufferPalette() 
    {
        // TODO: Finish me
        return nullptr;
    }
    
    int ZPostFilterWintelD3D::UnlockRedPalette(uint32_t lIndex)
    {
        // TODO: Finish me
        return 0;
    }
    
    int ZPostFilterWintelD3D::UnlockGreenPalette(uint32_t lIndex)
    {
        // TODO: Finish me
        return 0;
    }
    
    int ZPostFilterWintelD3D::UnlockBluePalette(uint32_t lIndex)
    {
        // TODO: Finish me
        return 0;
    }
    
    int ZPostFilterWintelD3D::UnlockZBufferPalette()
    {
        // TODO: Finish me
        return 0;
    }

    void ZPostFilterWintelD3D::SetHeatShimmerSpeed(float fSpeed)
    {
        m_fHeatShimmerSpeed = fSpeed;
        UpdateZPassEnable();
    }

    void ZPostFilterWintelD3D::UpdateCurvePalettes()
    {
        const uint32_t* pZBufferPalette = reinterpret_cast<const uint32_t*>(GetZBufferPalette());
        if (!pZBufferPalette)
            return;

        uint32_t lColorMask = 0;

        for (int i = 0; i < 64; ++i)
        {
            lColorMask |= pZBufferPalette[i];
        }

        UnlockZBufferPalette();

        m_bDepthEnabled = (lColorMask != 0);
        UpdateZPassEnable();
    }

    void ZPostFilterWintelD3D::UpdateZPassEnable()
    {
        m_bZBPassEnable = g_pRenderDll->m_fPostFilterLOD == 2.0f && (m_bDepthEnabled || (m_bHasBumpEnv && m_fHeatShimmerSpeed > 0.0f));
    }
}