#include <Glacier/Render/PostFilter/ZPostFilterWintelD3D.h>
#include <Glacier/Render/ZDirect3DDevice.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/ZRender.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/Data/ZGameData.h>
#include <algorithm>
#include <utility>


namespace Glacier
{
    ZPostFilterWintelD3D::ZPostFilterWintelD3D()
        : ZPostFilter()
    {
        m_pPostfilterName = nullptr;
        m_lUnknownB8 = 0;
        m_lUnknownBC = 0;
        m_lUnknownC0 = 0;
        m_lUnknownC4 = 0;
        m_lUnknownC8 = 0;
        m_lUnknownCC = 0;
        m_lUnknownD0 = 0;
        m_lUnknownD4 = 0;
        m_lUnknownD8 = 0;
        m_lUnknownDC = 0;
        m_lUnknownE0 = nullptr;
        m_lUnknownE4 = nullptr;
        m_lUnknownE8 = 0;
        m_lUnknownEC = 0;
        m_lUnknownF0 = nullptr;
        m_lUnknownF4 = nullptr;
        m_lUnknownF8 = nullptr;
        m_lUnknownFC = nullptr;
        m_lUnknown100 = nullptr;
        m_lUnknown104 = nullptr;
        m_lUnknown108 = 0;
        m_lUnknown10C = nullptr;
        for (auto& pTexture : m_pPaletteTexture)
            pTexture = nullptr;
        m_lUnknown140 = nullptr;
        m_lUnknown144 = 0;
        m_lUnknown148 = nullptr;
        m_lUnknown14C = nullptr;
        m_lUnknown150 = nullptr;
        m_lUnknown154 = nullptr;
        m_pSurface = nullptr;
        m_pRender = g_pSysInterface->WindowFirst;
        m_lUnknown160 = 0;
        m_lUnknown164 = 0;
        m_lUnknown168 = 0;
        m_lUnknown16C = nullptr;
        m_lUnknown170 = nullptr;
        m_lUnknown174 = nullptr;
        m_lUnknown178 = nullptr;
        m_lUnknown17C = nullptr;
        m_lUnknown180 = nullptr;
        m_lUnknown184 = nullptr;
        m_lUnknown188 = nullptr;
        m_lUnknown18C = nullptr;
        m_lUnknown190 = nullptr;
        m_pVertexDeclaration = nullptr;
        m_bZBPassEnable = 0;
        m_bDepthEnabled = 0;
        m_lUnknown1A0 = 0;
        m_bHasBumpEnv = 0;
        m_fUnknown1A8 = 0.0f;
        m_lUnknown1AC = 0;
        m_Viewport[0] = 0;
        m_Viewport[1] = 0;
        m_Viewport[2] = 0;
        m_Viewport[3] = 0;
        m_fUnknown1C0 = 0.0f;
        m_fUnknown1C4 = 0.0f;
    }

    ZPostFilterWintelD3D::~ZPostFilterWintelD3D()
    {
        FreeDeviceBuffers();
        g_pPostFilter = nullptr;
        m_pRender = nullptr;
    }

    void ZPostFilterWintelD3D::Update(ZRenderViewBase* pView)
    {
        (void)pView;

        // PC 0x004AC3D0 is an eight-pass orchestrator. The view argument is not read;
        // case 0x12 has already copied its view matrix into the base object.
        if (g_bDisablePostEffects || !g_pd3dDevice || !m_pRender || !m_lUnknownE0
            || g_pRenderDll->m_fPostFilterLOD == 0.0f)
            return;

        IDirect3DSurface9* pOriginalTarget = nullptr;
        IDirect3DSurface9* pOriginalDepth = nullptr;
        if (FAILED(g_pd3dDevice->GetRenderTarget(0, &pOriginalTarget)) || !pOriginalTarget)
            return;
        g_pd3dDevice->GetDepthStencilSurface(&pOriginalDepth);

        D3DSURFACE_DESC targetDesc{};
        pOriginalTarget->GetDesc(&targetDesc);
        const UINT lWidth = m_Viewport[2];
        const UINT lHeight = m_Viewport[3];

        IDirect3DTexture9* pCurrent = m_lUnknownE0;
        IDirect3DTexture9* pNext = m_lUnknownE4;
        IDirect3DSurface9* pCurrentSurface = nullptr;
        pCurrent->GetSurfaceLevel(0, &pCurrentSurface);
        if (!pCurrentSurface)
        {
            D3D_SAFE_RELEASE(pOriginalDepth);
            D3D_SAFE_RELEASE(pOriginalTarget);
            return;
        }

        RECT sourceRect{ 0, 0, static_cast<LONG>((std::min)(lWidth, targetDesc.Width)),
            static_cast<LONG>((std::min)(lHeight, targetDesc.Height)) };
        g_pd3dDevice->StretchRect(pOriginalTarget, &sourceRect, pCurrentSurface, &sourceRect, D3DTEXF_NONE);
        D3D_SAFE_RELEASE(pCurrentSurface);

        g_pd3dDevice->SetDepthStencilSurface(nullptr);
        g_pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 15);
        g_pd3dDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        g_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

