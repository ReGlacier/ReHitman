/*
 * TexCapture (nvCPL.dll drop-in) - dumps the procedural light/phase textures
 * that ZSharedResourcesD3D::Allocate creates in Hitman Blood Money (PC).
 *
 * Auto-load: the game delay-loads nvCPL.dll and calls NvCplGetDataInt at
 * startup, so placing this DLL next to HitmanBloodMoney.exe is enough.
 *
 * Approach mirrors ReHitman/BloodMoney ZDirect3D9DevicePatches: nothing D3D
 * runs on a helper thread. A setup thread only watches the renderer globals
 * until the D3D device exists, then swaps the Present / GetSwapChain vtable
 * slots of the device so the actual capture happens on the render thread.
 * Press F5 in the menu / level and the maps are LockRect()-ed and written as
 * raw 32-bit RGBA into <game dir>\ref_data\.
 *
 * Build for x86 (Win32). Requires the DirectX SDK (d3d9.h).
 */

#include <d3d9.h>
#include <windows.h>

#include <cstdarg>
#include <cstdint>
#include <cstdio>

#pragma comment(lib, "d3d9.lib")

namespace
{
    constexpr uintptr_t kImageBase = 0x00400000;
    uintptr_t g_ModBase = kImageBase;

    // Renderer globals (absolute VAs of the PC build).
    constexpr uintptr_t kGpD3DDevice = 0x0090AF0C;          // ZDirect3DDevice*
    constexpr uintptr_t kCubeNormalizer = 0x0090ADAC;       // IDirect3DCubeTexture9* (128)
    constexpr uintptr_t kMapAnisotropic = 0x0090AD64;       // IDirect3DTexture9* (128)
    constexpr uintptr_t kMapSpecular = 0x0090AD1C;
    constexpr uintptr_t kMapScatter = 0x0090ACD4;
    constexpr uintptr_t kMapHgPhase = 0x0090AC8C;
    constexpr uintptr_t kMapRefraction = 0x0090AC44;
    constexpr uintptr_t kSpotMap = 0x0090AA14;              // slot array, handle @ elem+0, stride 0x44
    constexpr uintptr_t kSpotMap2 = 0x0090A98C;
    constexpr uintptr_t kTexWhite = 0x0090ADC8;             // wrappers; object @ m_pUserData (+0x2C)
    constexpr uintptr_t kTexMouse = 0x0090AE10;
    constexpr uintptr_t kTexShadowClip = 0x0090AE58;        // holds ShadowClip1x2
    constexpr uintptr_t kTexFont = 0x0090AEA0;              // holds Font8x13

    constexpr const char* kOutDir = "ref_data";

    constexpr size_t kVtblGetSwapChain = 14;
    constexpr size_t kVtblDevicePresent = 17;
    constexpr size_t kVtblSwapChainPresent = 3;

    // IDirect3DDevice9 vtable indices are stable for D3D9; do not rely on any
    // engine wrapper layout besides ZDirect3DDevice::m_pDevice == +0x4.

    uintptr_t Addr(uintptr_t aAbsVa) { return g_ModBase + (aAbsVa - kImageBase); }

    // ------------------------------------------------------------------
    // Small COM vtbl patching helpers
    // ------------------------------------------------------------------

    bool PatchSlot(void** pVtblSlot, void* pReplacement, void** ppOriginal)
    {
        if (!*ppOriginal)
        {
            *ppOriginal = *pVtblSlot;
        }
        DWORD dwOldProtect = 0;
        if (!VirtualProtect(pVtblSlot, sizeof(void*), PAGE_EXECUTE_READWRITE, &dwOldProtect))
        {
            return false;
        }
        *pVtblSlot = pReplacement;
        VirtualProtect(pVtblSlot, sizeof(void*), dwOldProtect, &dwOldProtect);
        return true;
    }

    // ------------------------------------------------------------------
    // Capture
    // ------------------------------------------------------------------

    struct OriginalPresent
    {
        typedef HRESULT(WINAPI* DevicePresentFn)(IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*);
        typedef HRESULT(WINAPI* GetSwapChainFn)(IDirect3DDevice9*, UINT, IDirect3DSwapChain9**);
        typedef HRESULT(WINAPI* SwapChainPresentFn)(IDirect3DSwapChain9*, const RECT*, const RECT*, HWND, const RGNDATA*, DWORD);
        static void* devicePresent;
        static void* deviceGetSwapChain;
        static void* swapChainPresent;
    };
    void* OriginalPresent::devicePresent = nullptr;
    void* OriginalPresent::deviceGetSwapChain = nullptr;
    void* OriginalPresent::swapChainPresent = nullptr;

