#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/Input/Input.h>
#include <Glacier/Input/ZDevice.h>


namespace Glacier
{
    class ZInputDevice : public SysInput::ZDevice
    {
    public:
		// constants
		static constexpr size_t DigHistSize = 4;
		
		// types
		struct CtrlInfo
		{
			const char* name;
			int nr;
		};
		RE_VERIFY_SIZE(CtrlInfo, 0x8);

    	// vtbl
		~ZInputDevice() override;
        SysInput::EDeviceType Type() override;
        virtual int DigitalCount() override;
        virtual int DigitalState(int nr) override;
        virtual const char* DigitalName(int nr) override;
        virtual int DigitalId(const char* name) override;
        virtual TIMETYPE DigitalHist(int nr, int nth) override;
        virtual int AnalogCount() override;
        virtual float AnalogState(int nr) override;
        virtual const char* AnalogName(int nr) override;
        virtual int AnalogId(const char* name) override;
        virtual float AnalogMotion(int nr) override;
        virtual void ResetTables(bool bClearKeys) override;
        virtual void ClearKey(int id) override;
        virtual void TrapKey(int id, bool bTrap) override;
        virtual void RumbleSetPitch(int axis, float p) override;
        virtual bool StartHaptic(unsigned int id, float a, float b, bool bLoop) override;
        virtual void StopHaptics() override;
        virtual void Activate() override;
        virtual void Deactivate() override;
        virtual const char* GetName() override;
        virtual void PrintInfo() override;

        virtual bool Initialize();
        virtual bool Terminate();
        virtual void Update();
        virtual int Acquire(int retries);
        virtual int Acquire();
        virtual int Unacquire();

		// methods
		ZInputDevice(SysInput::EDeviceType deviceType, int digcount, int anacount);
		bool InstallHandler(SysInput::DeviceHandler handler, void* pUserData);
		bool UninstallHandler(SysInput::DeviceHandler handler);
		int setControlCount(int digcount, int anacount);
		int lookupNr(CtrlInfo* list, const char* name);
        void clear();
        void buttonSample(int nr, bool state, TIMETYPE time);
        int AnalogSample(int nr, float value);
        float AnalogMotionSample(int nr, float value);

		// members
		SysInput::EDeviceType m_type;
		char name[32];
        TIMETYPE (*m_dighist)[DigHistSize];
        bool* m_pbDigital;
        int m_digcount;
		CtrlInfo* m_diginf;
        int m_diginfcount;
        float* m_anamotion;
		float* m_analog;
        int m_anacount; // yep
		CtrlInfo* m_anainf;
        int m_anainfcount;
		TIMETYPE m_updatetime;
		bool m_active;
		bool m_connected;
		RE_ADD_PADDING(2);
		SysInput::DeviceHandler m_vHandlers[2]; // +0x58
		void* m_vHandlerArgs[2]; // +0x60
		uint8_t m_iDeviceID; // +0x68
		RE_ADD_PADDING(3);
    };
	RE_VERIFY_SIZE(ZInputDevice, 0x6C);
	RE_VERIFY_OFFSET(ZInputDevice, m_vHandlers, 0x58);
	RE_VERIFY_OFFSET(ZInputDevice, m_vHandlerArgs, 0x60);
	RE_VERIFY_OFFSET(ZInputDevice, m_iDeviceID, 0x68);
}
