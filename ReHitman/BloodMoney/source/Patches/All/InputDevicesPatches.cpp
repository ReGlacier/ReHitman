#include <BloodMoney/Patches/All/InputDevicesPatches.h>
#include <Glacier/ZInputDevice.h>
#include <spdlog/spdlog.h>

#include <imgui.h>

namespace Hitman::BloodMoney
{
    namespace Consts
    {
        static constexpr std::intptr_t ZWintelMouse_Constructor = 0x0045007C;
        static constexpr size_t ZInputDevice_OnUpdateIndex = 26;
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

            return BasicPatch::Apply(modules);
        }

        return false;
    }

    void InputDevicesPatches::Revert(const ModPack& modules)
    {
        m_wintelMouseCtorHook->remove();
        Globals::g_pZMouseWintelOnUpdate.reset(nullptr);
    }
}