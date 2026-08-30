#pragma once

#include <Glacier/Audio/ZIOStream.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>

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
        int32_t m_lPause;
        int32_t m_lPathIdx;
        uint32_t m_lPrio;
        int32_t m_bLooping;
        uint32_t m_lSndRef;
        uint32_t m_lHeaderOffset;
        uint32_t m_lBufferType;
        int32_t m_lPitch;
        int32_t m_lBufferId;
        uint32_t m_lStartOffset;
        int32_t m_lVolume;
        int32_t m_lVolumeL;
        int32_t m_lVolumeR;
        int32_t m_lVolumeS;
        int32_t m_lVolumeLS;
        int32_t m_lVolumeRS;
        int32_t m_lAngleH;
    };

    struct SStartSound : SStartSoundBase
    {
        RE_ADD_PADDING(4);
        float m_fVolume;
        RE_ADD_PADDING(0x20);
    };

    struct SStartSound2D : SStartSound
    {
        int32_t m_lPan;
    };

    struct SStartSoundBFormat : SStartSound
    {
        float m_fW;
        float m_fX;
        float m_fY;
        float m_fZ;
    };

    struct SStartSound3D : SStartSound
    {
        ZVector3 m_vConeOrientation;
        RE_ADD_PADDING(0x18);
        ZVector3 m_vPosition;
        ZVector3 m_vVelocity;
        float m_fMinDistance;
        float m_fMaxDistance;
        float m_fInnerConeAngle;
        float m_fOuterConeAngle;
        float m_fOuterConeVolume;
    };

    struct SSynthFilterBase : SSynthCmdBase
    {
        int32_t m_lNextFilter;
    };

    struct SCmdOcclusionBase : SSynthFilterBase
    {
        float m_fOpenness;
    };

    struct SCmdOcclusionWintel : SCmdOcclusionBase
    {
        int32_t m_lExclusion;
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
        bool AllocateWaveResource(ZIOStream* _stream, SStartSoundBase* _command);

        ZSynth* m_pSoundCon;
        uint32_t m_lBufferSize;
        int32_t m_lCurPlaySeg;
        int32_t m_lBufferId;
        bool m_bWaitingForMetaSync;
        char m_padding15[3];
        int32_t m_lBufferIndex;
        bool m_bFrameClaimed;
        RE_ADD_PADDING(3);
        int32_t m_lGroupType;
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
        int32_t m_lVolume;
        int32_t m_lVolumeL;
        int32_t m_lVolumeR;
        int32_t m_lVolumeS;
        int32_t m_lPitch;
        int32_t m_lVolumeLS;
        int32_t m_lVolumeRS;
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
    RE_VERIFY_SIZE(SStartSoundBase, 0x54);
    RE_VERIFY_OFFSET(SStartSound, m_fVolume, 0x58);
    RE_VERIFY_SIZE(SStartSound, 0x7C);
    RE_VERIFY_OFFSET(SStartSound2D, m_lPan, 0x7C);
    RE_VERIFY_SIZE(SStartSound2D, 0x80);
    RE_VERIFY_OFFSET(SStartSoundBFormat, m_fW, 0x7C);
    RE_VERIFY_SIZE(SStartSoundBFormat, 0x8C);
    RE_VERIFY_OFFSET(SStartSound3D, m_vPosition, 0xA0);
    RE_VERIFY_OFFSET(SStartSound3D, m_fMinDistance, 0xB8);
    RE_VERIFY_SIZE(SStartSound3D, 0xCC);
    RE_VERIFY_SIZE(SSynthFilterBase, 0x0C);
    RE_VERIFY_OFFSET(SCmdOcclusionBase, m_fOpenness, 0x0C);
    RE_VERIFY_SIZE(SCmdOcclusionWintel, 0x14);
    RE_VERIFY_SIZE(SWaveHeader, 0x34);
    RE_VERIFY_SIZE(SDecodeInfo, 0x4028);
    RE_VERIFY_OFFSET(_ZSoundBuffer, m_bLowpassEnabled, 0x24);
    RE_VERIFY_OFFSET(_ZSoundBuffer, m_lGroupType, 0x20);
    RE_VERIFY_OFFSET(_ZSoundBuffer, m_bFrameClaimed, 0x1C);
    RE_VERIFY_OFFSET(_ZSoundBuffer, m_lBufferIndex, 0x18);
    RE_VERIFY_OFFSET(_ZSoundBuffer, m_VoiceState, 0x28);
    RE_VERIFY_OFFSET(_ZSoundBuffer, m_pStream, 0x44);
    RE_VERIFY_OFFSET(_ZSoundBuffer, m_rWave, 0x78);
    RE_VERIFY_OFFSET(_ZSoundBuffer, m_eBufferType, 0x90);
    RE_VERIFY_OFFSET(_ZSoundBuffer, m_Inf, 0x9C);
    RE_VERIFY_SIZE(_ZSoundBuffer, 0x40C4);
}
