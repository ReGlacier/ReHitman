#include <BloodMoney/Patches/All/ZDirect3D9DevicePatches.h>
#include <BloodMoney/Delegates/IDirect3DDelegate.h>
#include <BloodMoney/Engine/ZDirect3DDevice.h>

#include <spdlog/spdlog.h>

namespace Consts
{
    enum DX9DeviceAPIVFTableIndex : size_t
    {
        //BeginScene = 41,
        //EndScene = 42,
        //CreateAdditionalSwapChain = 13,
        GetSwapChain = 14,
        Reset = 16,
        DevicePresent = 17,
        SetTexture = 65,
        DrawIndexedPrimitive = 82
    };

    enum DX9SwapChainAPIVFTableIndex : size_t
    {
        Present = 3
    };

    static constexpr std::intptr_t kZDirect3DDeviceConstructor = 0x0049091C;
}

namespace Globals
{
    static std::unique_ptr<Hitman::BloodMoney::IDirect3DDelegate> g_pDelegate = nullptr;

    static void SetupD3DHooks(IDirect3DDevice9* device);
}

namespace Original
{
    typedef HRESULT(__stdcall* D3DReset_t)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
    typedef HRESULT(__stdcall* D3DPresent_t)(IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*);
    typedef HRESULT(__stdcall* D3D9SwapChain_Present_t)(IDirect3DSwapChain9*, const RECT*, const RECT*, HWND, const RGNDATA*, DWORD);
    typedef HRESULT(__stdcall* D3D9GetSwapChain_t)(IDirect3DDevice9*, UINT, IDirect3DSwapChain9**);

    D3DReset_t					    originalResetFunc;
    D3DPresent_t                    originalPresent;
    D3D9SwapChain_Present_t         originalSwapChainPresent;
    D3D9GetSwapChain_t              originalGetSwapChain;
}

namespace Globals {
    static std::unique_ptr<HF::Hook::VFHook<IDirect3DDevice9>> g_Direct3DDevice_Reset_Hook{nullptr};
    static std::unique_ptr<HF::Hook::VFHook<IDirect3DDevice9>> g_Direct3DDevice_Present_Hook{nullptr};
    static std::unique_ptr<HF::Hook::VFHook<IDirect3DDevice9>> g_Direct3DDevice_GetSwapChain_Hook{nullptr};
    static std::unique_ptr<HF::Hook::VFHook<IDirect3DSwapChain9>> g_Direct3DSwapChain9_Present_Hook{nullptr};
}

namespace Callbacks
{
    HRESULT __stdcall D3D9_OnReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters)
    {
        if (!Globals::g_pDelegate)
        {
            return Original::originalResetFunc(device, pPresentationParameters);
        }

        Globals::g_pDelegate->OnDeviceLost();
        auto result = Original::originalResetFunc(device, pPresentationParameters);
        Globals::g_pDelegate->OnReset(device);
        Globals::g_pDelegate->OnDeviceRestored(device);

        return result;
    }

    HRESULT __stdcall D3D9_OnPresentSwapChain(IDirect3DSwapChain9* pSwapChain,
                                              const RECT* pSourceRect,
                                              const RECT* pDestRect,
                                              HWND hDestWindowOverride,
                                              const RGNDATA *pDirtyRegion,
                                              DWORD dwFlags)
    {
        if (Globals::g_pDelegate) {
            IDirect3DDevice9* pDevice = nullptr;
            auto result = pSwapChain->GetDevice(&pDevice);
            if (SUCCEEDED(result) && pDevice) {
                Globals::g_pDelegate->OnPresent(pDevice);
            }
        }

        return Original::originalSwapChainPresent(pSwapChain,
                                                  pSourceRect,
                                                  pDestRect,
                                                  hDestWindowOverride,
                                                  pDirtyRegion,
                                                  dwFlags);
    }

    HRESULT __stdcall D3D9_OnPresent(IDirect3DDevice9* device,
                                     const RECT* pSourceRect,
                                     const RECT* pDestRect,
                                     HWND hDestWindowOverride,
                                     const RGNDATA* pDirtyRegion)
    {
        if (Globals::g_pDelegate) {
            Globals::g_pDelegate->OnPresent(device);
        }
        return Original::originalPresent(device, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
    }

    HRESULT __stdcall D3D9_OnGetSwapChain(IDirect3DDevice9* pDevice, UINT iSwapChain, IDirect3DSwapChain9** pSwapChain)
    {
        HRESULT res = Original::originalGetSwapChain(pDevice, iSwapChain, pSwapChain);
        spdlog::info("GetSwapChain({}, {:08X})", iSwapChain, (int)(*pSwapChain));

        if (*pSwapChain) {
            if (!Globals::g_Direct3DSwapChain9_Present_Hook) {
                Globals::g_Direct3DSwapChain9_Present_Hook = HF::Hook::HookVirtualFunction<IDirect3DSwapChain9, Consts::DX9SwapChainAPIVFTableIndex::Present>(
                        *pSwapChain, &Callbacks::D3D9_OnPresentSwapChain
                );

                if (reinterpret_cast<DWORD>(Original::originalSwapChainPresent) != Globals::g_Direct3DSwapChain9_Present_Hook->getOriginalPtr()) {
                    Original::originalSwapChainPresent = reinterpret_cast<Original::D3D9SwapChain_Present_t>(Globals::g_Direct3DSwapChain9_Present_Hook->getOriginalPtr());
                }
            }
            else {
                if (Globals::g_pDelegate) {
                    Globals::g_pDelegate->OnDeviceLost();
                    Globals::g_pDelegate->OnDeviceRestored(pDevice);
                }
            }
        }

        return res;
    }

    static void __stdcall OnZDirect3DDeviceConstructor(Hitman::BloodMoney::Engine::ZDirect3DDevice* pDevice)
    {
        spdlog::info("ZDirect3DDevice constructed at {:08X}", reinterpret_cast<std::intptr_t>(pDevice));
        spdlog::info(" D3D Device at {:08X}", reinterpret_cast<std::intptr_t>(pDevice->m_d3dDevice));

        Globals::SetupD3DHooks(pDevice->m_d3dDevice);

        if (Globals::g_pDelegate)
        {
            Globals::g_pDelegate->OnInitialised(pDevice->m_d3dDevice);
        }
    }
}

