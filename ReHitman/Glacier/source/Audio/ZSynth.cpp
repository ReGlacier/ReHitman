#include <Glacier/Audio/ZSynth.h>
#include <Glacier/Audio/ZSoundListener.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZUniAssert.h>

#include <cstring>

namespace Glacier
{
    namespace
    {
        constexpr int CommandBufferSize = 0x10000;
        char g_SynthCommandBuffer[CommandBufferSize]{};
        char g_SynthFrameBuffer[CommandBufferSize]{};
        char g_SynthLayerData[4 * 70624]{};
        char g_SynthPoseData[4 * 0x1000]{};

        struct SResultId : SSynthCmdBase
        {
            int32_t m_lId;
        };

        struct SResultBufferId : SSynthCmdBase
        {
            uint32_t m_lGeomRef;
            int32_t m_lBufferId;
        };
    }

    ZSynth::ZSynth()
        : m_lMaxNumGroupedPlaying(4)
        , m_pLayerData(g_SynthLayerData)
        , m_lLayerSize(70624)
        , m_pBuffers(nullptr)
        , m_pStreamer(nullptr)
        , m_lNumChains(0)
        , m_lFmvVolume(100)
        , m_lTransferRequestCnt(0)
        , m_bDriveReadyRequest(false)
        , m_bDriveReady(false)
        , m_bDriveLocked(false)
        , m_bMemStreamReady(false)
        , m_bMemStreamRunning(false)
        , m_bMemStreamNeedData(false)
        , m_lMemStreamAddr(0)
        , m_lMemStreamSize(0)
        , m_lMemStreamReceived(0)
        , m_lMemStreamBytesPlayed(0)
        , m_pMemStreamBuffer(nullptr)
        , m_lMemStreamBufferSize(0)
        , m_lMuteAllFadeTime(0)
        , m_lCurrentFrameSegment(0)
        , m_lBufferIdCnt(0)
        , m_lStartedBuffersCnt(0)
        , m_lStoppedBuffersCnt(0)
        , m_lDelayedBuffersCnt(0)
        , m_lPosesLoadedCnt(0)
        , m_pPoseDestAddr(g_SynthPoseData)
        , m_bRunning(false)
        , m_pCmdFrameBuffer(g_SynthFrameBuffer)
        , m_pCmd(g_SynthCommandBuffer)
        , m_lNumPlaying(0)
        , m_lMaxNumPlaying(16)
        , m_lNumBuffers(16)
        , m_pWaveHeaders(nullptr)
        , m_lWaveHeadersSize(0)
        , m_lNumUsedBuffers(0)
        , m_pListener(nullptr)
        , m_lCurNumPlaying(0)
        , m_lCurLowestPrio(0)
    {
        for (ZBufferGroup& group : m_BufferGroups)
        {
            std::memset(&group, 0, sizeof(group));
        }
        std::memset(m_Chains, 0, sizeof(m_Chains));
        std::memset(m_TransferRequests, 0, sizeof(m_TransferRequests));
        std::memset(m_FrameSegment, 0, sizeof(m_FrameSegment));
        std::memset(m_tBufferId, 0, sizeof(m_tBufferId));
        std::memset(m_StartedBuffers, 0, sizeof(m_StartedBuffers));
        std::memset(m_StoppedBuffers, 0, sizeof(m_StoppedBuffers));
        std::memset(m_DelayedBuffers, 0, sizeof(m_DelayedBuffers));
        std::memset(m_PosesBuffers, 0, sizeof(m_PosesBuffers));
    }

    void ZSynth::AllocateBuffers()
    {
        m_pBuffers = static_cast<_ZSoundBuffer**>(
            ZUniMemory::Allocate(sizeof(_ZSoundBuffer*) * m_lNumBuffers));

        for (int32_t i = 0; i < m_lNumBuffers; ++i)
        {
            m_pBuffers[i] = AllocateBuffer();
            m_pBuffers[i]->m_lBufferIndex = i;
        }
    }

    void ZSynth::FreeBuffers()
    {
    }

