#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Input/ZSysInput.h>
#include <Glacier/Input/WintelFwd.h>
#include <Glacier/ZUniMemory.h>

#include <Windows.h>
#include <cstdint>


namespace Glacier
{
	class ZSysInputWintel : public ZSysInput 
	{
	public:
		// vtbl
		~ZSysInputWintel() override;
        int Update() override;
		void SetActiveController(int controlledIndex) override;
		uint8_t SetTimeoutCallback(SysInput::TimeoutCallback callback, int timeout) override;
		void UnSetTimeoutCallback() override;
		bool IsControllerConnected() override;

		// methods
		ZSysInputWintel();
		void UpdateBindings();
		void UpdateBindings(SysInput::EDeviceType eDeviceType);
		void DetectDevices(void);
		void AcquireDevices(void);
		void UnacquireDevices();

		static BOOL CALLBACK EnumDevicesCallback(const DIDEVICEINSTANCEA* pDIDeviceInstance, void* pG1Device);
		
		// members
		// Tail fields are present in the PC layout but no confirmed PC code path has
		// been found to access them. They may belong to controller timeout/dialog state
		// used by other platform builds.
		int m_field98;
		int m_field9C;
		int m_fieldA0;
		int m_fieldA4;
		int m_fieldA8;
	};
	RE_VERIFY_SIZE(ZSysInputWintel, 0xAC); // Verified
	RE_VERIFY_OFFSET(ZSysInputWintel, m_field98, 0x98);
	RE_VERIFY_OFFSET(ZSysInputWintel, m_field9C, 0x9C);
	RE_VERIFY_OFFSET(ZSysInputWintel, m_fieldA0, 0xA0);
	RE_VERIFY_OFFSET(ZSysInputWintel, m_fieldA4, 0xA4);
	RE_VERIFY_OFFSET(ZSysInputWintel, m_fieldA8, 0xA8);

	STATIC_GLOBAL_CLASS_INSTANCE(IDirectInput8A*, g_pDI);
}