    bool WriteAll(HANDLE hFile, const void* pData, DWORD nSize)
    {
        DWORD nWritten = 0;
        return hFile && WriteFile(hFile, pData, nSize, &nWritten, nullptr) && nWritten == nSize;
    }

    FILE* OpenInfo()
    {
        char szPath[MAX_PATH];
        sprintf_s(szPath, "%s\\info.txt", kOutDir);
        return fopen(szPath, "w");
    }

    void LogMsg(const char* pszFmt, ...)
    {
        CreateDirectoryA(kOutDir, nullptr);
        char szPath[MAX_PATH];
        sprintf_s(szPath, "%s\\_log.txt", kOutDir);
        FILE* f = fopen(szPath, "a");
        if (!f)
        {
            return;
        }
        va_list args;
        va_start(args, pszFmt);
        fprintf(f, "[%lu] ", GetTickCount());
        vfprintf(f, pszFmt, args);
        fputc('\n', f);
        fclose(f);
        va_end(args);
    }

    bool Dump2D(IDirect3DTexture9* pTex, const char* pszName, FILE* pInfo)
    {
        if (!pTex)
        {
            return false;
        }
        D3DSURFACE_DESC Desc;
        if (FAILED(pTex->GetLevelDesc(0, &Desc)))
        {
            return false;
        }
        D3DLOCKED_RECT Locked;
        if (FAILED(pTex->LockRect(0, &Locked, nullptr, D3DLOCK_READONLY)))
        {
            return false;
        }

        char szPath[MAX_PATH];
        sprintf_s(szPath, "%s\\%s.raw", kOutDir, pszName);
        HANDLE hFile = CreateFileA(szPath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        const auto* pBits = static_cast<const uint8_t*>(Locked.pBits);
        bool bOk = hFile != INVALID_HANDLE_VALUE;
        for (DWORD y = 0; bOk && y < Desc.Height; ++y)
        {
            bOk = WriteAll(hFile, pBits + static_cast<size_t>(y) * Locked.Pitch, Desc.Width * 4);
        }
        if (hFile != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hFile);
        }
        pTex->UnlockRect(0);
        if (bOk && pInfo)
        {
            fprintf(pInfo, "%s %u %u\n", pszName, Desc.Width, Desc.Height);
        }
        return bOk;
    }