        const auto runPass = [&](IDirect3DPixelShader9* pShader,
                                 IDirect3DBaseTexture9* pTexture1 = nullptr,
                                 IDirect3DBaseTexture9* pTexture2 = nullptr,
                                 IDirect3DBaseTexture9* pTexture3 = nullptr)
        {
            if (!pNext)
                return;

            IDirect3DSurface9* pNextSurface = nullptr;
            pNext->GetSurfaceLevel(0, &pNextSurface);
            if (!pNextSurface)
                return;

            g_pd3dDevice->SetRenderTarget(0, pNextSurface);
            D3D_SAFE_RELEASE(pNextSurface);
            g_pd3dDevice->SetTexture(0, pCurrent);
            g_pd3dDevice->SetTexture(1, pTexture1);
            g_pd3dDevice->SetTexture(2, pTexture2);
            g_pd3dDevice->SetTexture(3, pTexture3);
            for (DWORD i = 0; i < 4; ++i)
            {
                g_pd3dDevice->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
                g_pd3dDevice->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
                g_pd3dDevice->SetSamplerState(i, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
                g_pd3dDevice->SetSamplerState(i, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
            }
            g_pd3dDevice->SetPixelShader(pShader);
            ZDirect3DDevice::DrawPlane(g_pd3dDevice, 0.0f, 0.0f,
                static_cast<float>(lWidth), static_cast<float>(lHeight), 0xFFFFFFFF, 0.0f, 1.0f, 1.0f);
            std::swap(pCurrent, pNext);
        };

        // PC call order at 0x004AC3DE..0x004AC40D: bloom, color/depth, heat,
        // finish, feedback, color offset, noise, then state restore/copy.
        if (m_fBloomAmount != 0.0f)
            runPass(m_lUnknownF0);

        runPass(m_lUnknown178, m_pPaletteTexture[0], m_pPaletteTexture[1], m_pPaletteTexture[2]);
        if (g_pRenderDll->m_fPostFilterLOD == 2.0f && m_fDepthBlur != 0.0f)
            runPass(m_lUnknown174, m_lUnknown140);
        if (g_pRenderDll->m_fPostFilterLOD == 2.0f && m_bHasBumpEnv && m_fHeatShimmerSpeed != 0.0f)
            runPass(m_lUnknown17C, m_lUnknownF8, m_lUnknown140);
        if (m_fFinishBlendAmount != 0.0f)
            runPass(m_lUnknown188, m_lUnknown148);
        if (m_fLastBlendAmountFrameBuffer != 0.0f)
            runPass(m_lUnknown184, m_lUnknown154);
        if (m_fOffset[0][0] != 0.0f || m_fOffset[0][1] != 0.0f
            || m_fOffset[1][0] != 0.0f || m_fOffset[1][1] != 0.0f
            || m_fOffset[2][0] != 0.0f || m_fOffset[2][1] != 0.0f)
            runPass(m_lUnknown180, pCurrent, pCurrent);
        if (m_fNoiseMax != 0.0f)
            runPass(m_lUnknown190, m_lUnknown150);

        IDirect3DSurface9* pFinalSurface = nullptr;
        pCurrent->GetSurfaceLevel(0, &pFinalSurface);
        if (pFinalSurface)
        {
            g_pd3dDevice->StretchRect(pFinalSurface, &sourceRect, pOriginalTarget, &sourceRect, D3DTEXF_NONE);
            D3D_SAFE_RELEASE(pFinalSurface);
        }

        g_pd3dDevice->SetRenderTarget(0, pOriginalTarget);
        g_pd3dDevice->SetDepthStencilSurface(pOriginalDepth);
        m_pRender->SetViewport(static_cast<float>(m_Viewport[0]), static_cast<float>(m_Viewport[1]),
            static_cast<float>(m_Viewport[2]), static_cast<float>(m_Viewport[3]));
        g_pd3dDevice->SetPixelShader(nullptr);
        g_pd3dDevice->SetVertexShader(nullptr);
        g_pd3dDevice->SetIndices(nullptr);
        g_pd3dDevice->SetStreamSource(0, nullptr, 0, 0);
        for (DWORD i = 0; i < 4; ++i)
            g_pd3dDevice->SetTexture(i, nullptr);
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
        g_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
        g_pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 7);
        g_pd3dDevice->SynchronizeStateCaches();

        D3D_SAFE_RELEASE(pOriginalDepth);
        D3D_SAFE_RELEASE(pOriginalTarget);
    }