namespace Globals
{
    static void SetupD3DHooks(IDirect3DDevice9* device)
    {
        /**
         * @todo Optimize this place! Less allocations! (cause EndScene() calling that every time)
         */
        g_Direct3DDevice_Reset_Hook = HF::Hook::HookVirtualFunction<IDirect3DDevice9, Consts::DX9DeviceAPIVFTableIndex::Reset>(device, &Callbacks::D3D9_OnReset);
        g_Direct3DDevice_Present_Hook = HF::Hook::HookVirtualFunction<IDirect3DDevice9, Consts::DX9DeviceAPIVFTableIndex::DevicePresent>(device, &Callbacks::D3D9_OnPresent);
        g_Direct3DDevice_GetSwapChain_Hook = HF::Hook::HookVirtualFunction<IDirect3DDevice9, Consts::DX9DeviceAPIVFTableIndex::GetSwapChain>(device, &Callbacks::D3D9_OnGetSwapChain);

        if (reinterpret_cast<DWORD>(Original::originalResetFunc) != g_Direct3DDevice_Reset_Hook->getOriginalPtr())
            Original::originalResetFunc      = reinterpret_cast<Original::D3DReset_t>(g_Direct3DDevice_Reset_Hook->getOriginalPtr());

        if (reinterpret_cast<DWORD>(Original::originalGetSwapChain) != g_Direct3DDevice_GetSwapChain_Hook->getOriginalPtr())
            Original::originalGetSwapChain = reinterpret_cast<Original::D3D9GetSwapChain_t>(g_Direct3DDevice_GetSwapChain_Hook->getOriginalPtr());

        if (reinterpret_cast<DWORD>(Original::originalPresent) != g_Direct3DDevice_Present_Hook->getOriginalPtr())
            Original::originalPresent = reinterpret_cast<Original::D3DPresent_t>(g_Direct3DDevice_Present_Hook->getOriginalPtr());
    }
}

namespace Hitman::BloodMoney
{
    ZDirect3D9DevicePatches::ZDirect3D9DevicePatches(std::unique_ptr<IDirect3DDelegate>&& delegate)
    {
        Globals::g_pDelegate = std::move(delegate);
    }

    std::string_view ZDirect3D9DevicePatches::GetName() const
    {
        return "D3D9 Hooking Patch";
    }

    bool ZDirect3D9DevicePatches::Apply(const ModPack& modules)
    {
        using namespace HF::Hook;

        if (auto process = modules.process.lock())
        {
            m_ZDirect3DDevice_Constructor = HookFunction<void(__stdcall*)(Engine::ZDirect3DDevice*), 10>(
                    process,
                    Consts::kZDirect3DDeviceConstructor,
                    &Callbacks::OnZDirect3DDeviceConstructor,
                    {
                        HF::X86::PUSH_AD,
                        HF::X86::PUSH_FD,
                        HF::X86::PUSH_EAX
                    },
                    {
                        HF::X86::POP_FD,
                        HF::X86::POP_AD,
                    });

            if (!m_ZDirect3DDevice_Constructor->setup())
            {
                spdlog::error("Failed to initialise hook for D3D");
                return false;
            }

            return BasicPatch::Apply(modules);
        }

        return false;
    }

    void ZDirect3D9DevicePatches::Revert(const ModPack& modules)
    {
        BasicPatch::Revert(modules);

        Globals::g_pDelegate = nullptr;
        Globals::g_Direct3DDevice_Reset_Hook.reset(nullptr);
        Globals::g_Direct3DDevice_Present_Hook.reset(nullptr);
        Globals::g_Direct3DDevice_GetSwapChain_Hook.reset(nullptr);
        Globals::g_Direct3DSwapChain9_Present_Hook.reset(nullptr);
    }
}