    _ZSoundBuffer* ZSynth::GetBuffer(uint32_t _priority, SWaveHeader* _wave, uint32_t, bool _required)
    {
        _ZSoundBuffer* freeBuffer = nullptr;
        _ZSoundBuffer* lowestResident = nullptr;
        _ZSoundBuffer* lowestStream = nullptr;
        int streamCount = 0;
        m_lNumUsedBuffers = 0;
        for (int i = 0; i < m_lNumBuffers; ++i)
        {
            _ZSoundBuffer* buffer = m_pBuffers[i];
            if (!buffer->m_bInUse)
            {
                if (!freeBuffer)
                    freeBuffer = buffer;
                continue;
            }
            ++m_lNumUsedBuffers;
            if (static_cast<int8_t>(buffer->m_rWave->m_iFlags) >= 0 || buffer->m_lGroupType == 2)
            {
                if (!lowestResident || buffer->m_lPrio < lowestResident->m_lPrio)
                    lowestResident = buffer;
            }
            else
            {
                if (!lowestStream || buffer->m_lPrio < lowestStream->m_lPrio)
                    lowestStream = buffer;
                ++streamCount;
            }
        }

        if (_required)
        {
            if (freeBuffer)
                return freeBuffer;
            if (m_lNumUsedBuffers == m_lNumBuffers)
            {
                if (!lowestResident || (lowestStream && lowestResident->m_lPrio >= lowestStream->m_lPrio))
                    lowestResident = lowestStream;
                return lowestResident && lowestResident->m_lPrio < _priority ? lowestResident : nullptr;
            }
        }
        if (static_cast<int8_t>(_wave->m_iFlags) < 0 && streamCount == 4)
            return lowestStream && lowestStream->m_lPrio < _priority ? lowestStream : freeBuffer;
        if (m_lNumUsedBuffers == m_lNumBuffers && lowestResident && lowestResident->m_lPrio < _priority)
            return lowestResident;
        return freeBuffer;
    }

    void ZSynth::PushScene()
    {
        m_bDriveReady = false;
        m_bDriveReadyRequest = false;
        m_bDriveLocked = false;
        m_lWaveHeadersSize = 0;
        PopScene();
    }

    void ZSynth::PopScene()
    {
        Reset();
    }

    void ZSynth::InitFrame()
    {
        m_pCmd = g_SynthCommandBuffer;
        auto* command = reinterpret_cast<SSynthCmdBase*>(m_pCmd);
        command->m_lType = 0x10000;
        command->m_lSize = sizeof(SSynthCmdBase);
    }

    bool ZSynth::SendFrame()
    {
        SignalFrameReady();
        return true;
    }

    bool ZSynth::SoundEngineReady()
    {
        return true;
    }

    void ZSynth::SignalFrameReady()
    {
    }

    void ZSynth::RenderBuffers()
    {
        for (int i = 0; i < m_lNumBuffers; ++i)
        {
            if (m_pBuffers[i] && m_pBuffers[i]->m_bInUse)
                m_pBuffers[i]->Render();
        }
    }

    void ZSynth::BuildFrame()
    {
        AddFrameCommands();

        auto appendIds = [this](uint32_t _type, const int32_t* _ids, int _count)
        {
            for (int i = 0; i < _count; ++i)
            {
                auto* command = reinterpret_cast<SResultId*>(m_pCmd);
                command->m_lType = _type;
                command->m_lSize = sizeof(SResultId);
                command->m_lId = _ids[i];
                m_pCmd += sizeof(SResultId);
            }
        };

        appendIds(0x1003, m_DelayedBuffers, m_lDelayedBuffersCnt);
        appendIds(0x1001, m_StartedBuffers, m_lStartedBuffersCnt);
        appendIds(0x1002, m_StoppedBuffers, m_lStoppedBuffersCnt);

        for (int i = 0; i < m_lBufferIdCnt; ++i)
        {
            auto* command = reinterpret_cast<SResultBufferId*>(m_pCmd);
            command->m_lType = 0x1004;
            command->m_lSize = sizeof(SResultBufferId);
            command->m_lGeomRef = m_tBufferId[i].m_lGeomRef;
            command->m_lBufferId = m_tBufferId[i].m_lBufferId;
            m_pCmd += sizeof(SResultBufferId);
        }

        m_lStartedBuffersCnt = 0;
        m_lStoppedBuffersCnt = 0;
        m_lDelayedBuffersCnt = 0;
        m_lPosesLoadedCnt = 0;
        m_lBufferIdCnt = 0;
    }

    void ZSynth::AddFrameCommands()
    {
    }

    void ZSynth::FadeAllBuffers(float)
    {
    }

    void ZSynth::SetEffectsImage()
    {
    }

    bool ZSynth::CmdFrameReady()
    {
        return true;
    }

    void ZSynth::SignalCmdFrameProcessed()
    {
    }

    uint32_t ZSynth::GetWaveDuration(SWaveHeader* _wave)
    {
        return (_wave->m_lNumSamples / _wave->m_lSampleRate) / _wave->m_lNumChannels;
    }

    SWaveHeader* ZSynth::GetWaveHeader(int _offset)
    {
        if (!m_pWaveHeaders || !_offset)
            return nullptr;
        return reinterpret_cast<SWaveHeader*>(m_pWaveHeaders + _offset);
    }

