#pragma once

#include <Glacier/Audio/ZIOStream.h>
#include <Glacier/ReGlacier.h>

#include <cstdint>

namespace Glacier
{
    class ZSynth;

    struct SSynthCmdBase
    {
        uint32_t m_lType;
        uint32_t m_lSize;
    };

    struct SStartSoundBase : SSynthCmdBase
    {
        int32_t m_lLowpassEnabled;
        int32_t m_lFlags;
        int32_t m_lPathIdx;
        uint32_t m_lPrio;
        uint32_t m_lSndRef;
        uint32_t m_lHeaderOffset;
        int32_t m_lPitch;
        int32_t m_lBufferId;
        uint32_t m_lStartOffset;
        uint32_t m_lBufferType;
        bool m_bLooping;
        char m_padding31[3];
        const char* m_pDebugSoundName;
        int32_t m_field38;
        int32_t m_field3C;
        int32_t m_field40;
        int32_t m_field44;
        int32_t m_field48;
        int32_t m_field4C;
    };

    struct SWaveHeader
    {
        uint32_t m_field00;
        uint32_t m_field04;
        uint16_t m_iDataType;
        uint8_t m_iFlags;
        uint8_t m_iLayerInfo;
        uint32_t m_lSampleRate;
        uint32_t m_lBitsPerSample;
        uint32_t m_lDataSize;
        uint32_t m_lPackedSize;
        uint32_t m_lNumChannels;
        uint32_t m_lDataOffset;
        uint32_t m_lNumSamples;
        uint32_t m_lBlockAlign;
        uint32_t m_lSamplesPerBlock;
        int16_t m_iPosChunkSize;
        int16_t m_iSoundChunkSize;
    };

    enum EVoiceState : int32_t
    {
        VS_STARTPLAY = 1,
        VS_PLAYING = 2,
        VS_STARTFADE = 3,
        VS_FADING = 4,
        VS_STARTSOFTFADE = 5,
        VS_SOFTFADING = 6,
        VS_READY = 7
    };

    enum EBufferType : int32_t
    {
        SBT_NORMAL = 1,
        SBT_STREAM = 2,
        SBT_DISCSTREAM = 3
    };

    struct SDecodeInfo
    {
        int32_t m_Previous;
        int32_t m_Index;
        int32_t m_Loop;
        uint32_t m_dwSamples;
        int32_t m_ByteSample;
        int32_t* m_pSource;
        int32_t m_dwSampleCnt;
        int32_t m_lSourceIdx;
        char m_Data[0x4000];
        int32_t m_lBytesLeft;
        ZIOStream* m_pStream;
    };

    class _ZSoundBuffer
    {
    public:
        explicit _ZSoundBuffer(ZSynth* _synth);

        virtual bool NeedData(int* _needed);
        virtual ~_ZSoundBuffer();
        virtual bool Create(const SWaveHeader* _wave, uint32_t _bufferType, uint32_t _flags);
        virtual void Destroy();
        virtual void Start();
        virtual void Start(SSynthCmdBase* _command);
        virtual void Start(bool _start);
        virtual void Update(SStartSoundBase* _command);
        virtual void Update();
        virtual void Stop();
        virtual _ZSoundBuffer* Duplicate();
        virtual bool UpdateVolume(bool _immediate);
        virtual int CopyWaveData();
        virtual int GetPlayCursor();
        virtual void Unlock();
        virtual void Render();
        virtual void Free();
        virtual bool Allocate();
        virtual void PauseCheck(int _pause);
        virtual void Resume();
        virtual void Pause();
        virtual void ResetVolume();

        bool GetLayerSampleRates(uint32_t& _minimum, uint32_t& _maximum) const;

        ZSynth* m_pSoundCon;
        uint32_t m_lBufferSize;
        int32_t m_lCurPlaySeg;
        int32_t m_lBufferId;
        bool m_bWaitingForMetaSync;
        char m_padding15[3];
        int32_t m_lBufferIndex;
        char m_padding1C[8];
        bool m_bLowpassEnabled;
        bool m_bAddIdCmd;
        char m_padding26[2];
        EVoiceState m_VoiceState;
        int32_t m_lPlayCursor;
        int32_t m_lPrevPlayCursor;
        int32_t m_field34;
        char* m_pPoseData;
        int32_t m_lCrossFadePct;
        bool m_bFinished;
        char m_padding41[3];
        ZIOStream* m_pStream;
        int32_t m_lChainIdx;
        int32_t m_field4C;
        uint32_t m_lPrio;
        uint32_t m_dwBufferType;
        int32_t m_lDopplerPitch;
        int32_t m_field5C;
        int32_t m_field60;
        int32_t m_field64;
        int32_t m_field68;
        int32_t m_field6C;
        int32_t m_field70;
        int32_t m_field74;
        const SWaveHeader* m_rWave;
        uint32_t m_rSndObj;
        int32_t m_dwWriteOffset;
        uint32_t m_dwPlayPos;
        int32_t m_dwSize;
        uint8_t* m_pData;
        EBufferType m_eBufferType;
        bool m_bPause;
        bool m_bPlaying;
        bool m_bLooping;
        bool m_bReady;
        bool m_bInUse;
        bool m_bSegmentDataNeeded;
        char m_padding9A[2];
        SDecodeInfo m_Inf;
    };

    RE_VERIFY_SIZE(SSynthCmdBase, 0x08);
    RE_VERIFY_SIZE(SStartSoundBase, 0x50);
    RE_VERIFY_SIZE(SWaveHeader, 0x34);
    RE_VERIFY_SIZE(SDecodeInfo, 0x4028);
    RE_VERIFY_OFFSET(_ZSoundBuffer, m_bLowpassEnabled, 0x24);
    RE_VERIFY_OFFSET(_ZSoundBuffer, m_lBufferIndex, 0x18);
    RE_VERIFY_OFFSET(_ZSoundBuffer, m_VoiceState, 0x28);
    RE_VERIFY_OFFSET(_ZSoundBuffer, m_pStream, 0x44);
    RE_VERIFY_OFFSET(_ZSoundBuffer, m_rWave, 0x78);
    RE_VERIFY_OFFSET(_ZSoundBuffer, m_eBufferType, 0x90);
    RE_VERIFY_OFFSET(_ZSoundBuffer, m_Inf, 0x9C);
    RE_VERIFY_SIZE(_ZSoundBuffer, 0x40C4);
}
