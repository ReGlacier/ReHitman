#include <Glacier/Render/ZRenderWintelD3DDll.h>
#include <Glacier/Render/Bitmap/ZBitmap32.h>
#include <Glacier/Render/Material/ZRenderMaterialBufferD3D.h>
#include <Glacier/Render/Prim/ZPrimControlWintel.h>
#include <Glacier/Render/ZRender.h>
#include <Glacier/Render/ZDirect3DDevice.h>
#include <Glacier/Render/ZRenderWintelD3D.h>
#include <Glacier/Render/ZSharedResourcesD3D.h>
#include <Glacier/Render/ZTextureManagerD3D.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniMemory.h>
#include <cstring>


namespace Glacier
{
    // PC 0x00487DE0; iOS 0x100269768.
    ZRenderBaseDll* CreateD3DDll()
    {
        auto* pRenderDll = ZUniMemory::New<ZRenderWintelD3DDll>();
        if (!pRenderDll)
            return nullptr;

        static constexpr char sRenderDllName[] = "RenderD3D.dll";
        pRenderDll->m_pName = static_cast<char*>(ZUniMemory::Allocate(sizeof(sRenderDllName)));
        std::memcpy(pRenderDll->m_pName, sRenderDllName, sizeof(sRenderDllName));
        return pRenderDll;
    }

    ZRenderWintelD3DDll::ZRenderWintelD3DDll() = default;

    // PC 0x00487900; iOS 0x1002697E8.
    ZRenderWintelD3DDll::~ZRenderWintelD3DDll() = default;

    // PC 0x00487910; iOS 0x100269800.
    void ZRenderWintelD3DDll::Init()
    {
        ZRenderBaseDll::Init();
        SetTextureLevel(1);
        m_pMaterialBuffer = ZUniMemory::New<ZRenderMaterialBufferD3D>();
    }

    // PC 0x00487990; iOS 0x10026985C.
    void ZRenderWintelD3DDll::End()
    {
        if (m_pTexCon)
            ZUniMemory::Delete(m_pTexCon);

        if (ZSharedResourcesD3D::g_pInstance)
            ZSharedResourcesD3D::Release();

        D3D_SAFE_RELEASE(g_pd3dDevice);
        D3D_SAFE_RELEASE(g_pd3dInterface);
    }

    // PC 0x004879E0; iOS 0x1002698DC.
    ZRender* ZRenderWintelD3DDll::SetupWindow(void* pWindow)
    {
        auto* pRender = ZUniMemory::New<ZRenderWintelD3D>(
            reinterpret_cast<int>(g_pSysInterface->m_hInstance), pWindow);
        pRender->MakeMe(static_cast<HWND>(pWindow));
        return pRender;
    }

    // PC 0x00487A60; iOS 0x100269960.
    void ZRenderWintelD3DDll::CreatePrimControl()
    {
        if (!m_pPrimControl)
            m_pPrimControl = ZUniMemory::New<ZPrimControlWintel>(false);
    }

    // PC 0x00487AE0; iOS 0x1002699BC.
    void ZRenderWintelD3DDll::InstallPrimBuffer(void* pPrimBuffer, uint32_t lPrimBufferSize)
    {
        InitPrimPack();
        CreatePrimControl();

        if (g_pSysInterface->WindowFirst)
            g_pSysInterface->WindowFirst->FreeDrawBuffers();

        ZRenderBaseDll::InstallPrimBuffer(pPrimBuffer, lPrimBufferSize);
    }

    // PC 0x00487B80; iOS 0x100269A54.
    void ZRenderWintelD3DDll::InstallTextureBuffer(void* pTextureBuffer, uint32_t lTextureBufferSize)
    {
        ZRenderBaseDll::InstallTextureBuffer(pTextureBuffer, lTextureBufferSize);
        if (g_pd3dDevice)
        {
            static_cast<ZTextureManagerD3D*>(m_pTexCon)->InstallTextureBuffer(pTextureBuffer);
            ZUniMemory::Free(m_pTextureBuffer);
            m_pTextureBuffer = nullptr;
        }
    }

    // PC 0x00487BD0; iOS 0x100269ABC.
    void ZRenderWintelD3DDll::InstallMaterialBuffer(
        void* pBuffer, uint32_t lBufferSize, uint32_t lAllocatedBufferSize)
    {
        ZRenderBaseDll::InstallMaterialBuffer(pBuffer, lBufferSize, lAllocatedBufferSize);
        if (g_pd3dDevice)
        {
            m_pMaterialBuffer->FreeResources();
            m_pMaterialBuffer->AllocateResources();
        }
    }

    // PC 0x00487CE0; iOS 0x100269B20.
    uint32_t ZRenderWintelD3DDll::ReserveTexture(uint32_t lWidth, uint32_t lHeight)
    {
        auto* pTextureManager = static_cast<ZTextureManagerD3D*>(m_pTexCon);
        const uint32_t lTextureId = pTextureManager->ReserveTexture();

        uint32_t aPixels[256 * 256];
        for (uint32_t y = 0; y < 256; ++y)
        {
            for (uint32_t x = 0; x < 256; ++x)
                aPixels[y * 256 + x] = ((x ^ y) & 1) ? 0xFF00FF00u : 0xFFFF0000u;
        }

        ZBitmap32 bitmap(aPixels, lWidth, lHeight, 0xFF0000, 0xFF00, 0xFF, 0xFF000000, false);
        bitmap.SetId(lTextureId);
        bitmap.SetName("runtime");
        pTextureManager->CreateTexture(&bitmap, pTextureManager->GetTexture(lTextureId, 0));
        return lTextureId;
    }

    // PC 0x00487C10; iOS 0x10026A580.
    void ZRenderWintelD3DDll::UpdateTexture(uint32_t lTextureId, const ZBitmap* pBitmap)
    {
        auto* pTextureManager = static_cast<ZTextureManagerD3D*>(m_pTexCon);
        pTextureManager->UpdateTexture(pBitmap, pTextureManager->GetTexture(lTextureId, 0));
    }
}