    void DumpCube(IDirect3DCubeTexture9* pCube, const char* pszName, FILE* pInfo)
    {
        if (!pCube)
        {
            return;
        }
        for (DWORD lFace = 0; lFace < 6; ++lFace)
        {
            D3DLOCKED_RECT Locked;
            if (FAILED(pCube->LockRect(static_cast<D3DCUBEMAP_FACES>(lFace), 0, &Locked, nullptr, D3DLOCK_READONLY)))
            {
                continue;
            }
            char szName[64];
            sprintf_s(szName, "%s_face%u", pszName, lFace);
            char szPath[MAX_PATH];
            sprintf_s(szPath, "%s\\%s.raw", kOutDir, szName);
            HANDLE hFile = CreateFileA(szPath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
            bool bOk = hFile != INVALID_HANDLE_VALUE;
            const auto* pBits = static_cast<const uint8_t*>(Locked.pBits);
            for (DWORD y = 0; bOk && y < 128; ++y)
            {
                bOk = WriteAll(hFile, pBits + static_cast<size_t>(y) * Locked.Pitch, 128 * 4);
            }
            if (hFile != INVALID_HANDLE_VALUE)
            {
                CloseHandle(hFile);
            }
            pCube->UnlockRect(static_cast<D3DCUBEMAP_FACES>(lFace), 0);
            if (bOk && pInfo)
            {
                fprintf(pInfo, "%s %u %u\n", szName, 128u, 128u);
            }
        }
    }

    template <typename T>
    T* ReadPtr(uintptr_t aAbsVa)
    {
        return *reinterpret_cast<T**>(Addr(aAbsVa));
    }

    void CaptureOnce()
    {
        CreateDirectoryA(kOutDir, nullptr);
        LogMsg("capture started");
        if (FILE* pInfo = OpenInfo())
        {
            DumpCube(ReadPtr<IDirect3DCubeTexture9>(kCubeNormalizer), "mapCubeNormalizer", pInfo);
            Dump2D(ReadPtr<IDirect3DTexture9>(kMapAnisotropic), "mapAnisotropic", pInfo);
            Dump2D(ReadPtr<IDirect3DTexture9>(kMapSpecular), "mapSpecular", pInfo);
            Dump2D(ReadPtr<IDirect3DTexture9>(kMapScatter), "mapScatter", pInfo);
            Dump2D(ReadPtr<IDirect3DTexture9>(kMapHgPhase), "mapHgPhase", pInfo);
            Dump2D(ReadPtr<IDirect3DTexture9>(kMapRefraction), "mapRefraction", pInfo);

            for (DWORD i = 0; i < 2; ++i)
            {
                char szName[64];
                IDirect3DTexture9* pTex = *reinterpret_cast<IDirect3DTexture9**>(Addr(kSpotMap) + static_cast<size_t>(i) * 0x44);
                sprintf_s(szName, "spotAttenMap0_r%u", i);
                Dump2D(pTex, szName, pInfo);

                pTex = *reinterpret_cast<IDirect3DTexture9**>(Addr(kSpotMap2) + static_cast<size_t>(i) * 0x44);
                sprintf_s(szName, "spotAttenMap1_r%u", i);
                Dump2D(pTex, szName, pInfo);
            }

            Dump2D(*reinterpret_cast<IDirect3DTexture9**>(Addr(kTexWhite) + 0x2C), "texWhite1x1", pInfo);
            Dump2D(*reinterpret_cast<IDirect3DTexture9**>(Addr(kTexMouse) + 0x2C), "texMouse16x16", pInfo);
            Dump2D(*reinterpret_cast<IDirect3DTexture9**>(Addr(kTexShadowClip) + 0x2C), "texShadowClip1x2", pInfo);
            Dump2D(*reinterpret_cast<IDirect3DTexture9**>(Addr(kTexFont) + 0x2C), "texFont8x13", pInfo);

            fclose(pInfo);
        }
        LogMsg("capture finished");
        OutputDebugStringA("TexCapture: written to ref_data\\\n");
    }

    // ------------------------------------------------------------------
    // Present / GetSwapChain callbacks (render thread)
    // ------------------------------------------------------------------

    HRESULT WINAPI OnSwapChainPresent(IDirect3DSwapChain9* pSwapChain,
                                      const RECT* pSrc,
                                      const RECT* pDst,
                                      HWND hOverride,
                                      const RGNDATA* pDirty,
                                      DWORD dwFlags);

    HRESULT WINAPI OnDevicePresent(IDirect3DDevice9* pDevice,
                                   const RECT* pSrc,
                                   const RECT* pDst,
                                   HWND hOverride,
                                   const RGNDATA* pDirty)
    {
        static bool s_bLogged = false;
        static DWORD s_dwLastCapture = 0;
        if (!s_bLogged)
        {
            s_bLogged = true;
            LogMsg("present hook fired (device=%p)", reinterpret_cast<void*>(pDevice));
        }
        if (GetAsyncKeyState(VK_F5) & 0x8000)
        {
            const DWORD dwNow = GetTickCount();
            if (dwNow - s_dwLastCapture > 1500)
            {
                s_dwLastCapture = dwNow;
                LogMsg("F5 detected");
                CaptureOnce();
            }
        }
        return reinterpret_cast<OriginalPresent::DevicePresentFn>(OriginalPresent::devicePresent)(pDevice, pSrc, pDst, hOverride, pDirty);
    }

    HRESULT WINAPI OnDeviceGetSwapChain(IDirect3DDevice9* pDevice, UINT iSwapChain, IDirect3DSwapChain9** ppSwapChain)
    {
        const HRESULT hResult = reinterpret_cast<OriginalPresent::GetSwapChainFn>(OriginalPresent::deviceGetSwapChain)(pDevice, iSwapChain, ppSwapChain);
        if (SUCCEEDED(hResult) && *ppSwapChain)
        {
            if (!OriginalPresent::swapChainPresent)
            {
                void** pVtbl = *reinterpret_cast<void***>(*ppSwapChain);
                PatchSlot(&pVtbl[kVtblSwapChainPresent], reinterpret_cast<void*>(&OnSwapChainPresent), &OriginalPresent::swapChainPresent);
                LogMsg("swapchain %p Present hooked (slot %d)", reinterpret_cast<void*>(*ppSwapChain), (int)kVtblSwapChainPresent);
            }
        }
        return hResult;
    }

    HRESULT WINAPI OnSwapChainPresent(IDirect3DSwapChain9* pSwapChain,
                                      const RECT* pSrc,
                                      const RECT* pDst,
                                      HWND hOverride,
                                      const RGNDATA* pDirty,
                                      DWORD dwFlags)
    {
        static bool s_bLogged = false;
        if (!s_bLogged)
        {
            s_bLogged = true;
            LogMsg("swapchain present fired (%p)", reinterpret_cast<void*>(pSwapChain));
        }
        IDirect3DDevice9* pDevice = nullptr;
        if (SUCCEEDED(pSwapChain->GetDevice(&pDevice)) && pDevice)
        {
            return OnDevicePresent(pDevice, pSrc, pDst, hOverride, pDirty);
        }
        return reinterpret_cast<OriginalPresent::SwapChainPresentFn>(OriginalPresent::swapChainPresent)(pSwapChain, pSrc, pDst, hOverride, pDirty, dwFlags);
    }

    void InstallDeviceHooks(IDirect3DDevice9* pDevice)
    {
        void** pVtbl = *reinterpret_cast<void***>(pDevice);
        PatchSlot(&pVtbl[kVtblDevicePresent], reinterpret_cast<void*>(&OnDevicePresent), &OriginalPresent::devicePresent);
        PatchSlot(&pVtbl[kVtblGetSwapChain], reinterpret_cast<void*>(&OnDeviceGetSwapChain), &OriginalPresent::deviceGetSwapChain);
        LogMsg("hooks installed on device %p (vtbl=%p)", reinterpret_cast<void*>(pDevice), reinterpret_cast<void*>(pVtbl));
        OutputDebugStringA("TexCapture: D3D Present hooks installed\n");

        // The game presents through the implicit swap chain (device Present is
        // not called), and it may have fetched the swap chain before we hooked
        // GetSwapChain. Grab it ourselves so OnDeviceGetSwapChain patches its
        // Present slot too.
        IDirect3DSwapChain9* pSwapChain = nullptr;
        if (SUCCEEDED(pDevice->GetSwapChain(0, &pSwapChain)) && pSwapChain)
        {
            LogMsg("implicit swapchain acquired (%p)", reinterpret_cast<void*>(pSwapChain));
            pSwapChain->Release();
        }
        else
        {
            LogMsg("GetSwapChain(0) failed");
        }
    }

    DWORD WINAPI SetupThread(LPVOID)
    {
        Sleep(1000);
        int nTries = 0;
        for (;;)
        {
            g_ModBase = reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr));

            // ZDirect3DDevice* wrapper global; IDirect3DDevice9* = wrapper->m_pDevice (+0x4).
            void* pWrapper = ReadPtr<void>(kGpD3DDevice);
            if (pWrapper)
            {
                IDirect3DDevice9* pDevice = *reinterpret_cast<IDirect3DDevice9**>(static_cast<uint8_t*>(pWrapper) + 0x4);
                if (pDevice)
                {
                    InstallDeviceHooks(pDevice);
                    return 0; // done; the rest happens on the render thread
                }
                if ((++nTries & 3) == 1)
                {
                    LogMsg("wrapper=%p but device still null", pWrapper);
                }
            }
            else if ((++nTries & 3) == 1)
            {
                LogMsg("waiting for g_pd3dDevice...");
            }
            Sleep(250);
        }
        return 0;
    }
}

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hInstance);
        // Keep nvCPL.dll resident: the game calls FreeLibrary on the delay
        // loaded dll right after resolving the export (same fix as the working
        // ReHitman/NVCPL injector at 0x004884D9), which would otherwise leave
        // our setup thread running in unmapped code -> crash.
        constexpr uintptr_t kFreeLibraryCodeAddr = 0x004884D9;
        BYTE patch[7] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(kFreeLibraryCodeAddr), patch, sizeof(patch), nullptr);
        CreateThread(nullptr, 0, SetupThread, nullptr, 0, nullptr);
    }
    return TRUE;
}

//
// Exported symbol the game resolves on startup (same trick as ReHitman/NVCPL).
//
extern "C" __declspec(dllexport) bool __cdecl NvCplGetDataInt(int, int*)
{
    return false;
}