    void ZPostFilterWintelD3D::Init()
    {
        m_bHasBumpEnv = true;

        if (!g_pd3dDevice)
        {
            m_bHasBumpEnv = false;
            return;
        }

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

        for (int i = 0; i < MAX_PALETTE_TEXTURES_NR; ++i)
        {
            D3D_SAFE_RELEASE(m_pPaletteTexture[i]);
        }
        D3D_SAFE_RELEASE(m_lUnknownE0);
        D3D_SAFE_RELEASE(m_lUnknownE4);
        D3D_SAFE_RELEASE(m_lUnknownF0);
        D3D_SAFE_RELEASE(m_lUnknownF4);
        D3D_SAFE_RELEASE(m_lUnknownF8);
        D3D_SAFE_RELEASE(m_lUnknownFC);
        D3D_SAFE_RELEASE(m_lUnknown100);
        D3D_SAFE_RELEASE(m_lUnknown104);
        D3D_SAFE_RELEASE(m_lUnknown10C);
        D3D_SAFE_RELEASE(m_lUnknown140);
        D3D_SAFE_RELEASE(m_lUnknown148);
        D3D_SAFE_RELEASE(m_lUnknown14C);
        D3D_SAFE_RELEASE(m_lUnknown150);
        D3D_SAFE_RELEASE(m_lUnknown154);
        D3D_SAFE_RELEASE(m_pSurface);
        D3D_SAFE_RELEASE(m_lUnknown16C);
        D3D_SAFE_RELEASE(m_lUnknown170);
        D3D_SAFE_RELEASE(m_lUnknown174);
        D3D_SAFE_RELEASE(m_lUnknown178);
        D3D_SAFE_RELEASE(m_lUnknown17C);
        D3D_SAFE_RELEASE(m_lUnknown180);
        D3D_SAFE_RELEASE(m_lUnknown184);
        D3D_SAFE_RELEASE(m_lUnknown188);
        D3D_SAFE_RELEASE(m_lUnknown18C);
        D3D_SAFE_RELEASE(m_lUnknown190);
        D3D_SAFE_RELEASE(m_pVertexDeclaration);
    }

    void ZPostFilterWintelD3D::AllocateDeviceBuffers()
    {
        if (!m_pRender || !g_pd3dDevice)
            return;

        FreeDeviceBuffers();

        const UINT lWidth = m_Viewport[2];
        const UINT lHeight = m_Viewport[3];
        if (!lWidth || !lHeight)
            return;

        g_pd3dDevice->CreateTexture(lWidth, lHeight, 1, D3DUSAGE_RENDERTARGET,
            D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_lUnknownE0, nullptr);
        g_pd3dDevice->CreateTexture(lWidth, lHeight, 1, D3DUSAGE_RENDERTARGET,
            D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_lUnknownE4, nullptr);
        g_pd3dDevice->CreateTexture(lWidth, lHeight, 1, D3DUSAGE_RENDERTARGET,
            D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_lUnknown154, nullptr);
        g_pd3dDevice->CreateTexture((std::max)(1u, lWidth / 2), (std::max)(1u, lHeight / 2), 1,
            D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_lUnknownFC, nullptr);
        g_pd3dDevice->CreateTexture((std::max)(1u, lWidth / 2), (std::max)(1u, lHeight / 2), 1,
            D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_lUnknown100, nullptr);
        g_pd3dDevice->CreateTexture((std::max)(1u, lWidth / 2), (std::max)(1u, lHeight / 2), 1,
            D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_lUnknown104, nullptr);
        g_pd3dDevice->CreateTexture((std::max)(1u, lWidth / 4), (std::max)(1u, lHeight / 4), 1,
            D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_lUnknown14C, nullptr);
        if (m_lUnknown154)
            m_lUnknown154->GetSurfaceLevel(0, &m_pSurface);

        // PC palettes are 256 entries; red/blue are vertical and green is horizontal.
        for (int i = 0; i < MAX_PALETTE_TEXTURES_NR; ++i)
        {
            const UINT width = i % 3 == 1 ? 256u : 1u;
            const UINT height = i % 3 == 1 ? 1u : 256u;
            g_pd3dDevice->CreateTexture(width, height, 1, 0, D3DFMT_A8R8G8B8,
                D3DPOOL_MANAGED, &m_pPaletteTexture[i], nullptr);
        }

        g_pd3dDevice->CreateTexture(256, 1, 1, 0, D3DFMT_A8R8G8B8,
            D3DPOOL_MANAGED, &m_lUnknown140, nullptr);
        g_pd3dDevice->CreateTexture(256, 1, 1, 0, D3DFMT_A8R8G8B8,
            D3DPOOL_MANAGED, &m_lUnknown148, nullptr);
        g_pd3dDevice->CreateTexture(64, 64, 1, 0, D3DFMT_A8R8G8B8,
            D3DPOOL_MANAGED, &m_lUnknown150, nullptr);
        if (m_bHasBumpEnv)
            g_pd3dDevice->CreateTexture(32, 32, 1, 0, D3DFMT_V8U8,
                D3DPOOL_MANAGED, &m_lUnknownF8, nullptr);
    }

