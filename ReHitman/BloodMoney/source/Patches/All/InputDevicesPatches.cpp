#include <BloodMoney/Patches/All/InputDevicesPatches.h>
#include <Glacier/ZInputDevice.h>
#include <Windows.h>

#include <spdlog/spdlog.h>

#include <imgui.h>

// Win32 message handler
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Hitman::BloodMoney
{
    namespace Consts
    {
        static constexpr std::intptr_t ZWintelMouse_Constructor = 0x0045007C;
        static constexpr size_t ZInputDevice_OnUpdateIndex = 26;

        static constexpr std::intptr_t kOriginalWndProcAddr = 0x004513E0;
        static constexpr std::intptr_t kRegisterClassExAddr = 0x00453E68;
        static constexpr float kWheelDelta = WHEEL_DELTA;
    }

    namespace Globals
    {
        static std::unique_ptr<HF::Hook::VFHook<Glacier::ZInputDevice>> g_pZMouseWintelOnUpdate = nullptr;
    }

    namespace Callbacks
    {
        struct ZMouseWintel
        {
            char pad_0000[44]; //0x0000
            uint32_t m_mouseState; //0x002C
            char pad_0030[60]; //0x0030
            uint32_t m_directInput; //0x006C
            uint32_t m_device2; //0x0070
            char pad_0074[540]; //0x0074
            int32_t m_x; //0x0290
            int32_t m_y; //0x0294
            int32_t m_wheel; //0x0298
            uint8_t m_leftButton; //0x029C
            uint8_t m_rightButton; //0x029D
            uint8_t m_midButton; //0x029E
            char pad_029F[177]; //0x029F

            int OnUpdate()
            {
                HF::Hook::VFHook<Glacier::ZInputDevice>& hook = *Globals::g_pZMouseWintelOnUpdate;
                int result = hook.invoke<int>();

                ImGuiIO& io = ImGui::GetIO();
                io.MouseDown[0] = m_leftButton;
                io.MouseDown[1] = m_rightButton;
                io.MouseWheel += static_cast<float>(m_wheel) / Consts::kWheelDelta;

                return result;
            }
        };

        static void __stdcall OnZWintelMouse_Constructed(Glacier::ZInputDevice* inputDevice)
        {
            spdlog::info("Mouse device at {:08X}", (int)inputDevice);
            Globals::g_pZMouseWintelOnUpdate = HF::Hook::HookVirtualFunction<Glacier::ZInputDevice, Consts::ZInputDevice_OnUpdateIndex>(inputDevice, &ZMouseWintel::OnUpdate);
        }

        LRESULT WINAPI Glacier_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
        {
            typedef LRESULT(__stdcall* GlacierWndProc_t)(HWND, UINT, WPARAM, LPARAM);
            auto glacierWndProc = (GlacierWndProc_t)Consts::kOriginalWndProcAddr;

            ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
            const bool glacierResult = glacierWndProc(hWnd, msg, wParam, lParam);

            return glacierResult;
        }

        ATOM __stdcall RegisterClassExA_Hooked(WNDCLASSEXA* wndClass)
        {
            spdlog::info("Window class registered! Event loop function hooked!");
            wndClass->lpfnWndProc = Glacier_WndProc;
            return RegisterClassExA(wndClass);
        }

    }

    std::string_view InputDevicesPatches::GetName() const { return "Input"; }

    bool InputDevicesPatches::Apply(const ModPack& modules)
    {
        if (auto process = modules.process.lock())
        {
            m_wintelMouseCtorHook = HF::Hook::HookFunction(process, Consts::ZWintelMouse_Constructor, &Callbacks::OnZWintelMouse_Constructed,
                {
                    HF::X86::PUSH_AD,
                    HF::X86::PUSH_FD,
                    HF::X86::PUSH_EAX
                },
                {
                    HF::X86::POP_FD,
                    HF::X86::POP_AD
                });

            if (!m_wintelMouseCtorHook->setup())
            {
                spdlog::error("Failed to setup patch to ZMouseWintel::ctor!");
                return false;
            }

            // Do not revert this patch!
            if (!HF::Hook::FillMemoryByNOPs(process, Consts::kRegisterClassExAddr, kRegisterClassExPatchSize))
            {
                m_wintelMouseCtorHook->remove();
                spdlog::error("Failed to cleanup memory");
                return false;
            }

            m_registerClassExHook = HF::Hook::HookFunction<ATOM(__stdcall*)(WNDCLASSEXA*), kRegisterClassExPatchSize>(
                    process,
                    Consts::kRegisterClassExAddr,
                    &Callbacks::RegisterClassExA_Hooked,
                    {},
                    {});

            if (!m_registerClassExHook->setup())
            {
                m_wintelMouseCtorHook->remove();
                spdlog::error("Failed to setup patch to RegisterClassEx!");
                return false;
            }

            return BasicPatch::Apply(modules);
        }

        return false;
    }

    void InputDevicesPatches::Revert(const ModPack& modules)
    {
        m_wintelMouseCtorHook->remove();
        m_registerClassExHook->remove();
        Globals::g_pZMouseWintelOnUpdate.reset(nullptr);
    }
}