    ZSynth::~ZSynth() = default;

    bool ZSynth::Initialize()
    {
        if (!CreateSoundStreamer() || !Create())
        {
            if (m_pStreamer)
            {
                m_pStreamer->Destroy();
                ZUniMemory::Delete(m_pStreamer);
                m_pStreamer = nullptr;
            }
            return false;
        }

        AllocateBuffers();
        m_bRunning = true;
        for (int i = 0; i < 4; ++i)
        {
            m_pStreamer->m_Streams[i].m_pUserData = &m_BufferGroups[i];
            for (int layer = 0; layer < 4; ++layer)
                m_BufferGroups[i].m_Layers[layer].m_pLayerData = m_pLayerData + layer * m_lLayerSize;
        }
        return true;
    }

    void ZSynth::Reset()
    {
        m_lStartedBuffersCnt = 0;
        m_lStoppedBuffersCnt = 0;
        m_lDelayedBuffersCnt = 0;
        m_lPosesLoadedCnt = 0;
        m_lBufferIdCnt = 0;
        m_lTransferRequestCnt = 0;
        m_lNumChains = 0;
        std::memset(m_PosesBuffers, 0, sizeof(m_PosesBuffers));
        if (m_pWaveHeaders)
        {
            ZUniMemory::Free(m_pWaveHeaders);
            m_pWaveHeaders = nullptr;
        }
        for (int i = 0; m_pBuffers && i < m_lNumBuffers; ++i)
        {
            m_pBuffers[i]->Stop();
            m_pBuffers[i]->Free();
            m_pBuffers[i]->m_VoiceState = VS_READY;
        }
        if (m_pStreamer)
            m_pStreamer->Reset();
        for (ZBufferGroup& group : m_BufferGroups)
        {
            group.m_lNumGrouped = 0;
            group.m_pReferenceBuffer = nullptr;
            group.m_pFirstHeader = nullptr;
            group.m_lPlayCursor = 0;
            group.m_lCurPlaySeg = 0;
            group.m_bNewBuffers = false;
        }
    }

    void ZSynth::Free()
    {
        Reset();
        if (m_pStreamer)
        {
            m_pStreamer->Destroy();
            ZUniMemory::Delete(m_pStreamer);
            m_pStreamer = nullptr;
        }
        ZUniMemory::Delete(m_pListener);
        m_pListener = nullptr;
        for (int i = 0; m_pBuffers && i < m_lNumBuffers; ++i)
            ZUniMemory::Delete(m_pBuffers[i]);
        ZUniMemory::Free(m_pBuffers);
        m_pBuffers = nullptr;
    }

    void ZSynth::InstallStreamWaves(int, const char* _data, const char* _localizedData)
    {
        if (m_pStreamer)
            m_pStreamer->SetAudioStreamFile(_data, _localizedData);
    }

