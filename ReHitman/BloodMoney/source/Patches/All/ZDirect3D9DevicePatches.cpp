#include <BloodMoney/Patches/All/ZDirect3D9DevicePatches.h>
#include <BloodMoney/Delegates/IDirect3DDelegate.h>
#include <BloodMoney/Engine/ZDirect3DDevice.h>

#include <spdlog/spdlog.h>

namespace Consts
{
    enum DX9DeviceAPIVFTableIndex : size_t
    {
        BeginScene = 41,
        EndScene = 42,
        Reset = 16,
        SetTexture = 65,
        DrawIndexedPrimitive = 82
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
    typedef HRESULT(__stdcall* D3DBeginScene_t)(IDirect3DDevice9*);
    typedef HRESULT(__stdcall* D3DEndScene_t)(IDirect3DDevice9*);
    typedef HRESULT(__stdcall* D3DReset_t)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
//    typedef HRESULT(__stdcall* D3DDrawIndexedPrimitive_t)(IDirect3DDevice9*, D3DPRIMITIVETYPE, INT, UINT, UINT, UINT, UINT);
//    typedef HRESULT(__stdcall* D3DSetTexture_t)(IDirect3DDevice9*, DWORD, IDirect3DBaseTexture9*);

    D3DBeginScene_t				originalBeginSceneFunc;
    D3DEndScene_t				originalEndSceneFunc;
    D3DReset_t					originalResetFunc;
//    D3DDrawIndexedPrimitive_t	originalDrawIndexedPrimitiveFunc;
//    D3DSetTexture_t				originalD3DSetTextureFunc;
}

namespace Callbacks
{
    HRESULT __stdcall D3D9_OnBeginScene(IDirect3DDevice9* device)
    {
        HRESULT result = Original::originalBeginSceneFunc(device);

        if (Globals::g_pDelegate)
        {
            Globals::g_pDelegate->OnBeginScene(device);
        }

        return result;
    }

    HRESULT __stdcall D3D9_OnEndScene(IDirect3DDevice9* device)
    {
        HRESULT result = Original::originalEndSceneFunc(device);

        if (Globals::g_pDelegate)
        {
            Globals::g_pDelegate->OnEndScene(device);
        }

        return result;
    }

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
    static std::unique_ptr<HF::Hook::VFHook<IDirect3DDevice9>> g_Direct3DDevice_BeginScene_Hook { nullptr };
    static std::unique_ptr<HF::Hook::VFHook<IDirect3DDevice9>> g_Direct3DDevice_EndScene_Hook { nullptr };
    static std::unique_ptr<HF::Hook::VFHook<IDirect3DDevice9>> g_Direct3DDevice_Reset_Hook { nullptr };

    static void SetupD3DHooks(IDirect3DDevice9* device)
    {
        /**
         * @todo Optimize this place! Less allocations! (cause EndScene() calling that every time)
         */
        g_Direct3DDevice_BeginScene_Hook = HF::Hook::HookVirtualFunction<IDirect3DDevice9, Consts::DX9DeviceAPIVFTableIndex::BeginScene>(device, &Callbacks::D3D9_OnBeginScene);
        g_Direct3DDevice_EndScene_Hook   = HF::Hook::HookVirtualFunction<IDirect3DDevice9, Consts::DX9DeviceAPIVFTableIndex::EndScene>(device, &Callbacks::D3D9_OnEndScene);
        g_Direct3DDevice_Reset_Hook      = HF::Hook::HookVirtualFunction<IDirect3DDevice9, Consts::DX9DeviceAPIVFTableIndex::Reset>(device, &Callbacks::D3D9_OnReset);

        if (reinterpret_cast<DWORD>(Original::originalBeginSceneFunc) != g_Direct3DDevice_BeginScene_Hook->getOriginalPtr())
            Original::originalBeginSceneFunc = reinterpret_cast<Original::D3DBeginScene_t>(g_Direct3DDevice_BeginScene_Hook->getOriginalPtr());

        if (reinterpret_cast<DWORD>(Original::originalEndSceneFunc) != g_Direct3DDevice_EndScene_Hook->getOriginalPtr())
            Original::originalEndSceneFunc   = reinterpret_cast<Original::D3DEndScene_t>(g_Direct3DDevice_EndScene_Hook->getOriginalPtr());

        if (reinterpret_cast<DWORD>(Original::originalResetFunc) != g_Direct3DDevice_Reset_Hook->getOriginalPtr())
            Original::originalResetFunc      = reinterpret_cast<Original::D3DReset_t>(g_Direct3DDevice_Reset_Hook->getOriginalPtr());
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
        Globals::g_Direct3DDevice_BeginScene_Hook.reset(nullptr);
        Globals::g_Direct3DDevice_EndScene_Hook.reset(nullptr);
        Globals::g_Direct3DDevice_Reset_Hook.reset(nullptr);
    }
}