    uint32_t* ZPostFilterWintelD3D::GetRedPalette(uint32_t lIndex)
    {
        if (lIndex >= MAX_PALETTE_TEXTURES_NR / 3 || !m_pPaletteTexture[lIndex * 3])
            return nullptr;
        D3DLOCKED_RECT lockedRect;
        return SUCCEEDED(m_pPaletteTexture[lIndex * 3]->LockRect(0, &lockedRect, nullptr, 0))
            ? static_cast<uint32_t*>(lockedRect.pBits) : nullptr;
    }

    uint32_t* ZPostFilterWintelD3D::GetGreenPalette(uint32_t lIndex)
    {
        if (lIndex >= MAX_PALETTE_TEXTURES_NR / 3 || !m_pPaletteTexture[lIndex * 3 + 1])
            return nullptr;
        D3DLOCKED_RECT lockedRect;
        return SUCCEEDED(m_pPaletteTexture[lIndex * 3 + 1]->LockRect(0, &lockedRect, nullptr, 0))
            ? static_cast<uint32_t*>(lockedRect.pBits) : nullptr;
    }

    uint32_t* ZPostFilterWintelD3D::GetBluePalette(uint32_t lIndex)
    {
        if (lIndex >= MAX_PALETTE_TEXTURES_NR / 3 || !m_pPaletteTexture[lIndex * 3 + 2])
            return nullptr;
        D3DLOCKED_RECT lockedRect;
        return SUCCEEDED(m_pPaletteTexture[lIndex * 3 + 2]->LockRect(0, &lockedRect, nullptr, 0))
            ? static_cast<uint32_t*>(lockedRect.pBits) : nullptr;
    }

    uint8_t* ZPostFilterWintelD3D::GetZBufferPalette()
    {
        if (!m_lUnknown140)
            return nullptr;
        D3DLOCKED_RECT lockedRect;
        return SUCCEEDED(m_lUnknown140->LockRect(0, &lockedRect, nullptr, 0))
            ? static_cast<uint8_t*>(lockedRect.pBits) : nullptr;
    }

    int ZPostFilterWintelD3D::UnlockRedPalette(uint32_t lIndex)
    {
        if (lIndex >= MAX_PALETTE_TEXTURES_NR / 3 || !m_pPaletteTexture[lIndex * 3])
            return 0;
        return SUCCEEDED(m_pPaletteTexture[lIndex * 3]->UnlockRect(0)) ? 1 : 0;
    }

    int ZPostFilterWintelD3D::UnlockGreenPalette(uint32_t lIndex)
    {
        if (lIndex >= MAX_PALETTE_TEXTURES_NR / 3 || !m_pPaletteTexture[lIndex * 3 + 1])
            return 0;
        return SUCCEEDED(m_pPaletteTexture[lIndex * 3 + 1]->UnlockRect(0)) ? 1 : 0;
    }

    int ZPostFilterWintelD3D::UnlockBluePalette(uint32_t lIndex)
    {
        if (lIndex >= MAX_PALETTE_TEXTURES_NR / 3 || !m_pPaletteTexture[lIndex * 3 + 2])
            return 0;
        return SUCCEEDED(m_pPaletteTexture[lIndex * 3 + 2]->UnlockRect(0)) ? 1 : 0;
    }

    int ZPostFilterWintelD3D::UnlockZBufferPalette()
    {
        if (!m_lUnknown140)
            return 0;
        return SUCCEEDED(m_lUnknown140->UnlockRect(0)) ? 1 : 0;
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