    void ZSynth::CommitFrame()
    {
        if (!m_pCmdFrameBuffer)
            return;
        for (int i = 0; i < m_lNumBuffers; ++i)
            m_pBuffers[i]->m_bFrameClaimed = false;

        auto* command = reinterpret_cast<SSynthCmdBase*>(m_pCmdFrameBuffer);
        bool groupMode = false;
        ZIOStream* sharedGroupStream = nullptr;
        ZBufferGroup* group = nullptr;
        int groupCount = 0;
        while (command->m_lType != 0x10000)
        {
            if (command->m_lType == 1 || command->m_lType == 2 || command->m_lType == 9)
            {
                auto* start = reinterpret_cast<SStartSoundBase*>(command);
                SWaveHeader* wave = GetWaveHeader(start->m_lHeaderOffset);
                if (wave && wave->m_lDataOffset && (!groupMode || groupCount < m_lMaxNumGroupedPlaying))
                {
                    _ZSoundBuffer* buffer = nullptr;
                    if (start->m_lBufferId > 0 && start->m_lBufferId < m_lNumBuffers)
                    {
                        auto* candidate = m_pBuffers[start->m_lBufferId];
                        if (candidate->m_bInUse && candidate->m_rSndObj == start->m_lSndRef)
                            buffer = candidate;
                    }
                    if (!buffer)
                    {
                        for (int i = 0; i < m_lNumBuffers; ++i)
                        {
                            if (m_pBuffers[i]->m_bInUse && m_pBuffers[i]->m_rSndObj == start->m_lSndRef)
                            {
                                buffer = m_pBuffers[i];
                                break;
                            }
                        }
                    }
                    if (!buffer && ((start->m_lFlags & 1) || start->m_bLooping))
                    {
                        buffer = GetBuffer(start->m_lPrio, wave, 0, groupMode && sharedGroupStream);
                        if (buffer)
                        {
                            buffer->Stop();
                            buffer->Free();
                            buffer->m_bLooping = start->m_bLooping != 0;
                            if (!buffer->Create(wave, start->m_lBufferType, 0) ||
                                !buffer->AllocateWaveResource(sharedGroupStream, start))
                            {
                                buffer->Free();
                                buffer = nullptr;
                            }
                            else if (groupMode)
                            {
                                sharedGroupStream = buffer->m_pStream;
                                if (!group)
                                {
                                    group = sharedGroupStream->m_pUserData;
                                    group->m_lPlayCursor = 0;
                                    group->m_lCurPlaySeg = 0;
                                    group->m_lNumGrouped = 0;
                                    group->m_bNewBuffers = false;
                                    if (!group->m_pReferenceBuffer)
                                        group->m_pReferenceBuffer = buffer;
                                }
                            }
                        }
                    }
                    if (buffer)
                    {
                        buffer->m_bFrameClaimed = true;
                        buffer->Update(start);
                        if (groupMode && group)
                        {
                            group->m_Buffers[group->m_lNumGrouped++] = buffer;
                            buffer->m_lGroupType = groupCount++ ? 2 : 1;
                        }
                    }
                }
            }
            else if (command->m_lType == 4 || command->m_lType == 0x50)
            {
                const auto* reference = reinterpret_cast<const SCmdSoundRef*>(command);
                for (int i = 0; i < m_lNumBuffers; ++i)
                {
                    auto* buffer = m_pBuffers[i];
                    if (buffer && buffer->m_bInUse && buffer->m_rSndObj == reference->m_lSndRef)
                    {
                        if (command->m_lType == 4)
                        {
                            buffer->Stop();
                            buffer->Free();
                        }
                        else
                            buffer->PauseCheck(1);
                        break;
                    }
                }
            }
            else if (command->m_lType == 5)
            {
                for (int i = 0; i < m_lNumBuffers; ++i)
                    m_pBuffers[i]->Free();
            }
            else if (command->m_lType == 0x0A)
            {
                const auto* begin = reinterpret_cast<const SGroupStart*>(command);
                groupMode = true;
                sharedGroupStream = nullptr;
                group = nullptr;
                groupCount = 0;
                for (int entry = 0; entry < begin->m_lGroupEntries && groupCount < m_lMaxNumGroupedPlaying; ++entry)
                {
                    const auto& id = begin->m_Entries[entry];
                    for (int i = 0; i < m_lNumBuffers; ++i)
                    {
                        auto* buffer = m_pBuffers[i];
                        if (buffer->m_bInUse && buffer->m_rSndObj == id.m_lGeomRef)
                        {
                            if (!sharedGroupStream)
                            {
                                sharedGroupStream = buffer->m_pStream;
                                group = sharedGroupStream->m_pUserData;
                                group->m_lPlayCursor = buffer->GetPlayCursor();
                                group->m_lCurPlaySeg = buffer->m_lBufferId;
                                group->m_lNumGrouped = 0;
                            }
                            ++groupCount;
                            break;
                        }
                    }
                }
                if (group)
                    group->m_bNewBuffers = groupCount != begin->m_lGroupEntries;
            }
            else if (command->m_lType == 0x0B)
                groupMode = false;
            else if (command->m_lType == 0x30)
            {
                int chainIndex = 0;
                SChain* chain = &m_Chains[0];
                int filterIndex = 0;
                command = reinterpret_cast<SSynthCmdBase*>(reinterpret_cast<char*>(command) + command->m_lSize);
                while (command->m_lType != 0x31)
                {
                    switch (command->m_lType)
                    {
                    case 0x41:
                    {
                        const auto* begin = reinterpret_cast<const SCmdChainBegin*>(command);
                        chain->m_bCreate = begin->m_bCreate;
                        chain->m_lRemapIdx = begin->m_lRemapIdx;
                        filterIndex = 0;
                        break;
                    }
                    case 0x42:
                        chain->m_Filters[filterIndex++] = {reinterpret_cast<SSynthFilterBase*>(command), 1};
                        break;
                    case 0x44:
                        chain->m_Filters[filterIndex++] = {reinterpret_cast<SSynthFilterBase*>(command), 7};
                        break;
                    case 0x70:
                        chain->m_Filters[filterIndex++] = {reinterpret_cast<SSynthFilterBase*>(command), 2};
                        break;
                    case 0x71:
                        chain->m_Filters[filterIndex++] = {nullptr, 3};
                        break;
                    case 0x40:
                        chain->m_lNumFilters = filterIndex;
                        CreateChain(chain, chainIndex++);
                        chain = &m_Chains[chainIndex];
                        filterIndex = 0;
                        break;
                    default:
                        ZASSERT(false);
                        break;
                    }
                    command = reinterpret_cast<SSynthCmdBase*>(reinterpret_cast<char*>(command) + command->m_lSize);
                }
                m_lNumChains = chainIndex;
                CreateChainsEnd();
                CreateFilterChains();
            }
            else if (command->m_lType == 0x32)
            {
                auto* remove = reinterpret_cast<SCmdRemoveChains*>(command);
                RemoveChains(remove->m_Chains, remove->m_lCount);
            }
            else if (command->m_lType == 0x20)
                m_pListener->Update(command);
            else if (command->m_lType == 0x46)
                SetFmvVolume(*reinterpret_cast<int32_t*>(reinterpret_cast<char*>(command) + 8));
            else if (command->m_lType == 0x45)
            {
                const auto* fade = reinterpret_cast<const SCmdLPFade*>(command);
                for (int i = 0; i < m_lNumBuffers; ++i)
                {
                    if (m_pBuffers[i]->m_rSndObj == fade->m_lGeomRef)
                        m_pBuffers[i]->m_lCrossFadePct = fade->m_lFadePct;
                }
            }
            else if (command->m_lType == 0x80)
                PushScene();
            else if (command->m_lType == 0x81)
                PopScene();
            else if (command->m_lType == 0x300)
                StartMemStream();
            else if (command->m_lType == 0x301)
                StopMemStream();
            else if (command->m_lType == 0x1000)
                m_bRunning = false;
            else if (command->m_lType == 0x1200 || command->m_lType == 0x1201)
            {
                for (int i = 0; i < m_lNumBuffers; ++i)
                {
                    if (!m_pBuffers[i]->m_bInUse)
                        continue;
                    if (command->m_lType == 0x1200)
                        m_pBuffers[i]->PauseCheck(1);
                    else
                        m_pBuffers[i]->ResetVolume();
                }
            }
            command = reinterpret_cast<SSynthCmdBase*>(reinterpret_cast<char*>(command) + command->m_lSize);
        }
        *reinterpret_cast<uint32_t*>(m_pCmdFrameBuffer) = 0x10000;
        for (int i = 0; i < m_lNumBuffers; ++i)
        {
            auto* buffer = m_pBuffers[i];
            if (!buffer->m_bFrameClaimed)
                continue;
            ZASSERT(m_lBufferIdCnt < 128);
            m_tBufferId[m_lBufferIdCnt++] = {buffer->m_rSndObj, buffer->m_lBufferIndex};
        }
        SignalCmdFrameProcessed();
    }

