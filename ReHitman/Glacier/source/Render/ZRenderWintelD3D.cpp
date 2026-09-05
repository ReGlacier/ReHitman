#include <Glacier/Render/ZRenderWintelD3D.h>
#include <Glacier/Render/Draw/ZRenderDrawBase.h>
#include <Glacier/Render/ZDirect3DDevice.h>
#include <Glacier/Render/ZSharedResourcesD3D.h>
#include <Glacier/Render/ZRD3DStaticVB.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/ZTextureManagerD3D.h>
#include <Glacier/Render/Material/ZRenderMaterialBuffer.h>
#include <Glacier/ZSTL/ZOffsetAlloc.h>
#include <Glacier/ZSTL/REFTAB.h>
#define XMD_H   // avoid INT16/INT32 typedefs from jmorecfg.h conflicting with other headers
#include <jpeglib.h>
#undef XMD_H
#include <Glacier/ZSTL/STLport.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/System/ZRX86AllocIf.h>
#include <Glacier/Input/SysInput.h>
#include <Glacier/Input/ZSysInputWintel.h>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <intrin.h>


namespace Glacier
{
    // Minimal stand-in for the engine's ZExceptionRenderD3D (thrown from Init), derived from
    // the Wintel exception (module + description).
    class ZExceptionRenderD3D : public stlp::exception
    {
    public:
        ZExceptionRenderD3D(const char* pszModule, const char* pszDescription)
            : stlp::exception(pszDescription), m_pszModule(pszModule), m_pszDescription(pszDescription)
        {
        }

    private:
        const char* m_pszModule;
        const char* m_pszDescription;
    };

    bool ZRenderWintelD3D::m_bInScene = false;

    // PC 0x00488300. D3D-specific construction on top of ZRenderWintel: zeroes the offscreen /
    // swap-chain state, creates the shared resources singleton and binds its render-draw object.
    ZRenderWintelD3D::ZRenderWintelD3D(int hInstance, void* hParentWnd)
        : ZRenderWintel(hInstance, hParentWnd)
    {
        m_field1524 = 0;
        m_field1525 = 0;
        field_1526 = 0;
        m_pOffscreenSurface = 0;
        m_iOffscreenSurfaceWidth = 0;
        m_iOffscreenSurfaceHeight = 0;
        m_pSwapChain = nullptr;
        m_pSurface16A4 = nullptr;
        m_pSurface16A8 = nullptr;
        field_16AC = 0;
        field_16AE = 0;
        field_16AF = 1;
        m_bDisableMMX = 0;
        m_field16B4 = 0;
        m_pTexture16B8 = nullptr;
        m_pSurface16BC = nullptr;
        m_pAntialiasSurface = nullptr;
        m_pBinkVideoTextureD3D = nullptr;
        m_field1710 = 0;

        if (!ZSharedResourcesD3D::g_pInstance)
            ZSharedResourcesD3D::Create();

        m_pRenderDraw = reinterpret_cast<ZRenderDrawBase*>(ZSharedResourcesD3D::g_pInstance->m_pRenderDrawShared);

        m_field16F4 = nullptr;
        m_field16F8 = nullptr;
        m_field16EC = nullptr;
        m_field16F0 = nullptr;
        m_field16FC = nullptr;
        m_field1700 = 0;
    }

    // PC 0x004887E0. Releases the D3D surfaces/textures owned by the render (no-op when null).
    void ZRenderWintelD3D::FreeViewBuffers()
    {
        if (m_pAntialiasSurface && !m_pAntialiasSurface->Release())
            m_pAntialiasSurface = nullptr;
        if (m_pSurface16BC)
        {
            m_pSurface16BC->Release();
            m_pSurface16BC = nullptr;
        }
        if (m_pTexture16B8 && !m_pTexture16B8->Release())
            m_pTexture16B8 = nullptr;
        if (m_pSurface16A4 && !m_pSurface16A4->Release())
            m_pSurface16A4 = nullptr;
        if (m_pSurface16A8 && !m_pSurface16A8->Release())
            m_pSurface16A8 = nullptr;
        if (m_pSwapChain && !m_pSwapChain->Release())
            m_pSwapChain = nullptr;

        for (int i = 0; i < 2; ++i)
        {
            IDirect3DTexture9* pTex = (i == 0) ? m_field16EC : m_field16F0;
            IDirect3DSurface9* pSurf = (i == 0) ? m_field16F4 : m_field16F8;
            if (pTex && !pTex->Release())
                (i == 0) ? (m_field16EC = nullptr) : (m_field16F0 = nullptr);
            if (pSurf && !pSurf->Release())
                (i == 0) ? (m_field16F4 = nullptr) : (m_field16F8 = nullptr);
        }

        if (m_field16FC)
        {
            m_field16FC->Release();
            m_field16FC = nullptr;
        }
    }

    // PC 0x004885D0. Creates the swap chain, back buffer and offscreen surfaces.
    void ZRenderWintelD3D::InitSwapChain()
    {
        if (m_d3dPresentParameters.Windowed)
        {
            ZASSERT(m_pSwapChain == nullptr);
            g_pd3dDevice->CreateAdditionalSwapChain(&m_d3dPresentParameters, &m_pSwapChain);
            m_pSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &m_pSurface16A4);
            g_pd3dDevice->CreateDepthStencilSurface(
                m_d3dPresentParameters.BackBufferWidth,
                m_d3dPresentParameters.BackBufferHeight,
                m_d3dPresentParameters.AutoDepthStencilFormat,
                m_d3dPresentParameters.MultiSampleType,
                0,
                false,
                &m_pSurface16A8,
                nullptr);
            g_pd3dDevice->ColorFill(m_pSurface16A4, nullptr, 0);
        }
        else
        {
            g_pd3dDevice->GetSwapChain(0, &m_pSwapChain);
            g_pd3dDevice->GetRenderTarget(0, &m_pSurface16A4);
            g_pd3dDevice->GetDepthStencilSurface(&m_pSurface16A8);
        }

        D3DSURFACE_DESC sColorDesc, sDepthDesc;
        m_pSurface16A4->GetDesc(&sColorDesc);
        m_pSurface16A8->GetDesc(&sDepthDesc);

        if (g_pRenderDll->m_lAntialias)
        {
            g_pd3dDevice->CreateRenderTarget(
                sColorDesc.Width,
                sColorDesc.Height,
                sColorDesc.Format,
                static_cast<D3DMULTISAMPLE_TYPE>(g_pRenderDll->m_lAntialias),
                0,
                false,
                &m_pAntialiasSurface,
                nullptr);
            g_pd3dDevice->ColorFill(m_pAntialiasSurface, nullptr, 0);
        }

        g_pd3dDevice->CreateTexture(
            sColorDesc.Width,
            sColorDesc.Height,
            1u,
            1u,
            sColorDesc.Format,
            D3DPOOL_DEFAULT,
            &m_pTexture16B8,
            nullptr);
        ZASSERT(m_pTexture16B8 != nullptr);
        m_pTexture16B8->GetSurfaceLevel(0, &m_pSurface16BC);
        ZASSERT(m_pSurface16BC != nullptr);
        g_pd3dDevice->ColorFill(m_pSurface16BC, nullptr, 0);

        for (int i = 0; i < 2; ++i)
        {
            IDirect3DTexture9** ppTex = (i == 0) ? &m_field16EC : &m_field16F0;
            IDirect3DSurface9** ppSurf = (i == 0) ? &m_field16F4 : &m_field16F8;
            g_pd3dDevice->CreateTexture(sColorDesc.Width, sColorDesc.Height, 1u, 1u, sColorDesc.Format, D3DPOOL_DEFAULT, ppTex, nullptr);
            (*ppTex)->GetSurfaceLevel(0, ppSurf);
        }

