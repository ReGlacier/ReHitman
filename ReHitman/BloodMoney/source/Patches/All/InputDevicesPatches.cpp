#include <BloodMoney/Patches/All/InputDevicesPatches.h>
#include <BloodMoney/Delegates/IInputDelegate.h>
#include <BloodMoney/Game/Globals.h>
#include <Glacier/ZSysInterfaceWintel.h>
#include <Glacier/ZSysInputWintel.h>
#include <Glacier/ZInputDevice.h>

#include <spdlog/spdlog.h>

#include <imgui.h>

#include <Windows.h>
#include <dinput.h>


namespace Hitman::BloodMoney
{
    struct ZSysInputCustom;

	struct SKeyCodeRepr
	{
		uint32_t directInputKeyCode;
		uint32_t winApiKeyCode;
		ImGuiKey imGuiCode;

		[[nodiscard]] explicit operator bool() const noexcept { return directInputKeyCode != 0 || winApiKeyCode != 0; }

		static SKeyCodeRepr DirectInput(uint32_t dik) { return SKeyCodeRepr { dik, 0, ImGuiKey_COUNT }; }
	};

	//
	// Stored all 'special' key codes. Other codes are 'ascii' representable and could be converted via MapVirtualKeyA method
	static const std::unordered_map<uint32_t, SKeyCodeRepr> s_GlacierKeyCodesTable = {
		{ 0x0, SKeyCodeRepr { DIK_PRIOR, 0, 0 } },
		{ 0x1, SKeyCodeRepr { DIK_NEXT, 0, 0 } },
		{ 0x3, SKeyCodeRepr { 0, VK_HOME, ImGuiKey_Home } },
		{ 0x4, SKeyCodeRepr { 0, VK_END, ImGuiKey_End } },
		{ 0x5, SKeyCodeRepr { DIK_UP, 0, ImGuiKey_UpArrow } },
		{ 0x6, SKeyCodeRepr { DIK_DOWN, 0, ImGuiKey_DownArrow } },
		{ 0x7, SKeyCodeRepr { DIK_LEFT, 0, ImGuiKey_LeftArrow } },
		{ 0x8, SKeyCodeRepr { DIK_RIGHT, 0, ImGuiKey_RightArrow } },
		{ 0x9, SKeyCodeRepr { 0, VK_RETURN, ImGuiKey_Enter } },
		{ 0xA, SKeyCodeRepr { DIK_GRAVE, VK_OEM_5, 0 } },
		{ 0xB, SKeyCodeRepr { DIK_DELETE, 0, ImGuiKey_Delete } },
		{ 0xC, SKeyCodeRepr { 0, VK_BACK, ImGuiKey_Backspace } },
		{ 0xE, SKeyCodeRepr { 0, VK_TAB, ImGuiKey_Tab } },
		{ 0xF, SKeyCodeRepr::DirectInput(DIK_F1) },
		{ 0x10, SKeyCodeRepr::DirectInput(DIK_F2) },
		{ 0x11, SKeyCodeRepr::DirectInput(DIK_F3) },
		{ 0x12, SKeyCodeRepr::DirectInput(DIK_F4) },
		{ 0x13, SKeyCodeRepr::DirectInput(DIK_F5) },
		{ 0x14, SKeyCodeRepr::DirectInput(DIK_F6) },
		{ 0x15, SKeyCodeRepr::DirectInput(DIK_F7) },
		{ 0x16, SKeyCodeRepr::DirectInput(DIK_F8) },
		{ 0x17, SKeyCodeRepr::DirectInput(DIK_F9) },
		{ 0x18, SKeyCodeRepr::DirectInput(DIK_F10) },
		{ 0x19, SKeyCodeRepr::DirectInput(DIK_F11) },
		{ 0x1A, SKeyCodeRepr::DirectInput(DIK_F12) },
		{ 0x1E, SKeyCodeRepr { DIK_INSERT, 0, ImGuiKey_Insert } }
	};

    namespace Consts
    {
      static constexpr size_t ZSysInput_OnUpdateIndex = 1;
	  static constexpr float kWheelDelta = WHEEL_DELTA;
    }

    namespace Globals
    {
        static std::unique_ptr<IInputDelegate> g_pInputDelegate = nullptr;
        static std::unique_ptr<HF::Hook::VFHook<ZSysInputCustom>> g_sysInputOnUpdateHook = nullptr;
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