    void ZSynth::Render()
    {
        RenderBuffers();
        if (!SoundEngineReady())
            return;

        InitFrame();
        BuildFrame();
        auto* end = reinterpret_cast<SSynthCmdBase*>(m_pCmd);
        end->m_lType = 0x10000;
        end->m_lSize = sizeof(SSynthCmdBase);
        m_pCmd += sizeof(SSynthCmdBase);
        SendFrame();
        m_lTransferRequestCnt = 0;
    }

    bool ZSynth::CopyWaveData(_ZSoundBuffer*)
    {
        return true;
    }

    _ZSoundBuffer* ZSynth::Duplicate(_ZSoundBuffer*)
    {
        return nullptr;
    }

    void ZSynth::StartMemStream()
    {
        m_bMemStreamRunning = true;
        m_bMemStreamNeedData = true;
        m_lMemStreamAddr = 0;
        m_lMemStreamSize = m_lMemStreamBufferSize;
        m_lMemStreamReceived = 0;
        m_lMemStreamBytesPlayed = 0;
        if (m_pMemStreamBuffer && m_lMemStreamBufferSize)
            std::memset(m_pMemStreamBuffer, 0, m_lMemStreamBufferSize);
    }

    void ZSynth::StopMemStream()
    {
        m_bMemStreamRunning = false;
        m_bMemStreamReady = false;
        if (m_pMemStreamBuffer && m_lMemStreamBufferSize)
            std::memset(m_pMemStreamBuffer, 0, m_lMemStreamBufferSize);
    }

    void ZSynth::SetFmvVolume(int _volume)
    {
        m_lFmvVolume = _volume;
    }

    void ZSynth::RemoveFromMetaTab(_ZSoundBuffer* _buffer)
    {
        for (_ZSoundBuffer*& entry : m_PosesBuffers)
        {
            if (entry == _buffer)
                entry = nullptr;
        }
    }
}
