#pragma once

#include <Glacier/Audio/ZDllSound.h>
#include <Glacier/Audio/ZSynthWintel.h>

#include <Windows.h>
#include <dsound.h>

namespace Glacier
{
    class ZDllSoundWintel : public ZDllSound
    {
    public:
        ZDllSoundWintel();
        ~ZDllSoundWintel() override;

        static ZDllSoundWintel* BuildInstance();

        void End() override;
        void Initialize() override;
        bool RenderFrame() override;
        bool InstallSynthesizer() override;
        void AddSourceCommand(ZSoundObject* _object, SStartSoundBase* _command) override;
        void AddListenerCommands() override;
        void FreeSynth() override;
        char* GetSynthCmdArray() override;
        void ProcessSynthCmdArray() override;
        bool ProcessSynthCmd(const char* _command) override;
        bool SynthModuleReady() override;
        void ResetSynthReady() override;
        void SignalSoundEngineReady() override;
        bool FrameReady() override;
        bool SendCmdFrameBuffer() override;

        virtual int32_t GetNumSynths();
        virtual SSynthCaps* GetSynthCaps(const char* _name);
        virtual SSynthCaps* GetSynthCaps(int _index);
        virtual int32_t GetSynthIndex(const char* _name);
        virtual void DisplaySynthStatus();
        virtual void UpdateSoundEngine();
        virtual IDirectSound8* GetDirectSound();

        int32_t m_lNumSynths;
        char* m_pMetaMemory;
        int32_t m_lSelectedSynth;
        ZSynthWintel* m_pSynth;
        HANDLE m_hSynthThread;
    };

    RE_VERIFY_OFFSET(ZDllSoundWintel, m_lNumSynths, 0x90C8);
    RE_VERIFY_OFFSET(ZDllSoundWintel, m_pSynth, 0x90D4);
    RE_VERIFY_OFFSET(ZDllSoundWintel, m_hSynthThread, 0x90D8);
    RE_VERIFY_SIZE(ZDllSoundWintel, 0x90DC);
}
