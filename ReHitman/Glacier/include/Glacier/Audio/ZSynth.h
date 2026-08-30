#pragma once

#include <Glacier/Audio/ZIOStreamer.h>
#include <Glacier/Audio/ZSoundBuffer.h>
#include <Glacier/ReGlacier.h>

#include <cstdint>

namespace Glacier
{
    class ZSoundListener;

    struct ZBufferGroup
    {
        struct SLayer
        {
            char* m_pLayerData;
            int32_t m_lBytesPerSample;
            int32_t m_lByteOffset;
        };

        SDecodeInfo m_DecodeInfo;
        SLayer m_Layers[4];
        bool m_bNewBuffers;
        char m_padding4059[3];
        int32_t m_lSampleSize;
        int32_t m_lSamples;
        const SWaveHeader* m_pFirstHeader;
        int32_t m_lPlayCursor;
        int32_t m_lCurPlaySeg;
        int32_t m_lNumGrouped;
        _ZSoundBuffer* m_pReferenceBuffer;
        _ZSoundBuffer* m_Buffers[4];
    };

    struct SFilterCon
    {
        void* m_pFilter;
        uint32_t m_lType;
    };

    struct SChain
    {
        int32_t m_bCreate;
        int32_t m_lRemapIdx;
        int32_t m_lNumFilters;
        int32_t m_lNext;
        SFilterCon m_Filters[32];
    };

    struct SBufferId
    {
        uint32_t m_lGeomRef;
        int32_t m_lBufferId;
    };

    struct SCmdChainBegin : SSynthCmdBase
    {
        int32_t m_bCreate;
        int32_t m_lRemapIdx;
    };

    struct SCmdRemoveChains : SSynthCmdBase
    {
        int32_t m_lCount;
        int32_t m_Chains[1];
    };

    struct SGroupStart : SSynthCmdBase
    {
        int32_t m_lGroupEntries;
        SBufferId m_Entries[1];
    };

    struct SCmdSoundRef : SSynthCmdBase
    {
        uint32_t m_lSndRef;
    };

    struct SCmdLPFade : SSynthCmdBase
    {
        uint32_t m_lGeomRef;
        int32_t m_lFadePct;
    };

    class ZSynth
    {
    public:
        // types
        struct STransferRequest
        {
            char* m_pDestAddr;
            char* m_pSourceAddr;
            uint32_t m_lSize;
        };

        struct SVirtualBuffer
        {
            int32_t m_lSndRef;
            uint32_t m_lErrorType;
        };

        // vtbl
        virtual void AllocateBuffers();
        virtual void FreeBuffers();
        virtual _ZSoundBuffer* GetBuffer(uint32_t _priority, SWaveHeader* _wave, uint32_t _flags, bool _required);
        virtual bool Create() = 0;
        virtual void PushScene();
        virtual void PopScene();
        virtual void InitFrame();
        virtual bool SendFrame();
        virtual bool SoundEngineReady();
        virtual void SignalFrameReady();
        virtual void RenderBuffers();
        virtual void BuildFrame();
        virtual void AddFrameCommands();
        virtual void FadeAllBuffers(float _time);
        virtual void SetEffectsImage();
        virtual bool CmdFrameReady();
        virtual void SignalCmdFrameProcessed();
        virtual _ZSoundBuffer* AllocateBuffer() = 0;
        virtual uint32_t GetWaveDuration(SWaveHeader* _wave);
        virtual SWaveHeader* GetWaveHeader(int _offset);
        virtual ~ZSynth();
        virtual bool Initialize();
        virtual void Reset();
        virtual void Free();
        virtual bool CreateSoundStreamer() = 0;
        virtual void InstallWaveHeaders(int _size, const char* _data) = 0;
        virtual void InstallWaves(int _size, const char* _data) = 0;
        virtual void InstallStreamWaves(int _size, const char* _data, const char* _localizedData);
        virtual void CommitFrame();
        virtual void Render();
        virtual bool CopyWaveData(_ZSoundBuffer* _buffer);
        virtual _ZSoundBuffer* Duplicate(_ZSoundBuffer* _buffer);
        virtual void CreateFilterChains() = 0;
        virtual void CreateChain(SChain* _chain, int _index) = 0;
        virtual void CreateChainsEnd() = 0;
        virtual void RemoveChains(int* _chains, int _count) = 0;
        virtual void RemoveChain(int _chain) = 0;
        virtual void StartMemStream();
        virtual void StopMemStream();
        virtual void SetFmvVolume(int _volume);

        // methods
        ZSynth();
        void RemoveFromMetaTab(_ZSoundBuffer* _buffer);