    static int __cdecl GlacierKeyboardHandler(unsigned int key, int mask, void* pData) {
		// Detect pressed keys and classify 'em
    	const unsigned int keyId     = (key & 0xFFFu); // Identifier of key with modifier
		const unsigned int applyMask = (key & 0xFFFFF000u); // mask of key (sometimes contains other values, need to research)
    	const int deviceIndex = reinterpret_cast<int>(pData);
    	const bool isASCIIRepresentable = (keyId & 0x700) != 0;

    	/**
    	 * Known modifiers
    	 */
    	enum KeyModifier : unsigned int {
    		NO_MODIFIER = 0x0u,
    		L_SHIFT = 0x1u,
    		R_SHIFT = 0x2u,
		    L_CTRL = 0x8u,
		    R_CTRL = 0x10u,
		    L_ALT = 0x40u,
		    R_ALT = 0x80u,
		    PRESSED = 0x200u,
	    };

	    //const unsigned int modifier = (mask & 0xFu); // It's a mask based on KeyModifier

	    //spdlog::info("G1HDL[{}]: CC={:08X}({}) (CID={:X}, AM={:X}) MSK={:08X} MOD={:01X} IP={}", deviceIndex, key, (char)((keyId & 0xFFu)), keyId, applyMask, mask, modifier, ((mask & KeyModifier::PRESSED) ? "P": "/"));

		if (ImGui::GetCurrentContext()) {
			auto& io = ImGui::GetIO();

			io.KeyCtrl = (mask & KeyModifier::L_CTRL) || (mask & KeyModifier::R_CTRL);
			io.KeyShift = (mask & KeyModifier::L_SHIFT) || (mask & KeyModifier::R_SHIFT);
			io.KeyAlt = (mask & KeyModifier::L_ALT) || (mask & KeyModifier::R_ALT);

			auto g1code = static_cast<UINT>(static_cast<unsigned char>(keyId & 0xFFu));
			if (auto it = s_GlacierKeyCodesTable.find(g1code); it != s_GlacierKeyCodesTable.end())
			{
				// It's a special key, use mappings
				const auto& vk = it->second.winApiKeyCode;
				const auto& dik = it->second.directInputKeyCode;
				const auto& im = it->second.imGuiCode;

				if (vk)
				{
					io.KeysDown[vk] = (mask & KeyModifier::PRESSED);
				}
				else if (im != ImGuiKey_COUNT)
				{
					io.KeysDown[io.KeyMap[im]] = (mask & KeyModifier::PRESSED);
				}

				if (dik == DIK_F3 && Globals::g_pInputDelegate && ((mask & KeyModifier::PRESSED))) {
					Globals::g_pInputDelegate->setKeyState(VK_F3, 0);
				}
			} else if (isASCIIRepresentable)
			{
				// Normalize key from lower to upper case
				if (g1code >= 'a' && g1code <= 'z')
					g1code = (g1code - static_cast<UINT>('a')) + static_cast<UINT>('A');

				// It's ascii code, need to re-map it
				const auto vkCode = MapVirtualKeyA(g1code, MAPVK_VSC_TO_VK_EX);
				if (vkCode != 0x0)
				{
					io.KeysDown[vkCode] = (mask & KeyModifier::PRESSED);

					if (mask & KeyModifier::PRESSED) {
						io.AddInputCharacter(static_cast<unsigned char>(keyId & 0xFFu));
					}
				}
				else spdlog::warn("Failed to translate key code from g1 ascii code {:02X} ({})", g1code, g1code);
			}
		}

		//
		// Always return 1 because Glacier will remove this handler if we will return 0
    	return 1;
    }

    struct ZSysInputCustom : public Glacier::ZSysInputWintel {
        static constexpr int kMouseIndex = 0;

        /**
         * @fn OnUpdate
         * @brief Called by Glacier when engine wants to update input devices
         */
		bool OnUpdate() {
			if (m_attachedDevicesCount) {
				for (int i = 0; i < m_attachedDevicesCount; i++) {
					if (!m_devices[i]) {
						continue;
					}

					// Call OnUpdate of each input device if it's points valid
					m_devices[i]->Update();
				}

				UpdateMouse();

				static bool g_kbHandlerAttached = false;
				if (!g_kbHandlerAttached) {
					int kbDeviceIndex = GetPrimaryDevice(Glacier::EDeviceType::KEYBOARD);
					g_kbHandlerAttached = InstallHandler(kbDeviceIndex, GlacierKeyboardHandler, (void*)kbDeviceIndex);
					if (g_kbHandlerAttached) {
						spdlog::info("Keyboard handler added!");
					} else {
						g_kbHandlerAttached = false;
						spdlog::warn("Failed to install handler to keyboard");
					}
				}
			}

			return true;
		}

    private:
    	void UpdateMouse() {
			auto mouseDevice = reinterpret_cast<ZMouseWintel*>(GetPrimaryDevicePtr(Glacier::EDeviceType::MOUSE));

		    if (mouseDevice && Globals::g_pInputDelegate) {
			    Globals::g_pInputDelegate->setMouseKeyState(0, mouseDevice->m_leftButton);
			    Globals::g_pInputDelegate->setMouseKeyState(1, mouseDevice->m_rightButton);
			    Globals::g_pInputDelegate->setMouseWheelState(static_cast<float>(mouseDevice->m_wheel) / Consts::kWheelDelta);
		    }
		}
    };

    InputDevicesPatches::InputDevicesPatches(std::unique_ptr<IInputDelegate>&& delegate)
    {
        Globals::g_pInputDelegate = std::move(delegate);
    }

    std::string_view InputDevicesPatches::GetName() const { return "Input"; }

    bool InputDevicesPatches::Apply(const ModPack& modules)
    {
	    // Override vtbl of SysInput method
	    auto sysInput = Glacier::getInterface<ZSysInputCustom>(Hitman::BloodMoney::Globals::kSysInputAddr);
	    if (sysInput) {
		    Globals::g_sysInputOnUpdateHook = HF::Hook::HookVirtualFunction<ZSysInputCustom, Consts::ZSysInput_OnUpdateIndex>(sysInput, &ZSysInputCustom::OnUpdate);
		    spdlog::info("ZSysInput::OnUpdate hook inited!");
	    }

	    return BasicPatch::Apply(modules);
    }

    void InputDevicesPatches::Revert(const ModPack& modules)
    {
        BasicPatch::Revert(modules);

	    Globals::g_sysInputOnUpdateHook.reset(nullptr);
        Globals::g_pInputDelegate.reset(nullptr);
    }
}