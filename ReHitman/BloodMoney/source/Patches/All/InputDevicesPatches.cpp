#include <BloodMoney/Patches/All/InputDevicesPatches.h>
#include <BloodMoney/Delegates/IInputDelegate.h>
#include <BloodMoney/Game/Globals.h>
#include <Glacier/ZSysInterfaceWintel.h>
#include <Glacier/ZInputDevice.h>
#include <Windows.h>

#include <spdlog/spdlog.h>

namespace Hitman::BloodMoney
{
    struct ZSysInputCustom;

    namespace Consts
    {
      static constexpr size_t ZSysInput_OnUpdateIndex = 1;
      static constexpr size_t ZInputDevice_OnUpdateIndex = 26;

        static constexpr std::intptr_t kOriginalWndProcAddr = 0x004513E0;
        static constexpr std::intptr_t kRegisterClassExAddr = 0x00453E68;
        static constexpr float kWheelDelta = WHEEL_DELTA;
    }

    namespace Globals
    {
        static std::unique_ptr<IInputDelegate> g_pInputDelegate = nullptr;
        static std::unique_ptr<HF::Hook::VFHook<ZSysInputCustom>> g_sysInputOnUpdateHook = nullptr;
    }

    namespace Callbacks
    {
        LRESULT WINAPI Glacier_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
        {
            typedef LRESULT(__stdcall* GlacierWndProc_t)(HWND, UINT, WPARAM, LPARAM);
            auto glacierWndProc = (GlacierWndProc_t)Consts::kOriginalWndProcAddr;

            if (Globals::g_pInputDelegate)
            {
                switch (msg)
                {
                    case WM_KEYDOWN:
                        Globals::g_pInputDelegate->setKeyState(static_cast<int>(wParam), true);
                        break;
                    case WM_KEYUP:
                        Globals::g_pInputDelegate->setKeyState(static_cast<int>(wParam), false);
                        break;
                    default: /* unhandled event */ break;
                }

                Globals::g_pInputDelegate->onWindowsEvent(hWnd, msg, wParam, lParam);
            }

            const bool glacierResult = glacierWndProc(hWnd, msg, wParam, lParam);

            return glacierResult;
        }
    }

    struct ZMouseWintel {
	    ///
		/// REVERSED DATA [READ ONLY, DO NOT CHANGE THE STRUCTURE]
		///
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
    };

    struct ZSysInputCustom {
        struct ZInputDevice {
            int* vtbl;
        };

        static constexpr int kMouseIndex = 0;

        /**
         * @fn OnUpdate
         * @brief Called by Glacier when engine wants to update input devices
         */
		bool OnUpdate() {
			if (m_attachedDevicesCount) {
				for (int i = 0; i < m_attachedDevicesCount; i++) {
					if (!m_devices.allDevices[i]) {
						continue;
					}

					// Call OnUpdate of each input device if it's points valid
					(reinterpret_cast<void(__thiscall*)(ZInputDevice*)>(m_devices.allDevices[i]->vtbl[Consts::ZInputDevice_OnUpdateIndex]))(m_devices.allDevices[i]);
				}

				UpdateMouse();
			}

			return true;
		}

    private:
    	void UpdateMouse() {
		    if (m_devices.mapped.m_mouse && Globals::g_pInputDelegate) {
			    Globals::g_pInputDelegate->setMouseKeyState(0, m_devices.mapped.m_mouse->m_leftButton);
			    Globals::g_pInputDelegate->setMouseKeyState(1, m_devices.mapped.m_mouse->m_rightButton);
			    Globals::g_pInputDelegate->setMouseWheelState(static_cast<float>(m_devices.mapped.m_mouse->m_wheel) / Consts::kWheelDelta);
		    }
		}

    public:
		int vtbl { 0 };
		int m_unk4 { 0 };
		int m_unk8 { 0 };
		int m_unkC { 0 };
		union {
		  ZInputDevice* allDevices[32];
		  struct  {
			ZMouseWintel* m_mouse;
			int m_keyboard;
			// other devices here
			// ...
		  } mapped;
		} m_devices; //+0x10
		int m_attachedDevicesCount { 0 };
		int* m_field94 { nullptr };
    };
    static_assert(offsetof(ZSysInputCustom, m_field94) == 0x94, "ZSysInput bad offset!");

    InputDevicesPatches::InputDevicesPatches(std::unique_ptr<IInputDelegate>&& delegate)
    {
        Globals::g_pInputDelegate = std::move(delegate);
    }

    std::string_view InputDevicesPatches::GetName() const { return "Input"; }

    bool InputDevicesPatches::Apply(const ModPack& modules)
    {
        if (auto process = modules.process.lock())
        {
            auto sysInterface = Glacier::getInterface<Glacier::ZSysInterfaceWintel>(Globals::kSysInterfaceAddr);
            SetWindowLongPtr((HWND)sysInterface->m_appWindowHWND, GWL_WNDPROC, (LONG)(LONG_PTR)Callbacks::Glacier_WndProc);

            // Override vtbl of SysInput method
            auto sysInput = Glacier::getInterface<ZSysInputCustom>(Hitman::BloodMoney::Globals::kSysInputAddr);
            if (sysInput) {
	            Globals::g_sysInputOnUpdateHook = HF::Hook::HookVirtualFunction<ZSysInputCustom, Consts::ZSysInput_OnUpdateIndex>(sysInput, &ZSysInputCustom::OnUpdate);
	            spdlog::info("ZSysInput::OnUpdate hook inited!");
            }

            return BasicPatch::Apply(modules);
        }

        return false;
    }

    void InputDevicesPatches::Revert(const ModPack& modules)
    {
        BasicPatch::Revert(modules);

	    Globals::g_sysInputOnUpdateHook.reset(nullptr);
        Globals::g_pInputDelegate.reset(nullptr);
    }
}