        g_pd3dDevice->CreateDepthStencilSurface(
            sColorDesc.Width,
            sColorDesc.Height,
            m_d3dPresentParameters.AutoDepthStencilFormat,
            D3DMULTISAMPLE_NONE,
            0,
            false,
            &m_field16FC,
            nullptr);
    }

    // PC 0x0048B210 (Release PC 0x0048C090)
    ZRenderWintelD3D::~ZRenderWintelD3D()
    {
        if (m_bInitialized)
        {
            ZRender::Close();
            FreeViewBuffers();
        }

        ZASSERT(m_pSwapChain == nullptr); // PC: if (m_pSwapChain) __debugbreak();
    }

    // PC 0x00488BE0 (slot 9, fills the antialias + backbuffer surfaces)
    void ZRenderWintelD3D::ColorFill()
    {
        if (g_pRenderDll->m_lAntialias)
            g_pd3dDevice->ColorFill(m_pAntialiasSurface, nullptr, 0);
        g_pd3dDevice->ColorFill(m_pSurface16BC, nullptr, 0);
    }

    // PC 0x00488C20
    bool ZRenderWintelD3D::BeginScene()
    {
        if (g_bd3dDeviceLost)
            return false;

        ZRenderX86::BeginScene();
        ZASSERT(!m_bInScene); // PC: if (m_bInScene) __debugbreak();

        if (g_pd3dDevice->BeginScene() < 0)
            return false;

        m_bInScene = true;
        g_pd3dDevice->SetRenderTarget(0, m_pSurface16A4);
        g_pd3dDevice->SetDepthStencilSurface(m_pSurface16A8);

        if (ZSharedResourcesD3D::g_bSVBLockAcquired)
        {
            ZSharedResourcesD3D::g_pInstance->m_pSVB->LockedData();
            ZSharedResourcesD3D::g_pInstance->m_pSVB->Interface();
            ZSharedResourcesD3D::g_bSVBLockAcquired = false;
        }

        return true;
    }

    // PC 0x00488CB0
    bool ZRenderWintelD3D::EndScene()
    {
        ZASSERT(m_bInScene); // PC: if (!m_bInScene) __debugbreak();

        g_pd3dDevice->EndScene();
        m_bInScene = false;
        return true;
    }

    // PC 0x0048BE70 (implements the base pure virtual Flip)
    void ZRenderWintelD3D::Flip()
    {
        ZSharedResourcesD3D::g_pInstance->m_pSVB->Interface();

        const HRESULT hPresent = (!m_pSwapChain) ? D3DERR_DEVICELOST : m_pSwapChain->Present(nullptr, nullptr, nullptr, nullptr, 0);
        if (hPresent == D3DERR_DEVICELOST || g_bForceResetDevice)
        {
            g_bd3dDeviceLost = true;

            if (g_bForceResetDevice || g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
            {
                g_bForceResetDevice = false;
                if (m_bInScene)
                    EndScene();
                m_pRenderDraw->Flush();
                g_bIsResettingDevice = true;
                m_pRenderDraw->FreeDeviceBuffers();
                FreeViewBuffers();

                if (m_pBinkVideoTextureD3D)
                {
                    DestroyOffscreenSurface();
                    m_pOffscreenSurface = 1;
                }

                g_pRenderDll->m_pMaterialBuffer->FreeResources();
                ZSharedResourcesD3D::g_pInstance->Free();
                g_bIsResettingDevice = false;

                m_d3dPresentParameters.BackBufferWidth = g_pSysInterface->m_lResolution[0];
                m_d3dPresentParameters.BackBufferHeight = g_pSysInterface->m_lResolution[1];
                if (!g_pd3dDevice->Reset(&m_d3dPresentParameters))
                {
                    InitSwapChain();
                    sub_48AB70();
                    m_pRenderDraw->AllocateDeviceBuffers();
                    g_bd3dDeviceLost = false;
                    SysInput::instance->ResetTables(true);
                }
            }
        }

        if (g_lShowBufferAllocators)
        {
            PlotOffsetAlloc(10, 10, "StaticVB", &ZSharedResourcesD3D::g_pInstance->m_pVertexAllocator->m_Allocator, ZSharedResourcesD3D::g_pInstance->m_pVertexAllocator->m_lMaxNumObjects);
            PlotOffsetAlloc(10, 11, "StaticIB", &ZSharedResourcesD3D::g_pInstance->m_pIndexAllocator->m_Allocator, ZSharedResourcesD3D::g_pInstance->m_pIndexAllocator->m_lMaxNumObjects);
        }
    }

    // PC 0x00488CF0
    void ZRenderWintelD3D::SetViewport(const float fX, const float fY, const float fW, const float fH)
    {
        D3DVIEWPORT9 viewport;
        if (m_bLetterBox)
        {
            viewport.X = static_cast<DWORD>(fX);
            viewport.Y = static_cast<DWORD>((fH - fH * 0.75f) * 0.5f + fY);
            viewport.Width = static_cast<DWORD>(fW);
            viewport.Height = static_cast<DWORD>(fH * 0.5f);
        }
        else
        {
            viewport.X = static_cast<DWORD>(fX);
            viewport.Y = static_cast<DWORD>(fY);
            viewport.Width = static_cast<DWORD>(fW);
            viewport.Height = static_cast<DWORD>(fH);
        }
        viewport.MinZ = 0.0f;
        viewport.MaxZ = 1.0f;

        ZASSERT(viewport.X + viewport.Width <= static_cast<DWORD>(GetSizeX()));
        ZASSERT(viewport.Y + viewport.Height <= static_cast<DWORD>(GetSizeY()));

        g_pd3dDevice->SetViewport(&viewport);
    }

    // PC 0x00488EA0 (ClearViewport: D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL)
    void ZRenderWintelD3D::ClearViewport(const unsigned int lFlags, const unsigned int lColor)
    {
        DWORD lClearFlags = 0;
        D3DCOLOR lClearColor = static_cast<D3DCOLOR>(lColor);
        if (g_bDisablePostEffects)
            lClearColor = 0xFFFFFFFF;

        if (lFlags & 1)
            lClearFlags |= D3DCLEAR_TARGET;
        if (lFlags & 2)
        {
            lClearFlags |= D3DCLEAR_ZBUFFER;
            if (field_16AE)
                lClearFlags |= D3DCLEAR_STENCIL;
        }

        g_pd3dDevice->Clear(0, nullptr, lClearFlags, lClearColor, 1.0f, 0);
    }

    // PC 0x00489170. Sets the fog table render states. The game uses the legacy D3D8 fog-table
    // ids 36/37 (D3DRENDERSTATE_FOGTABLESTART/END), which the D3D9 SDK does not define.
    void ZRenderWintelD3D::SetFog(const float fFogNear, const float fFogFar, const unsigned int dwFogColor)
    {
        if (g_bDisablePostEffects)
        {
            g_dwFogColor = 0xFFFFFFFF;
            g_fFogNear = 1.0f;
            g_fFogFar = 7000.0f;
            g_pd3dDevice->SetRenderState(D3DRS_FOGCOLOR, 0xFFFFFFFF);
        }
        else
        {
            g_dwFogColor = dwFogColor | 0xFF000000;
            g_fFogFar = fFogFar;
            g_fFogNear = fFogNear;
            g_pd3dDevice->SetRenderState(D3DRS_FOGCOLOR, dwFogColor);
        }

        g_pd3dDevice->SetRenderState(D3DRS_FOGTABLEMODE, 0);
        g_pd3dDevice->SetRenderState(static_cast<D3DRENDERSTATETYPE>(36), *reinterpret_cast<const DWORD*>(&fFogNear)); // FOGTABLESTART
        g_pd3dDevice->SetRenderState(static_cast<D3DRENDERSTATETYPE>(37), *reinterpret_cast<const DWORD*>(&fFogFar));  // FOGTABLEEND
        g_pd3dDevice->SetRenderState(D3DRS_FOGVERTEXMODE, 0);
    }

    // PC 0x00488410
    const char* ZRenderWintelD3D::GetRenderName()
    {
        return "Direct3D9";
    }

    // PC 0x004884F0 (implements BackupBufferExists, slot 152)
    bool ZRenderWintelD3D::BackupBufferExists()
    {
        return m_field159C != 0;
    }

    // PC 0x004888E0. Fills the GLC caps from the D3D device: antialias multisample support plus
    // the shader/texture-compression flags (EMBM/DXT capability checks come from DLLTEXCON).
    void ZRenderWintelD3D::GetRenderCaps(_GLC_RENDERCAPS* pCaps)
    {
        int lSamplesNr = static_cast<int>(D3DMULTISAMPLE_NONE);
        uint32_t lCaps = 0;
        uint32_t lMultiSample = 0;

        do
        {
            if (lSamplesNr >= static_cast<int>(D3DMULTISAMPLE_2_SAMPLES)
                && g_pd3dInterface->CheckDeviceMultiSampleType(
                       uActiveAdapter,
                       D3DDEVTYPE_HAL,
                       m_d3dPresentParameters.BackBufferFormat,
                       !g_pSysInterface->m_bFullScreen,
                       static_cast<D3DMULTISAMPLE_TYPE>(lSamplesNr),
                       0) >= 0)
            {
                lCaps |= 2u;
                lMultiSample |= 1u << (lSamplesNr - 1);
            }
            ++lSamplesNr;
        } while (lSamplesNr < static_cast<int>(D3DMULTISAMPLE_16_SAMPLES));

        if ((m_field1554 & 0x20000) != 0)
            lCaps |= 1u;

        auto* pTexMan = static_cast<ZTextureManagerD3D*>(g_pRenderDll->m_pTexCon);
        if (pTexMan->HasDXT())
            lCaps |= 8u;
        if (pTexMan->HasEMBM())
            lCaps |= 4u;

        pCaps->dwCaps = (m_field1570 & 0x20000) != 0 ? (lCaps | 0x10) : lCaps;
        pCaps->dwMultiSample = lMultiSample;
    }

    // PC 0x004889C0. Handles the GLC render states owned by the D3D render and forwards the rest.
    void ZRenderWintelD3D::SetRenderState(const _GLC_RENDERSTATE eRenderState, const unsigned int* pValue)
    {
        switch (eRenderState)
        {
        case GLC_RENDERSTATE_ANISOTROPY:
            if (m_field159C > 1)
            {
                if (*pValue)
                {
                    g_lMaxAnisotropy = *pValue;
                    if (*pValue >= static_cast<unsigned int>(m_field159C))
                        g_lMaxAnisotropy = static_cast<unsigned int>(m_field159C);
                    g_dwMinFilter = 2;
                    g_dwMagFilter = 3;
                }
                else
                {
                    g_lMaxAnisotropy = 1;
                    g_dwMinFilter = 2;
                    g_dwMagFilter = 2;
                }
                g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, g_dwMagFilter);
                g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, g_dwMinFilter);
                g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAXANISOTROPY, g_lMaxAnisotropy);
                g_pRenderDll->m_lAnisotropy = *pValue;
            }
            break;
        case GLC_RENDERSTATE_ANTIALIAS:
            if (g_pSysInterface->m_bFullScreen)
            {
                if (g_pRenderDll->m_lAntialias != *pValue
                    && g_pd3dInterface->CheckDeviceMultiSampleType(
                           uActiveAdapter,
                           D3DDEVTYPE_HAL,
                           m_d3dPresentParameters.BackBufferFormat,
                           0,
                           static_cast<D3DMULTISAMPLE_TYPE>(*pValue),
                           0) >= 0)
                {
                    g_pRenderDll->m_lAntialias = *pValue;
                    m_d3dPresentParameters.MultiSampleType = static_cast<D3DMULTISAMPLE_TYPE>(g_pRenderDll->m_lAntialias);
                    m_d3dPresentParameters.Flags = 0;
                }
            }
            else
            {
                g_pRenderDll->m_lAntialias = 0;
            }
            break;
        case GLC_RENDERSTATE_DXT:
            if (static_cast<ZTextureManagerD3D*>(g_pRenderDll->m_pTexCon)->HasDXT())
                g_pRenderDll->m_bDisableDXT = (*pValue == 0);
            break;
        case GLC_RENDERSTATE_VSYNC:
            g_pRenderDll->m_bDisableVSync = (*pValue == 0);
            m_d3dPresentParameters.PresentationInterval = g_pRenderDll->m_bDisableVSync ? 0x80000000u : 1;
            break;
        case GLC_RENDERSTATE_EMBM:
            if (static_cast<ZTextureManagerD3D*>(g_pRenderDll->m_pTexCon)->HasEMBM())
                g_pRenderDll->m_bDisableEMBM = (*pValue == 0);
            break;
        case GLC_RENDERSTATE_TRILINEAR:
            if ((m_field1570 & 0x20000) != 0)
            {
                g_dwMipFilter = (*pValue != 0) + 1;
                g_pRenderDll->m_bTrilinearFiltering = (*pValue != 0);
            }
            break;
        default:
            ZRenderX86::SetRenderState(eRenderState, pValue);
            break;
        }
    }

    // PC 0x00489370
    void ZRenderWintelD3D::SetGammaValue(const float fGamma)
    {
        ZRenderX86::SetGammaValue(fGamma);

        D3DGAMMARAMP sGammaRamp;
        for (int i = 0; i < 256; ++i)
        {
            double v = 0.0;
            if (i)
            {
                v = std::pow(static_cast<double>(i) * 0.00390625, 2.0 - fGamma) * 65535.0;
                if (v < 0.0)
                    v = 0.0;
                else if (v > 65535.0)
                    v = 65535.0;
            }
            const WORD wValue = static_cast<WORD>(v);
            sGammaRamp.red[i] = wValue;
            sGammaRamp.green[i] = wValue;
            sGammaRamp.blue[i] = wValue;
        }
        g_pd3dDevice->SetGammaRamp(0, 0, &sGammaRamp);
    }

    // PC 0x00488590 (implements the Wintel pure virtual OnPaint)
    void ZRenderWintelD3D::OnPaint(int)
    {
        PAINTSTRUCT paint;
        BeginPaint(static_cast<HWND>(m_hWnd), &paint);
        EndPaint(static_cast<HWND>(m_hWnd), &paint);
    }

    // PC 0x0048AA60 (implements the Wintel pure virtual OnSize)
    void ZRenderWintelD3D::OnSize(unsigned int lWidth, unsigned int lHeight)
    {
        g_pSysInterface->m_lResolutionWanted[0] = static_cast<int>(GetSizeX());
        g_pSysInterface->m_lResolutionWanted[1] = static_cast<int>(GetSizeY());

        const bool bFullScreen = g_pSysInterface->m_bFullScreen;
        unsigned int v6 = bFullScreen ? static_cast<unsigned int>(g_pSysInterface->m_lResolution[0]) : lWidth;
        unsigned int v7 = bFullScreen ? static_cast<unsigned int>(g_pSysInterface->m_lResolution[1]) : lHeight;

        if (v6 && v7)
        {
            if (!bFullScreen)
            {
                g_pSysInterface->m_lResolution[0] = static_cast<int>(v6);
                g_pSysInterface->m_lResolution[1] = static_cast<int>(v7);
            }

            if (!m_bInitialized || g_bd3dDeviceLost)
            {
                SetSize(v6, v7);
            }
            else
            {
                FreeViewBuffers();
                m_d3dPresentParameters.BackBufferWidth = v6;
                m_d3dPresentParameters.BackBufferHeight = v7;
                SetSize(v6, v7);
                InitSwapChain();
                g_pd3dDevice->SetRenderTarget(0, m_pSurface16A4);
                g_pd3dDevice->SetDepthStencilSurface(m_pSurface16A8);
            }

            m_pRenderDraw->UpdateRenderSize(this, v6, v7);
            Update();
        }
    }

    // PC 0x00489330 (implements the Wintel virtual Clear)
    void ZRenderWintelD3D::Clear()
    {
        if (g_pd3dDevice)
        {
            g_bForceResetDevice = true;
            g_bd3dDeviceLost = true;
            g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET, 0, 1.0f, 0);
            Flip();
        }
    }

    // PC 0x00488420. Reads the "OverrideSLI" option or probes NVCPL.dll for a multi-GPU setup
    // that needs forced rendering on a single adapter.
    void ZRenderWintelD3D::HandleOverrideSLIOption()
    {
        char* pszValue = nullptr;
        if (ZSysInterface::GetOption("OverrideSLI", &pszValue))
        {
            g_lOverrideSLI = atol(pszValue);
            return;
        }

        int lNrGpus = 0;
        int lSliFlags = 0;
        g_lOverrideSLI = 0;

        if (HMODULE hLib = LoadLibraryA("NVCPL.dll"))
        {
            using FnNvCplGetDataInt = int(__cdecl*)(int, int*);
            auto pfnNvCplGetDataInt = reinterpret_cast<FnNvCplGetDataInt>(GetProcAddress(hLib, "NvCplGetDataInt"));
            if (pfnNvCplGetDataInt
                && pfnNvCplGetDataInt(8, &lNrGpus)
                && lNrGpus > 0
                && pfnNvCplGetDataInt(9, &lSliFlags)
                && (lSliFlags & 0x10000000) != 0
                && (lSliFlags & 4) == 0)
            {
                g_lOverrideSLI = 1;
            }
            else
            {
                FreeLibrary(hLib);
            }
        }
    }

    // PC 0x0048B280 (implements the Wintel pure virtual Init). Full D3D9 device initialisation:
    // creates the IDirect3D9, picks the adapter, builds the present parameters with a working
    // depth-stencil format, creates the device, the ZDirect3DDevice wrapper, the texture manager
    // and enumerates the available resolutions.
    void ZRenderWintelD3D::Init()
    {
        HandleOverrideSLIOption();

        if (ZSysInterface::GetOption("BreakOnVideoInit", nullptr))
            ZASSERT(false); // PC: __debugbreak()

        char* pszOpt = nullptr;
        if (ZSysInterface::GetOption("PixelAspect", &pszOpt))
            m_fPixelAspect = static_cast<float>(atof(pszOpt));
        m_fPixelAspect = 1.0f;

        if (ZSysInterface::GetOption("MaxAnisotropy", &pszOpt))
            g_lMaxAnisotropy = static_cast<uint32_t>(atol(pszOpt));

        if (!g_pd3dInterface)
        {
            g_pd3dInterface = Direct3DCreate9(D3D_SDK_VERSION);
            if (!g_pd3dInterface)
                throw ZExceptionRenderD3D("Direct3D", "Unable to create Direct3D9");
        }

        // Adapter selection: pick the adapter that owns the render window's monitor.
        const UINT lAdapterCount = g_pd3dInterface->GetAdapterCount();
        HMONITOR hRenderMonitor = MonitorFromWindow(static_cast<HWND>(m_hWnd), MONITOR_DEFAULTTONEAREST);
        uActiveAdapter = 0;
        UINT lAdapter = 0;
        for (UINT i = 0; i < lAdapterCount; ++i)
        {
            if (g_pd3dInterface->GetAdapterMonitor(i) == hRenderMonitor)
            {
                uActiveAdapter = static_cast<int>(i);
                m_field169C = reinterpret_cast<int>(hRenderMonitor);
                lAdapter = i;
                break;
            }
            lAdapter = i;
        }

        if (ZSysInterface::GetOption("DebugVertexShader", nullptr))
            ZSharedResourcesD3D::g_pInstance->m_bDebugVertexShader = true;
        if (ZSysInterface::GetOption("DebugPixelShader", nullptr))
            ZSharedResourcesD3D::g_pInstance->m_bDebugPixelShader = true;

        D3DCAPS9 sCaps;
        g_pd3dInterface->GetDeviceCaps(uActiveAdapter, D3DDEVTYPE_HAL, &sCaps);
        const bool bHWTnL = (sCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0;

        if (!bHWTnL)
            ZSharedResourcesD3D::g_pInstance->m_bUseSoftwareVertexProcessing = true;
        if (ZSharedResourcesD3D::g_pInstance->m_bUseSoftwareVertexProcessing && g_pRenderDll->m_bDisableHWTnL)
            ZSharedResourcesD3D::g_pInstance->m_bUseSoftwareVertexProcessing = true;

        // TextureCaps / surface flags: m_field16AC (Z-stencil enabled without a specific cap),
        // m_field16AD (hardware TnL available).
        field_16AC = (sCaps.TextureCaps & D3DPTEXTURECAPS_ALPHAPALETTE) && !(sCaps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY);
        field_16AD = 1;
        if (g_pRenderDll->m_bDisableHWTnL)
            field_16AD = 0;

        // Shader-quality level derived from the vertex shader version + the "ShaderQuality" option.
        const DWORD lVertexShaderVersion = sCaps.VertexShaderVersion & 0xFFFF;
        int lShaderQuality = 0;
        if (lVertexShaderVersion >= 0x101)
        {
            if (lVertexShaderVersion >= 0x200)
                lShaderQuality = 3 - (lVertexShaderVersion < 0x300);
            else
                lShaderQuality = 1;
        }

        char* pszShaderQuality = nullptr;
        if (ZSysInterface::GetOption("ShaderQuality", &pszShaderQuality))
        {
            int lQuality = lShaderQuality;
            if (!stricmp(pszShaderQuality, "Low") || !stricmp(pszShaderQuality, "Medium"))
                lQuality = 1;
            else if (!stricmp(pszShaderQuality, "High"))
                lQuality = 2;
            else if (!stricmp(pszShaderQuality, "VeryHigh"))
                lQuality = 3;
            if (lQuality <= lShaderQuality)
                lShaderQuality = lQuality;
        }

        if (!lShaderQuality)
            g_pSysInterface->m_bPostFiltersAvailable = false;
        ZSharedResourcesD3D::g_pInstance->m_lShaderQuality = lShaderQuality;

        if ((sCaps.PixelShaderVersion & 0xFFFF) == 0)
            ZSharedResourcesD3D::g_pInstance->m_bUseSoftwareVertexProcessing = true;

        // CPUID feature flags: MMX (bit 23) / SSE2 (bit 26, the PC shifts EDX by 26), kept enabled
        // unless the corresponding option is set.
        int aCpuInfo[4] = {};
        __cpuid(aCpuInfo, 1);
        const bool bHasMMX = (aCpuInfo[3] & 0x800000) != 0;
        m_bDisableMMX = bHasMMX && !ZSysInterface::GetOption("DisableMMX", nullptr);
        const bool bHasSSE = ((aCpuInfo[3] >> 26) & 1) != 0;
        m_bDisableSSE = bHasSSE && !ZSysInterface::GetOption("DisableSSE", nullptr);

        D3DDISPLAYMODE sDisplayMode;
        if (g_pd3dInterface->GetAdapterDisplayMode(uActiveAdapter, &sDisplayMode) < 0)
            throw ZExceptionRenderD3D("Direct3D", "Unable to get adapter display mode");

        D3DFORMAT lBackBufferFormat = sDisplayMode.Format;
        if (g_pSysInterface->m_bFullScreen)
            lBackBufferFormat = (g_pSysInterface->m_lBitsPerPixel != 32) ? D3DFMT_R5G6B5 : D3DFMT_X8R8G8B8;

        std::memset(&m_d3dPresentParameters, 0, sizeof(m_d3dPresentParameters));
        m_d3dPresentParameters.hDeviceWindow = static_cast<HWND>(m_hWnd);
        m_d3dPresentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
        m_d3dPresentParameters.Windowed = !g_pSysInterface->m_bFullScreen;
        m_d3dPresentParameters.BackBufferFormat = D3DFMT_A8R8G8B8;
        m_d3dPresentParameters.BackBufferCount = g_pRenderDll->m_bEnableTripleBuffering + 1;
        m_d3dPresentParameters.BackBufferWidth = static_cast<UINT>(GetSizeX());
        m_d3dPresentParameters.BackBufferHeight = static_cast<UINT>(GetSizeY());
        m_d3dPresentParameters.EnableAutoDepthStencil = TRUE;
        m_d3dPresentParameters.PresentationInterval = g_pRenderDll->m_bDisableVSync ? D3DPRESENT_INTERVAL_IMMEDIATE : D3DPRESENT_INTERVAL_ONE;

        D3DDEVTYPE eDeviceType = D3DDEVTYPE_HAL;
        if (ZSharedResourcesD3D::g_pInstance->m_bDebugVertexShader)
        {
            ZSharedResourcesD3D::g_pInstance->m_bUseSoftwareVertexProcessing = true;
            eDeviceType = D3DDEVTYPE_REF;
        }
        if (ZSharedResourcesD3D::g_pInstance->m_bDebugPixelShader)
            eDeviceType = D3DDEVTYPE_REF;

        if (g_pSysInterface->m_bFullScreen)
        {
            if (g_pRenderDll->m_lAntialias == 1)
                g_pRenderDll->m_lAntialias = 0;
            if (g_pRenderDll->m_lAntialias > 16)
                g_pRenderDll->m_lAntialias = 16;

            if (g_pRenderDll->m_lAntialias)
            {
                if (g_pd3dInterface->CheckDeviceMultiSampleType(
                        uActiveAdapter,
                        eDeviceType,
                        sDisplayMode.Format,
                        !g_pSysInterface->m_bFullScreen,
                        static_cast<D3DMULTISAMPLE_TYPE>(g_pRenderDll->m_lAntialias),
                        0) < 0)
                    g_pRenderDll->m_lAntialias = 0;
                else
                    m_d3dPresentParameters.MultiSampleType = static_cast<D3DMULTISAMPLE_TYPE>(g_pRenderDll->m_lAntialias);
            }
        }
        else
        {
            g_pRenderDll->m_lAntialias = 0;
        }

        // Depth-stencil format selection: prefer D24S8, fall back through D24X4S4/D15S1/D32/D16.
        field_16AE = 1;
        m_d3dPresentParameters.AutoDepthStencilFormat = D3DFMT_D24S8;
        int lDepthPrecision = 24;
        if (g_pd3dInterface->CheckDeviceFormat(uActiveAdapter, eDeviceType, lBackBufferFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24S8) < 0)
        {
            m_d3dPresentParameters.AutoDepthStencilFormat = D3DFMT_D24X4S4;
            if (g_pd3dInterface->CheckDeviceFormat(uActiveAdapter, eDeviceType, lBackBufferFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24X4S4) < 0)
            {
                m_d3dPresentParameters.AutoDepthStencilFormat = D3DFMT_D15S1;
                lDepthPrecision = 15;
                if (g_pd3dInterface->CheckDeviceFormat(uActiveAdapter, eDeviceType, lBackBufferFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D15S1) < 0)
                {
                    field_16AE = 0;
                    m_d3dPresentParameters.AutoDepthStencilFormat = D3DFMT_D32;
                    lDepthPrecision = 32;
                    if (g_pd3dInterface->CheckDeviceFormat(uActiveAdapter, eDeviceType, lBackBufferFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D32) < 0)
                    {
                        m_d3dPresentParameters.AutoDepthStencilFormat = D3DFMT_D16;
                        lDepthPrecision = 16;
                        if (g_pd3dInterface->CheckDeviceFormat(uActiveAdapter, eDeviceType, lBackBufferFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D16) < 0)
                            throw ZExceptionRenderD3D("Direct3D", "Unable to create device. Try changing resolution or color depth");
                    }
                }
            }
        }
        ZSharedResourcesD3D::g_pInstance->m_lDepthBufferPrecision = lDepthPrecision;
        m_d3dPresentParameters.Flags = 0;

        if (!g_pd3dDevice)
        {
            const DWORD lCreateFlags = (ZSharedResourcesD3D::g_pInstance->m_bUseSoftwareVertexProcessing ? D3DCREATE_SOFTWARE_VERTEXPROCESSING : D3DCREATE_MIXED_VERTEXPROCESSING) | D3DCREATE_MULTITHREADED;
            IDirect3DDevice9* pRawDevice = nullptr;
            const HRESULT hCreate = g_pd3dInterface->CreateDevice(uActiveAdapter, eDeviceType, static_cast<HWND>(m_hWnd), lCreateFlags, &m_d3dPresentParameters, &pRawDevice);
            if (hCreate < 0)
                throw ZExceptionRenderD3D("Direct3D", hCreate == 0x8876017C ? "Not enough video memory. Try changing resolution or color depth" : "Unable to create device. Try changing resolution or color depth");

            g_pd3dDevice = ZUniMemory::New<ZDirect3DDevice>(pRawDevice);

            g_pd3dDevice->GetSwapChain(0, &m_pSwapChain);
            if (m_pSwapChain && !m_pSwapChain->Release())
                m_pSwapChain = nullptr;

            auto* pTexMan = ZUniMemory::New<ZTextureManagerD3D>(g_pd3dDevice);
            g_pRenderDll->m_pTexCon = pTexMan;
            pTexMan->CheckFormats(lBackBufferFormat);

            if (!g_pd3dInterface->CheckDeviceFormat(uActiveAdapter, eDeviceType, lBackBufferFormat, 0x8 /*D3DUSAGE_TEXTURE*/, D3DRTYPE_TEXTURE, D3DFMT_R32F))
            {
                ZSharedResourcesD3D::g_pInstance->m_lShadowColorFormat = static_cast<D3DFORMAT>(114); // D3DFMT_R32F
                ZSharedResourcesD3D::g_pInstance->m_eShadowMode = ZSharedResourcesD3D::SHADOWMODE_COLOR;
            }
            if (!g_pd3dInterface->CheckDeviceFormat(uActiveAdapter, eDeviceType, lBackBufferFormat, 0x8 /*D3DUSAGE_TEXTURE*/, D3DRTYPE_TEXTURE, D3DFMT_R16F))
            {
                ZSharedResourcesD3D::g_pInstance->m_lShadowColorFormat = static_cast<D3DFORMAT>(111); // D3DFMT_R16F
                ZSharedResourcesD3D::g_pInstance->m_eShadowMode = ZSharedResourcesD3D::SHADOWMODE_COLOR;
            }

            if (!m_d3dPresentParameters.Windowed)
                InitSwapChain();

            ZSharedResourcesD3D::g_pInstance->Allocate();
            g_pRenderDll->m_pMaterialBuffer->AllocateResources();

            if (m_pOffscreenSurface)
                CreateOffscreenSurface(m_iOffscreenSurfaceWidth, m_iOffscreenSurfaceHeight);
        }

        if (m_d3dPresentParameters.Windowed)
            InitSwapChain();

        if (!m_pContext)
            m_pContext = static_cast<ZRenderContext*>(ZUniMemory::Allocate(664));

        // EMBM / DXT capability checks (the PC calls the texture manager's virtuals).
        auto* pTexCon = static_cast<ZTextureManagerD3D*>(g_pRenderDll->m_pTexCon);
        if (pTexCon->HasEMBM())
        {
            if (!pTexCon->HasEMBMUVL())
                field_16AF = 0;
        }
        else
        {
            g_pRenderDll->m_bDisableEMBM = 1;
        }
        if (!pTexCon->HasDXT())
            g_pRenderDll->m_bDisableDXT = 1;

        // Vendor-specific workarounds from the adapter identifier.
        D3DADAPTER_IDENTIFIER9 sAdapterIdentifier;
        g_pd3dInterface->GetAdapterIdentifier(uActiveAdapter, 0, &sAdapterIdentifier);

        m_field1708 = 8;
        if (sAdapterIdentifier.VendorId == 0x1039)
        {
            m_field1708 = 0;
        }
        else if (sAdapterIdentifier.VendorId == 0x102B)
        {
            if (sAdapterIdentifier.DeviceId == 0x527)
                field_1526 = 1;
        }
        else
        {
            if (sAdapterIdentifier.VendorId == 0x1142 || sAdapterIdentifier.VendorId == 0x10D9 || sAdapterIdentifier.VendorId == 0x121A)
                g_pRenderDll->m_bDisableDXT = 1;
            if (sAdapterIdentifier.VendorId == 0x1002 && sAdapterIdentifier.DeviceId == 20823)
                ZSharedResourcesD3D::g_pInstance->m_lShaderQuality = 0;
        }

        g_dwTextureUnits = static_cast<uint32_t>(sCaps.MaxSimultaneousTextures);
        g_dwTextureStages = static_cast<uint32_t>(sCaps.MaxTextureBlendStages);

        if (g_pRenderDll->m_fGammaValue != 1.0f)
            SetGammaValue(g_pRenderDll->m_fGammaValue);
        SetRenderState(GLC_RENDERSTATE_ANISOTROPY, reinterpret_cast<const unsigned int*>(&g_pRenderDll->m_lAnisotropy));

        if (field_16AC)
            g_pSysInterface->m_bPostFiltersAvailable = false;

        SetRenderState(GLC_RENDERSTATE_DXT, nullptr);
        Flip();
        SetRenderState(GLC_RENDERSTATE_DXT, nullptr);
        Flip();
        m_bInitialized = true;

        // Resolution enumeration into m_Resolutions (deduplicated, capped at the desktop size).
        const UINT lModeCount = g_pd3dInterface->GetAdapterModeCount(lAdapter, D3DFMT_X8R8G8B8);
        RECT rect;
        GetWindowRect(GetDesktopWindow(), &rect);
        const uint32_t lMaxWidth = rect.right - rect.left;
        const uint32_t lMaxHeight = rect.bottom - rect.top;
        uint32_t lResCount = 0;
        for (UINT i = 0; i < lModeCount && lResCount < 20; ++i)
        {
            D3DDISPLAYMODE mode;
            if (g_pd3dInterface->EnumAdapterModes(lAdapter, D3DFMT_X8R8G8B8, i, &mode))
                continue;
            if (g_pSysInterface->m_bFullScreen || (mode.Width <= lMaxWidth && mode.Height <= lMaxHeight))
            {
                bool bDup = false;
                for (uint32_t d = 0; d < lResCount; ++d)
                {
                    if (m_Resolutions[d].dwSizeX == mode.Width && m_Resolutions[d].dwSizeY == mode.Height)
                    {
                        bDup = true;
                        break;
                    }
                }
                if (!bDup)
                {
                    m_Resolutions[lResCount].dwSizeX = mode.Width;
                    m_Resolutions[lResCount].dwSizeY = mode.Height;
                    m_Resolutions[lResCount].dwBitsPerPixel = (mode.Format == D3DFMT_X8R8G8B8) ? 32 : 16;
                    m_Resolutions[lResCount].bFullScreen = g_pSysInterface->m_bFullScreen ? 1 : 0;
                    ++lResCount;
                }
            }
        }
        if (lResCount < 20)
            m_Resolutions[lResCount] = { 0, 0, 0, 0, _GLC_RENDER_RESOLUTION::Aspect4_3 };
    }

    // PC 0x00489270
    void ZRenderWintelD3D::MakeMe(HWND hWnd)
    {
        m_hWnd = hWnd;

        if (g_pSysInterface->m_sGameTitle.Length())
            AdjustWindow(static_cast<const char*>(g_pSysInterface->m_sGameTitle));
        else
            AdjustWindow("Direct3D");

        Init();

        DEVMODEA devMode;
        devMode.dmSize = sizeof(DEVMODEA);
        devMode.dmDriverExtra = 0;
        EnumDisplaySettingsA(nullptr, ENUM_CURRENT_SETTINGS, &devMode);
        m_FPS = devMode.dmDisplayFrequency;
    }

    // PC 0x00489310 (AdjustWindow override: base + Clear)
    int ZRenderWintelD3D::AdjustWindow(const char* pWindowTitle)
    {
        ZRenderWintel::AdjustWindow(pWindowTitle);
        Clear();
        return 0;
    }

    namespace
    {
        // PC sub_43A4A0 / iOS writetgacropped: writes the buffer as a 24-bit top-down TGA.
        void WriteTGA(const char* pszFileName, const uint32_t* pPixels, int lWidth, int lHeight)
        {
            FILE* pFile = nullptr;
            if (fopen_s(&pFile, pszFileName, "wb"))
                return;

            uint8_t aHeader[18] = {};
            aHeader[2] = 2; // uncompressed true-color
            aHeader[12] = static_cast<uint8_t>(lWidth & 0xFF);
            aHeader[13] = static_cast<uint8_t>((lWidth >> 8) & 0xFF);
            aHeader[14] = static_cast<uint8_t>(lHeight & 0xFF);
            aHeader[15] = static_cast<uint8_t>((lHeight >> 8) & 0xFF);
            aHeader[16] = 24;
            aHeader[17] = 0x20; // top-left origin
            fwrite(aHeader, 1, sizeof(aHeader), pFile);

            for (int y = 0; y < lHeight; ++y)
            {
                for (int x = 0; x < lWidth; ++x)
                {
                    const uint32_t lPx = pPixels[y * lWidth + x];
                    const uint8_t aBgr[3] = { static_cast<uint8_t>(lPx & 0xFF), static_cast<uint8_t>((lPx >> 8) & 0xFF), static_cast<uint8_t>((lPx >> 16) & 0xFF) };
                    fwrite(aBgr, 1, 3, pFile);
                }
            }
            fclose(pFile);
        }

        // PC sub_43A4A0 / iOS WriteJPEG: writes the buffer through libjpeg as a quality-90 JPEG.
        void WriteJPEG(const char* pszFileName, const uint32_t* pPixels, int lWidth, int lHeight)
        {
            FILE* pFile = nullptr;
            if (fopen_s(&pFile, pszFileName, "wb"))
                return;

            jpeg_compress_struct sCinfo;
            jpeg_error_mgr sJerr;
            sCinfo.err = jpeg_std_error(&sJerr);
            jpeg_create_compress(&sCinfo);
            jpeg_stdio_dest(&sCinfo, pFile);

            sCinfo.image_width = static_cast<JDIMENSION>(lWidth);
            sCinfo.image_height = static_cast<JDIMENSION>(lHeight);
            sCinfo.input_components = 3;
            sCinfo.in_color_space = JCS_RGB;
            jpeg_set_defaults(&sCinfo);
            jpeg_set_quality(&sCinfo, 90, TRUE);
            jpeg_start_compress(&sCinfo, TRUE);

            uint8_t* aRowBuffer = static_cast<uint8_t*>(ZUniMemory::Allocate(static_cast<size_t>(lWidth) * 3));
            while (sCinfo.next_scanline < sCinfo.image_height)
            {
                const uint32_t y = static_cast<uint32_t>(sCinfo.next_scanline);
                for (int x = 0; x < lWidth; ++x)
                {
                    const uint32_t lPx = pPixels[y * lWidth + static_cast<uint32_t>(x)];
                    aRowBuffer[static_cast<size_t>(x) * 3 + 0] = static_cast<uint8_t>(lPx & 0xFF);
                    aRowBuffer[static_cast<size_t>(x) * 3 + 1] = static_cast<uint8_t>((lPx >> 8) & 0xFF);
                    aRowBuffer[static_cast<size_t>(x) * 3 + 2] = static_cast<uint8_t>((lPx >> 16) & 0xFF);
                }
                JSAMPROW aRow[1] = { aRowBuffer };
                jpeg_write_scanlines(&sCinfo, aRow, 1);
            }

            ZUniMemory::Free(aRowBuffer);
            jpeg_finish_compress(&sCinfo);
            jpeg_destroy_compress(&sCinfo);
            fclose(pFile);
        }

        // PC sub_43C720 / iOS writebmp: writes the buffer as a 24-bit bottom-up BMP.
        void WriteBMP(const char* pszFileName, const uint32_t* pPixels, int lWidth, int lHeight)
        {
            FILE* pFile = nullptr;
            if (fopen_s(&pFile, pszFileName, "wb"))
                return;

            const int lRowSize = (3 * lWidth + 3) & ~3;
            const int lDataSize = lRowSize * lHeight;

            BITMAPFILEHEADER aFileHeader = {};
            aFileHeader.bfType = 0x4D42;
            aFileHeader.bfSize = 54 + lDataSize;
            aFileHeader.bfOffBits = 54;
            fwrite(&aFileHeader, 1, sizeof(aFileHeader), pFile);

            BITMAPINFOHEADER aInfoHeader = {};
            aInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
            aInfoHeader.biWidth = lWidth;
            aInfoHeader.biHeight = lHeight;
            aInfoHeader.biPlanes = 1;
            aInfoHeader.biBitCount = 24;
            aInfoHeader.biSizeImage = lDataSize;
            fwrite(&aInfoHeader, 1, sizeof(aInfoHeader), pFile);

            for (int y = lHeight - 1; y >= 0; --y)
            {
                for (int x = 0; x < lWidth; ++x)
                {
                    const uint32_t lPx = pPixels[y * lWidth + x];
                    const uint8_t aBgr[3] = { static_cast<uint8_t>(lPx & 0xFF), static_cast<uint8_t>((lPx >> 8) & 0xFF), static_cast<uint8_t>((lPx >> 16) & 0xFF) };
                    fwrite(aBgr, 1, 3, pFile);
                }
                const uint8_t aPad[4] = {};
                fwrite(aPad, 1, lRowSize - 3 * lWidth, pFile);
            }
            fclose(pFile);
        }
    }

    // PC 0x00489430 (Dump). Captures the front buffer and writes it as TGA/BMP/JPEG
    // depending on the file extension.
    void ZRenderWintelD3D::Dump(const char* pName)
    {
        D3DDISPLAYMODE sDisplayMode;
        if (g_pd3dInterface->GetAdapterDisplayMode(uActiveAdapter, &sDisplayMode) < 0)
            return;

        IDirect3DSurface9* pSurface = nullptr;
        if (g_pd3dDevice->CreateOffscreenPlainSurface(sDisplayMode.Width, sDisplayMode.Height, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pSurface, nullptr) < 0)
            return;

        m_pSwapChain->GetFrontBufferData(pSurface);

        D3DLOCKED_RECT sLockedRect;
        if (pSurface->LockRect(&sLockedRect, nullptr, 0) >= 0)
        {
            const uint32_t lWidth = sDisplayMode.Width;
            const uint32_t lHeight = sDisplayMode.Height;
            uint32_t* pPixels = static_cast<uint32_t*>(ZUniMemory::Allocate(sizeof(uint32_t) * lWidth * lHeight));
            for (uint32_t y = 0; y < lHeight; ++y)
                std::memcpy(pPixels + y * lWidth, static_cast<const uint8_t*>(sLockedRect.pBits) + y * sLockedRect.Pitch, lWidth * sizeof(uint32_t));
            pSurface->UnlockRect();

            const size_t lLen = std::strlen(pName);
            const char* pszExt = (lLen >= 4) ? pName + lLen - 4 : pName;
            if (!_memicmp(pszExt, ".tga", 4))
                WriteTGA(pName, pPixels, static_cast<int>(lWidth), static_cast<int>(lHeight));
            else if (!_memicmp(pszExt, ".bmp", 4))
                WriteBMP(pName, pPixels, static_cast<int>(lWidth), static_cast<int>(lHeight));
            else
            {
                // PC (0x00489430) / iOS WriteJPEG: encode through libjpeg.
                WriteJPEG(pName, pPixels, static_cast<int>(lWidth), static_cast<int>(lHeight));
            }

            ZUniMemory::Free(pPixels);
        }

        pSurface->Release();
    }

    // PC 0x0048ABB0. Draws an in-window progress bar (fPercent clamped to [0,1], re-rendered only
    // when it moved by at least 2%).
    void ZRenderWintelD3D::ProgressBar(const float fPercent, const unsigned int, const unsigned int)
    {
        static float fPrevPercent = 0.0f; // PC function-local static

        float fPct = fPercent;
        if (fPct < 0.0f)
            fPct = 0.0f;
        else if (fPct > 1.0f)
            fPct = 1.0f;

        if (std::fabs(fPrevPercent - fPct) < 0.02f)
            return;

        fPrevPercent = fPct;
        if (m_bInScene || !BeginScene())
            return;

        const ZVector4& rect = g_pSysInterface->m_v4ProgressBarRect;
        const float fSizeX = static_cast<float>(GetSizeX());
        const float fSizeY = static_cast<float>(GetSizeY());

        const float fLeft = fSizeX * rect.x;
        const float fBottom = fSizeY - (rect.w + rect.y) * fSizeY;
        const float fRight = (fPct * rect.z + rect.x) * fSizeX;
        const float fTop = fSizeY - fSizeY * rect.y;

        // PC sub_487E10 / sub_487E50: colour/alpha texture-stage helpers (SELECTARG1 of DIFFUSE/TEXTURE).
        g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
        g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
        g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);

        g_pd3dDevice->SetPixelShader(nullptr);
        ZDirect3DDevice::DrawPlane(g_pd3dDevice, fLeft, fBottom, fRight - fLeft, fTop - fBottom, 0xFFFF0000, 0.0f, 1.0f, 1.0f);
        EndScene();
        Flip();
    }

    // PC 0x00489C30
    unsigned int ZRenderWintelD3D::ShadowRenderLayerId()
    {
        if (ZSharedResourcesD3D::g_pInstance->m_eShadowMode == ZSharedResourcesD3D::SHADOWMODE_COLOR)
            return 2;
        ZASSERT(false); // PC: __debugbreak()
        return 0;
    }

    // PC 0x00489C50 (shared return-true implementation)
    bool ZRenderWintelD3D::ShadowRenderStart(unsigned int)
    {
        return true;
    }

    // PC 0x00489C60
    unsigned int ZRenderWintelD3D::NumShadowRenderCasterPasses(unsigned int)
    {
        if (ZSharedResourcesD3D::g_pInstance->m_eShadowMode)
            return 1;
        return 2 - ((m_field15B8 & 0x100) != 0);
    }

    // PC 0x005BD050
    unsigned int ZRenderWintelD3D::ShadowRenderCasterStart(unsigned int, unsigned int)
    {
        return 0;
    }

    // PC 0x00489F10. Stencil shadow-volume caster setup (render states decoded from assembly).
    void ZRenderWintelD3D::ShadowRenderCasters(unsigned int lCaster)
    {
        if (ZSharedResourcesD3D::g_pInstance->m_eShadowMode)
            return;

        g_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);

        if (m_field15B8 & 0x100)
        {
            if (!lCaster)
            {
                g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
                g_pd3dDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
                g_pd3dDevice->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
                g_pd3dDevice->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_INCR);
                g_pd3dDevice->SetRenderState(D3DRS_CCW_STENCILFAIL, D3DSTENCILOP_KEEP);
                g_pd3dDevice->SetRenderState(D3DRS_CCW_STENCILPASS, D3DSTENCILOP_KEEP);
                g_pd3dDevice->SetRenderState(D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_INCRSAT);
            }
        }
        else if (lCaster == 1)
        {
            g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
            g_pd3dDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
            g_pd3dDevice->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
            g_pd3dDevice->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_INCRSAT);
        }
        else
        {
            g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
            g_pd3dDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
            g_pd3dDevice->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
            g_pd3dDevice->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_INCR);
        }
    }

    // PC 0x0048A010. Stencil shadow-volume receiver setup (states decoded from assembly);
    // the color/depth shadow-map modes restore the main render targets.
    void ZRenderWintelD3D::ShadowRenderReceivers()
    {
        const auto lShadowMode = ZSharedResourcesD3D::g_pInstance->m_eShadowMode;
        if (lShadowMode)
        {
            g_pd3dDevice->SetRenderTarget(0, m_pSurface16A4);
            g_pd3dDevice->SetDepthStencilSurface(m_pSurface16A8);
            if (lShadowMode == ZSharedResourcesD3D::SHADOWMODE_COLOR)
            {
                if (g_bD3DViewportInited)
                {
                    g_bD3DViewportInited = false;
                    g_pd3dDevice->SetViewport(&g_sD3DViewport);
                    SetProjection(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, false);
                }
            }
            else
            {
                g_pd3dDevice->SetViewport(&g_sD3DViewport);
            }
            return;
        }

        g_pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 15);
        g_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_GREATEREQUAL);
        g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        g_pd3dDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_NOTEQUAL);
        g_pd3dDevice->SetRenderState(D3DRS_STENCILREF, 128);
        g_pd3dDevice->SetRenderState(D3DRS_STENCILWRITEMASK, 0);

        if (m_field15B8 & 0x100)
            g_pd3dDevice->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, FALSE);
    }

    // PC 0x0048A130 (stencil-mode shadow render teardown)
    void ZRenderWintelD3D::ShadowRenderEnd()
    {
        if (!ZSharedResourcesD3D::g_pInstance->m_eShadowMode)
        {
            g_pd3dDevice->SetRenderState(D3DRS_STENCILENABLE, 0);
            g_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, 1);
        }
    }

    // PC 0x00488E20. Sets the scissor rectangle and enables/disables the scissor test.
    void ZRenderWintelD3D::SetScissorRect(float fLeft, float fRight, float fBottom, float fTop, bool bSetOrReset)
    {
        if (bSetOrReset)
        {
            RECT rect;
            rect.left = static_cast<LONG>(fLeft);
            rect.right = static_cast<LONG>(fRight);
            rect.bottom = static_cast<LONG>(fBottom);
            rect.top = static_cast<LONG>(fTop);
            g_pd3dDevice->SetScissorRect(&rect);
            g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
        }
        else
        {
            g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        }
    }

    // PC 0x00489810 (stencil-based mirror render setup)
    bool ZRenderWintelD3D::MirrorRenderBegin()
    {
        if (!field_16AE)
            return false;

        g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_STENCIL, 0xFF, 1.0f, 0);
        g_pd3dDevice->SetRenderState(D3DRS_STENCILENABLE, TRUE);
        g_pd3dDevice->SetRenderState(D3DRS_STENCILMASK, 0xFFFFFFFF);
        g_pd3dDevice->SetRenderState(D3DRS_STENCILWRITEMASK, 0xFFFFFFFF);
        g_pd3dDevice->SetRenderState(D3DRS_STENCILREF, 1);
        return true;
    }

    // PC 0x00489880 (mirror reflector setup, states decoded from assembly)
    void ZRenderWintelD3D::MirrorRenderReflectors()
    {
        g_pd3dDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS); // state 0x38 = 8
        g_pd3dDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE); // state 0x37 = 3
        g_pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0); // state 0xA8
    }

    // PC 0x004898C0 (mirror reflected render setup, states decoded from assembly)
    void ZRenderWintelD3D::MirrorRenderReflected()
    {
        g_pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 7);  // state 0xA8
        g_pd3dDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_NOTEQUAL); // state 0x38 = 3
        g_pd3dDevice->SetRenderState(D3DRS_STENCILWRITEMASK, 0);  // state 0x3B
    }

    // PC 0x0048AD40 (slot 108). Draws a fullscreen quad filled with the fog colour over the
    // mirror's z-hole; render states decoded from assembly.
    bool ZRenderWintelD3D::MirrorRenderZHole()
    {
        struct SVertexXYZRHWColor
        {
            float x;
            float y;
            float z;
            float rhw;
            D3DCOLOR color;
        };

        const float fSizeX = static_cast<float>(GetSizeX());
        const float fSizeY = static_cast<float>(GetSizeY());

        SVertexXYZRHWColor aVertices[4] = {
            { -0.5f,            -0.5f,            1.0f, 1.0f, g_dwFogColor },
            { -0.5f,            fSizeY - 0.5f,     1.0f, 1.0f, g_dwFogColor },
            { fSizeX - 0.5f,    -0.5f,            1.0f, 1.0f, g_dwFogColor },
            { fSizeX - 0.5f,    fSizeY - 0.5f,     1.0f, 1.0f, g_dwFogColor },
        };

        g_pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
        g_pd3dDevice->SetPixelShader(nullptr);
        g_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS); // state 0x17 = 8
        g_pd3dDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
        g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
        g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, aVertices, sizeof(SVertexXYZRHWColor));

        return true;
    }

    // PC 0x00489900 (mirror render teardown, states decoded from assembly)
    void ZRenderWintelD3D::MirrorRenderEnd()
    {
        g_pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 7); // state 0xA8
        g_pd3dDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE); // state 0x34
    }

    // PC 0x004899B0. Locks the offscreen texture's level-0 rect; reports the pitch through the
    // output parameters (the exact output layout follows the D3DSURFACE_DESC of the texture).
    void* ZRenderWintelD3D::LockOffscreenSurface(unsigned int* pWidth, int* pHeight, int* pPitch, SZVRECT*)
    {
        D3DSURFACE_DESC sDesc;
        m_pBinkVideoTextureD3D->GetLevelDesc(0, &sDesc);
        D3DLOCKED_RECT sLockedRect;
        m_pBinkVideoTextureD3D->LockRect(0, &sLockedRect, nullptr, 0);

        if (pWidth)
            *pWidth = sDesc.Width;
        if (pHeight)
            *pHeight = sDesc.Height;
        if (pPitch)
            *pPitch = sLockedRect.Pitch;
        return sLockedRect.pBits;
    }

    // PC 0x00489A10
    void ZRenderWintelD3D::UnlockOffscreenSurface(void*)
    {
        m_pBinkVideoTextureD3D->UnlockRect(0);
    }

    // PC 0x00489A30. Draws the offscreen surface as a centered, aspect-corrected textured quad.
    void ZRenderWintelD3D::BlitOffscreenSurface()
    {
        D3DSURFACE_DESC sDesc;
        m_pBinkVideoTextureD3D->GetLevelDesc(0, &sDesc);

        const float fAspectRatio = static_cast<float>(sDesc.Height) / static_cast<float>(sDesc.Width);
        const float fSizeX = static_cast<float>(GetSizeX());
        const float fSizeY = fSizeX * fAspectRatio;
        const float fOffsetY = (static_cast<float>(GetSizeY()) - fSizeY) * 0.5f;

        g_pd3dDevice->SetPixelShader(nullptr);
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetTexture(0, m_pBinkVideoTextureD3D);
        g_pd3dDevice->SetTexture(1, nullptr);
        g_pd3dDevice->SetTexture(2, nullptr);
        g_pd3dDevice->SetTexture(3, nullptr);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP);
        g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
        g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
        g_pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
        g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAXMIPLEVEL, 0);
        g_pd3dDevice->SetSamplerState(0, D3DSAMP_MIPMAPLODBIAS, 0);
        ZDirect3DDevice::DrawPlane(g_pd3dDevice, 0.0f, fOffsetY, fSizeX, fSizeY, 0xFFFFFFFF, 0.0f, 1.0f, 1.0f);
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
    }

    // PC 0x00489930. Creates the offscreen render-target texture of the given size.
    bool ZRenderWintelD3D::CreateOffscreenSurface(int iWidth, int iHeight)
    {
        m_iOffscreenSurfaceWidth = iWidth;
        m_iOffscreenSurfaceHeight = iHeight;

        g_pd3dDevice->CreateTexture(
            iWidth,
            iHeight,
            1u,
            D3DUSAGE_RENDERTARGET,
            D3DFMT_X8R8G8B8,
            D3DPOOL_DEFAULT,
            &m_pBinkVideoTextureD3D,
            nullptr);
        return true;
    }

    // PC 0x00489970
    void ZRenderWintelD3D::DestroyOffscreenSurface()
    {
        if (m_pBinkVideoTextureD3D)
        {
            m_pBinkVideoTextureD3D->Release();
            m_pBinkVideoTextureD3D = nullptr;
        }
        m_pOffscreenSurface = 0;
    }

    // PC slot 156 (stub)
    void ZRenderWintelD3D::OnCopyData()
    {
        // Do nothing
    }

    // PC 0x00489C90. Stencil shadow-volume caster start (states decoded from assembly); the
    // colour/depth shadow-map modes bind the shadow-map render targets instead.
    unsigned int ZRenderWintelD3D::ShadowRenderCasterStart(unsigned int lCaster, unsigned int lPass, float fUnknown)
    {
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

        const auto lShadowMode = ZSharedResourcesD3D::g_pInstance->m_eShadowMode;
        if (lShadowMode)
        {
            if (lShadowMode == ZSharedResourcesD3D::SHADOWMODE_COLOR)
            {
                // Clear the texture stages, bind the shadow colour map surface + depth buffers.
                for (uint32_t i = 0; i < g_dwTextureUnits; ++i)
                    g_pd3dDevice->SetTexture(i, nullptr);

                IDirect3DSurface9* pSurface = nullptr;
                auto* pColorMap = reinterpret_cast<IDirect3DTexture9*>(g_texShadowMapColor[lPass * 17 % 4].m_pUserData);
                pColorMap->GetSurfaceLevel(0, &pSurface);

                if (!g_bD3DViewportInited)
                {
                    g_bD3DViewportInited = true;
                    g_pd3dDevice->GetViewport(&g_sD3DViewport);
                }
                g_pd3dDevice->SetRenderTarget(0, pSurface);
                pSurface->Release();
                g_pd3dDevice->SetDepthStencilSurface(g_pShadowMapDepthBuffers);
                g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xFFFFFFFF, 1.0f, 0);

                if (fUnknown != 0.0f)
                {
                    // PC (0x00489C90) vtbl[412] = slot 103 = SetScissorRect: a near-fullscreen scissor
                    // inset by 1 pixel, limiting the shadow-map rendering to the square area.
                    const float fSize = static_cast<float>(ZSharedResourcesD3D::g_pInstance->m_iShaderResolution - 2);
                    SetScissorRect(1.0f, fSize, fSize, 1.0f, true);
                }
                return lPass;
            }
            if (lShadowMode == ZSharedResourcesD3D::SHADOWMODE_DEPTH)
            {
                IDirect3DSurface9* pSurface = nullptr;
                auto* pColorMap = reinterpret_cast<IDirect3DTexture9*>(g_texShadowMapColor[0].m_pUserData);
                pColorMap->GetSurfaceLevel(0, &pSurface);
                g_pd3dDevice->SetRenderTarget(0, pSurface);
                pSurface->Release();

                IDirect3DSurface9* pDepthSurface = nullptr;
                auto* pDepthMap = reinterpret_cast<IDirect3DTexture9*>(g_texShadowDepth.m_pUserData);
                pDepthMap->GetSurfaceLevel(0, &pDepthSurface);
                g_pd3dDevice->SetDepthStencilSurface(pDepthSurface);
                pDepthSurface->Release();

                D3DVIEWPORT9 viewport = { 0, 0, static_cast<DWORD>(ZSharedResourcesD3D::g_pInstance->m_iShaderResolution), static_cast<DWORD>(ZSharedResourcesD3D::g_pInstance->m_iShaderResolution), 0.0f, 1.0f };
                g_pd3dDevice->SetViewport(&viewport);
                g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xFFFFFFFF, 1.0f, 0);
                return lPass;
            }
            ZASSERT(false); // PC: __debugbreak()
            return 0;
        }

        g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_STENCIL, 0xFF, 1.0f, 0x80);

        if (!g_lShowShadowVolumes)
            g_pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0); // state 0xA8
        g_pd3dDevice->SetRenderState(D3DRS_STENCILENABLE, TRUE); // state 0x34
        g_pd3dDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS); // state 0x38 = 8
        g_pd3dDevice->SetRenderState(D3DRS_STENCILMASK, 0xFFFFFFFF); // state 0x3A
        g_pd3dDevice->SetRenderState(D3DRS_STENCILWRITEMASK, 0xFFFFFFFF); // state 0x3B

        if (m_field15B8 & 0x100)
        {
            g_pd3dDevice->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, TRUE); // state 0xB9
            g_pd3dDevice->SetRenderState(D3DRS_CCW_STENCILFUNC, D3DCMP_ALWAYS); // state 0xBD
        }

        return 4;
    }

    // PC 0x0048AB70. Re-allocates the shared resources and material buffer after a device reset,
    // and rebuilds the offscreen surface if it was in use.
    void ZRenderWintelD3D::sub_48AB70()
    {
        ZSharedResourcesD3D::g_pInstance->Allocate();
        g_pRenderDll->m_pMaterialBuffer->AllocateResources();

        if (m_pOffscreenSurface)
            CreateOffscreenSurface(m_iOffscreenSurfaceWidth, m_iOffscreenSurfaceHeight);
    }

    // PC 0x00489670. Plots the free-list of an offset allocator (used by the buffer-allocator
    // stats overlay); each free range is validated against the total size.
    void ZRenderWintelD3D::PlotOffsetAlloc(int, int, const char*, ZOffsetAlloc* pAllocator, uint32_t lTotalSize)
    {
        const uint32_t lScale = lTotalSize >> 6;

        REFTAB refs(128, 1); // REFTAB::REFTAB(&refs, 128, 1)

        pAllocator->GetFreeList(&refs);

        RefRun run;
        refs.RunInitNxtRef(&run);
        for (const uint32_t* pRef = refs.RunNxtRefPtr(&run); pRef; pRef = refs.RunNxtRefPtr(&run))
        {
            const uint32_t lOffset = pRef[0];
            const uint32_t lSize = pRef[1] / lScale;
            uint32_t lX = lOffset / lScale;
            if (!lX)
                lX = (lTotalSize >> 7) < lOffset;

            ZASSERT(lSize + lX <= 0x40); // PC: if (lSize + lX > 0x40) __debugbreak();
        }
    }
}
