#include <Glacier/Audio/ZDllSound.h>
#include <Glacier/Audio/ZSoundGraph.h>
#include <Glacier/Audio/ZSynth.h>
#include <Glacier/Audio/ZWintelRoomReverb.h>
#include <Glacier/Geom/ZGEOM.h>

#include <cstring>

namespace Glacier
{
    void ZDllSound::Init() { /* TODO: Finish me */ }
    void ZDllSound::PushScene(const char*) { /* TODO: Finish me */ }
    void ZDllSound::PopScene() { /* TODO: Finish me */ }
    void ZDllSound::AllocSequenceStart() { /* TODO: Finish me */ }
    void ZDllSound::AllocSequenceEnd() { /* TODO: Finish me */ }
    void ZDllSound::InsertStart() { /* TODO: Finish me */ }
    void ZDllSound::InsertEnd() { /* TODO: Finish me */ }
    void ZDllSound::CrashFree() { /* TODO: Finish me */ }
    void ZDllSound::PrintStatus() { /* TODO: Finish me */ }
    void ZDllSound::Initialize() { /* TODO: Finish me */ }
    void ZDllSound::Init2() { /* TODO: Finish me */ }
    uint32_t ZDllSound::GetMapping(uint32_t, uint32_t) { /* TODO: Finish me */ return 0; }
    void ZDllSound::AdjustExit(ZROOM*, int, float) { /* TODO: Finish me */ }
    SExit* ZDllSound::GetExit(uint32_t) { /* TODO: Finish me */ return nullptr; }
    void ZDllSound::RegisterAudioEvents(SAudioEvent*, SAudioEventNoiseLevel*) { /* TODO: Finish me */ }
    void ZDllSound::ActorRegister(ZGEOM*) { /* TODO: Finish me */ }
    void ZDllSound::ActorRemove(ZGEOM*) { /* TODO: Finish me */ }
    void ZDllSound::DispatchSoundEvents() { /* TODO: Finish me */ }
    char* ZDllSound::TEMPCALCSOUNDGRAPH(uint32_t*) { /* TODO: Finish me */ return nullptr; }
    void ZDllSound::SetDefaultEnv(ZREF) { /* TODO: Finish me */ }
    void ZDllSound::SetTemporaryEnv(ZREF) { /* TODO: Finish me */ }
    void ZDllSound::MuteAllWaves(float) { /* TODO: Finish me */ }
    bool ZDllSound::AddEvent(ZGROUP*, float*, int, uint32_t, uint32_t, uint32_t) { /* TODO: Finish me */ return false; }
    bool ZDllSound::IsListenerFreeCam(ZGEOM*) { /* TODO: Finish me */ return false; }
    void ZDllSound::InitSoundPack() { /* TODO: Finish me */ }
    void ZDllSound::InitSoundPackPS2() { /* TODO: Finish me */ }
    void ZDllSound::InitSoundPackXBox() { /* TODO: Finish me */ }
    void ZDllSound::InitSoundPackGC() { /* TODO: Finish me */ }
    void ZDllSound::InitSoundPackXenon() { /* TODO: Finish me */ }
    bool ZDllSound::IsPlayingMono(ZREF) { /* TODO: Finish me */ return false; }
    void ZDllSound::SetMasterVolume(int) { /* TODO: Finish me */ }
    ZSoundObject* ZDllSound::AddSound(int _soundIndex, ZGEOM* _owner,
        float* _direction, float* _position)
    {
        if (!_soundIndex || !GetPackedObject(_soundIndex))
            return nullptr;
        ZSoundObject* object = AllocateObject();
        if (!object)
            return nullptr;
        object->m_rSound = _soundIndex;
        object->m_lOrigSound = _soundIndex;
        object->m_rGeomRef = _owner ? _owner->GetRef() : 0;
        object->m_fMinDist = 100.0f;
        object->m_fMinDistFactor = 1.0f;
        if (_position)
            object->m_vLocalPos = {_position[0], _position[1], _position[2]};
        if (_direction)
            object->m_vLocalOrient = {_direction[0], _direction[1], _direction[2]};
        object->m_eState = STATE_STARTPLAY;
        if (!AddNewPlayBuffer(object))
        {
            DeleteSoundPtr(object);
            return nullptr;
        }
        // TODO: Finish this place after packed audio controllers will be reversed
        return object;
    }
    void ZDllSound::NotifyStopped(ZREF, SSndMsg*) { /* TODO: Finish me */ }
    void ZDllSound::NotifyStarted(ZREF, SSourceStarted*) { /* TODO: Finish me */ }
    void ZDllSound::GetRefChainObjects(uint32_t, REFTAB32*) { /* TODO: Finish me */ }
    void ZDllSound::Pause(bool, bool) { /* TODO: Finish me */ }
    void ZDllSound::DrawSources(ZDrawDebugRender*) { /* TODO: Finish me */ }
    float ZDllSound::GetWaveDuration(int) { /* TODO: Finish me */ return 0.0f; }
    void ZDllSound::CalculateSoundGraph() { /* TODO: Finish me */ }
    char* ZDllSound::GetSoundGraph(uint32_t*) { /* TODO: Finish me */ return nullptr; }
    char* ZDllSound::GetRemapTable(uint32_t*) { /* TODO: Finish me */ return nullptr; }
    void ZDllSound::Reset() { /* TODO: Finish me */ }
    void ZDllSound::SetAllVolumes(float) { /* TODO: Finish me */ }
    int32_t ZDllSound::IsDVDReady() { /* TODO: Finish me */ return 0; }
    void ZDllSound::SendDVDReadyRequest() { /* TODO: Finish me */ }
    void ZDllSound::SendDVDUnlockRequest() { /* TODO: Finish me */ }
    bool ZDllSound::RenderFrameMulti(REFTAB32&, LINKSORTREFTAB&) { /* TODO: Finish me */ return false; }
    int32_t ZDllSound::SetMasterPan(int) { /* TODO: Finish me */ return 0; }
    void ZDllSound::UpdateBufferVolumes() { /* TODO: Finish me */ }
    int32_t ZDllSound::CalcVolume(float) { /* TODO: Finish me */ return 0; }
    int32_t ZDllSound::CalcVolume(float, float, float) { /* TODO: Finish me */ return 0; }
    float ZDllSound::CalcDelayTime(float) { /* TODO: Finish me */ return 0.0f; }
    ZGROUP* ZDllSound::ParentRoom(ZGEOM*) { /* TODO: Finish me */ return nullptr; }
    bool ZDllSound::SetEnvironment() { /* TODO: Finish me */ return false; }
    void ZDllSound::StopBuffers() { /* TODO: Finish me */ }
    void ZDllSound::SendSynthCmd(int) { /* TODO: Finish me */ }
    bool ZDllSound::SendCmdFrameBuffer() { /* TODO: Finish me */ return false; }
    void ZDllSound::GetDeletedSounds(REFTAB32&, REFTAB32&) { /* TODO: Finish me */ }
    void ZDllSound::AddFilterChainCommands()
    {
        if (!m_pSoundGraph)
            return;

        auto* reset = reinterpret_cast<SSynthCmdBase*>(m_pSynthCmd);
        reset->m_lType = 0x30;
        reset->m_lSize = sizeof(SSynthCmdBase);
        m_pSynthCmd += reset->m_lSize;

        for (int i = 0; i < m_pSoundGraph->m_lNumPathes; ++i)
        {
            const SPath& path = m_pSoundGraph->m_Pathes[i];
            auto* begin = reinterpret_cast<SCmdChainBegin*>(m_pSynthCmd);
            begin->m_lType = 0x41;
            begin->m_lSize = sizeof(SCmdChainBegin);
            begin->m_bCreate = path.m_bNew;
            begin->m_lRemapIdx = path.m_lRemapIdx;
            m_pSynthCmd += begin->m_lSize;

            for (int filterIndex = 0; filterIndex < path.m_lNumConnections; ++filterIndex)
            {
                const SFilter& filter = path.m_Filters[filterIndex];
                if (filter.m_lType == 1 && filter.m_lIdx < m_pSoundGraph->m_lNumExits)
                {
                    auto* command = reinterpret_cast<SCmdOcclusionWintel*>(m_pSynthCmd);
                    command->m_lType = 0x42;
                    command->m_lSize = sizeof(SCmdOcclusionWintel);
                    command->m_lNextFilter = 0;
                    command->m_fOpenness = m_pSoundGraph->m_pExits[filter.m_lIdx].m_fOpenness;
                    command->m_lExclusion = 0;
                    m_pSynthCmd += command->m_lSize;
                }
                else if (filter.m_lType == 2 && filter.m_lIdx)
                {
                    auto* command = reinterpret_cast<SSynthFilterBase*>(m_pSynthCmd);
                    command->m_lType = 0x70;
                    command->m_lSize = 0x7C;
                    command->m_lNextFilter = 0;
                    std::memcpy(m_pSynthCmd + 12,
                        reinterpret_cast<const void*>(filter.m_lIdx), sizeof(ZWintelRoomReverb));
                    m_pSynthCmd += command->m_lSize;
                }
                else if (filter.m_lType == 3)
                {
                    auto* command = reinterpret_cast<SSynthFilterBase*>(m_pSynthCmd);
                    command->m_lType = 0x71;
                    command->m_lSize = sizeof(SSynthFilterBase);
                    command->m_lNextFilter = 0;
                    m_pSynthCmd += command->m_lSize;
                }
                else if (filter.m_lType == 5)
                {
                    // TODO: Finish this place after ZSoundGraph environment morph will be reversed
                }
            }

            auto* end = reinterpret_cast<SSynthCmdBase*>(m_pSynthCmd);
            end->m_lType = 0x40;
            end->m_lSize = sizeof(SSynthCmdBase);
            m_pSynthCmd += end->m_lSize;
        }

        auto* finish = reinterpret_cast<SSynthCmdBase*>(m_pSynthCmd);
        finish->m_lType = 0x31;
        finish->m_lSize = sizeof(SSynthCmdBase);
        m_pSynthCmd += finish->m_lSize;
    }
    char* ZDllSound::GetSynthCmdArray() { /* TODO: Finish me */ return nullptr; }
    void ZDllSound::ProcessSynthCmdArray() { /* TODO: Finish me */ }
    bool ZDllSound::ProcessSynthCmd(const char*) { /* TODO: Finish me */ return false; }
    void ZDllSound::ResetSynthReady() { /* TODO: Finish me */ }
    void ZDllSound::SignalSoundEngineReady() { /* TODO: Finish me */ }
    bool ZDllSound::FrameReady() { /* TODO: Finish me */ return false; }
}