        int32_t m_lMaxNumGroupedPlaying;
        ZBufferGroup m_BufferGroups[4];
        char* m_pLayerData;
        int32_t m_lLayerSize;
        _ZSoundBuffer** m_pBuffers;
        ZIOStreamer* m_pStreamer;
        int32_t m_lNumChains;
        SChain m_Chains[32];
        int32_t m_lFmvVolume;
        STransferRequest m_TransferRequests[8];
        uint32_t m_lTransferRequestCnt;
        bool m_bDriveReadyRequest;
        bool m_bDriveReady;
        bool m_bDriveLocked;
        bool m_bMemStreamReady;
        bool m_bMemStreamRunning;
        bool m_bMemStreamNeedData;
        RE_ADD_PADDING(2);
        int32_t m_lMemStreamAddr;
        int32_t m_lMemStreamSize;
        int32_t m_lMemStreamReceived;
        int32_t m_lMemStreamBytesPlayed;
        char* m_pMemStreamBuffer;
        int32_t m_lMemStreamBufferSize;
        int32_t m_lMuteAllFadeTime;
        int32_t m_lCurrentFrameSegment;
        int32_t m_FrameSegment[2];
        SBufferId m_tBufferId[128];
        int32_t m_lBufferIdCnt;
        int32_t m_StartedBuffers[4096];
        int32_t m_StoppedBuffers[4096];
        int32_t m_DelayedBuffers[1024];
        _ZSoundBuffer* m_PosesBuffers[128];
        int32_t m_lStartedBuffersCnt;
        int32_t m_lStoppedBuffersCnt;
        int32_t m_lDelayedBuffersCnt;
        int32_t m_lPosesLoadedCnt;
        char* m_pPoseDestAddr;
        bool m_bRunning;
        RE_ADD_PADDING(3);
        char* m_pCmdFrameBuffer;
        char* m_pCmd;
        int32_t m_lNumPlaying;
        int32_t m_lMaxNumPlaying;
        int32_t m_lNumBuffers;
        char* m_pWaveHeaders;
        int32_t m_lWaveHeadersSize;
        int32_t m_lNumUsedBuffers;
        ZSoundListener* m_pListener;
        uint32_t m_lCurNumPlaying;
        uint32_t m_lCurLowestPrio;
    };

    RE_VERIFY_SIZE(ZBufferGroup, 0x4088);
    RE_VERIFY_SIZE(ZBufferGroup::SLayer, 0x0C);
    RE_VERIFY_OFFSET(ZBufferGroup, m_Layers, 0x4028);
    RE_VERIFY_OFFSET(ZBufferGroup, m_lNumGrouped, 0x4070);
    RE_VERIFY_OFFSET(ZBufferGroup, m_Buffers, 0x4078);
    RE_VERIFY_SIZE(SFilterCon, 0x08);
    RE_VERIFY_SIZE(SChain, 0x110);
    RE_VERIFY_SIZE(SBufferId, 0x08);
    RE_VERIFY_SIZE(SCmdChainBegin, 0x10);
    RE_VERIFY_SIZE(ZSynth::STransferRequest, 0x0C);
    RE_VERIFY_SIZE(ZSynth::SVirtualBuffer, 0x08);
    RE_VERIFY_OFFSET(ZSynth, m_BufferGroups, 0x08);
    RE_VERIFY_OFFSET(ZSynth, m_pLayerData, 0x10228);
    RE_VERIFY_OFFSET(ZSynth, m_pBuffers, 0x10230);
    RE_VERIFY_OFFSET(ZSynth, m_pStreamer, 0x10234);
    RE_VERIFY_OFFSET(ZSynth, m_lNumChains, 0x10238);
    RE_VERIFY_OFFSET(ZSynth, m_Chains, 0x1023C);
    RE_VERIFY_OFFSET(ZSynth, m_lFmvVolume, 0x1243C);
    RE_VERIFY_OFFSET(ZSynth, m_TransferRequests, 0x12440);
    RE_VERIFY_OFFSET(ZSynth, m_lTransferRequestCnt, 0x124A0);
    RE_VERIFY_OFFSET(ZSynth, m_lMemStreamAddr, 0x124AC);
    RE_VERIFY_OFFSET(ZSynth, m_tBufferId, 0x124D4);
    RE_VERIFY_OFFSET(ZSynth, m_StartedBuffers, 0x128D8);
    RE_VERIFY_OFFSET(ZSynth, m_StoppedBuffers, 0x168D8);
    RE_VERIFY_OFFSET(ZSynth, m_DelayedBuffers, 0x1A8D8);
    RE_VERIFY_OFFSET(ZSynth, m_PosesBuffers, 0x1B8D8);
    RE_VERIFY_OFFSET(ZSynth, m_pPoseDestAddr, 0x1BAE8);
    RE_VERIFY_OFFSET(ZSynth, m_lNumBuffers, 0x1BB00);
    RE_VERIFY_OFFSET(ZSynth, m_pWaveHeaders, 0x1BB04);
    RE_VERIFY_SIZE(ZSynth, 0x1BB1C);
}
