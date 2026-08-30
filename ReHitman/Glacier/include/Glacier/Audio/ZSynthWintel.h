#pragma once

#include <Glacier/Audio/ZSynth3D.h>

#include <cstdint>

namespace Glacier
{
    struct EaxListenerProperties;
    struct ZWintelRoomReverb;

    struct SSynthCaps
    {
        int32_t m_lIndex;
        char m_szName[64];
        char m_szDescription[128];
        int32_t m_lMaxBuffers;
        bool m_bEAX;
        RE_ADD_PADDING(3);
    };

    class ZSynthWintel : public ZSynth3D
    {
    public:
        ZSynthWintel();
        ~ZSynthWintel() override;

        bool Initialize() override;
        void Reset() override;
        void Free() override;
        bool CreateSoundStreamer() override;
        void InstallWaveHeaders(int _size, const char* _fileName) override;
        void InstallWaves(int _size, const char* _fileName) override;
        void CommitFrame() override;
        void Render() override;
        void CreateFilterChains() override;
        void CreateChain(SChain* _chain, int _index) override;
        void CreateChainsEnd() override;
        void RemoveChains(int* _chains, int _count) override;
        void RemoveChain(int _chain) override;
        bool SoundEngineReady() override;
        void SignalFrameReady() override;
        bool CmdFrameReady() override;
        void SignalCmdFrameProcessed() override;

        virtual bool CreateListener();
        virtual void* GetWindowHandle();
        virtual char* GetWaveform(int _offset);
        virtual void DisplayStatus();
        virtual int SetNumBuffers(int _count);
        virtual bool SetUseEAX(bool _enabled);
        virtual SSynthCaps* GetCaps();
        virtual int Probe();

        static void GetEaxProps(EaxListenerProperties* _eax, const ZWintelRoomReverb* _environment);
        static void GetEnvProps(ZWintelRoomReverb* _environment, const EaxListenerProperties* _eax);

        int32_t m_field1BB1C;
        char m_padding1BB20[8];
        bool m_field1BB28;
        bool m_bUseEAX;
        char m_padding1BB2A[2];
        char* m_pWaveData;
        bool m_bDisplayStatus;
        RE_ADD_PADDING(3);
    };

    RE_VERIFY_OFFSET(ZSynthWintel, m_bUseEAX, 0x1BB29);
    RE_VERIFY_OFFSET(ZSynthWintel, m_pWaveData, 0x1BB2C);
    RE_VERIFY_OFFSET(ZSynthWintel, m_bDisplayStatus, 0x1BB30);
    RE_VERIFY_SIZE(ZSynthWintel, 0x1BB34);
    RE_VERIFY_OFFSET(SSynthCaps, m_szName, 0x04);
    RE_VERIFY_OFFSET(SSynthCaps, m_szDescription, 0x44);
    RE_VERIFY_OFFSET(SSynthCaps, m_lMaxBuffers, 0xC4);
    RE_VERIFY_OFFSET(SSynthCaps, m_bEAX, 0xC8);
    RE_VERIFY_SIZE(SSynthCaps, 0xCC);
}
