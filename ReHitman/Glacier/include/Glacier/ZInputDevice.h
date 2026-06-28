#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Glacier.h>
#include <Glacier/ZSysInput.h>

namespace Glacier
{
    class ZInputDevice : public SysInput::ZDevice
    {
    public:
		struct CtrlInfo
		{
			char* name;
			int nr;
		};

    	// vftable
        virtual bool Initialize();
        virtual void Terminate();
        virtual void Update();
        virtual void Acquire(int);
        virtual void Acquire();
        virtual void Unacquire();

        // --------------------------------
	    // Keyboard Wintel size 0x1698
	    // Mouse Wintel size 0x2A4 <<<
	    // Game Control Wintel size 0x2C8
	    // data (total size is 0x2A4)
		SysInput::EDeviceType m_deviceType;
		char name[32];
        int m_field28;
        bool* m_pbDigital;
        int m_digcount;
		CtrlInfo* m_diginf;
        int m_diginfcount;
        float* m_anamotion;
		float m_analog;
        int m_anacount; // yep
		CtrlInfo* m_anainf;
        int m_anainfcount;
		TIMETYPE m_updatetime;
		bool m_active;
		bool m_connected;
		RE_ADD_PADDING(2);
		int   m_vHandlers   [2]; // +0x58
		void* m_vHandlerArgs[2]; // +0x60
		uint8_t m_iDeviceID; // +0x68
		RE_ADD_PADDING(3);
    };
	RE_VERIFY_SIZE(ZInputDevice, 0x6C);
	RE_VERIFY_OFFSET(ZInputDevice, m_vHandlers, 0x58);
}