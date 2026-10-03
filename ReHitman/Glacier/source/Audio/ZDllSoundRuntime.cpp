#include <Glacier/Audio/ZDllSound.h>
#include <Glacier/Audio/ZSoundGraph.h>
#include <Glacier/Audio/ZSynth.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/ZSTL/LINKSORTREFTAB.h>

#include <Windows.h>

#include <cmath>
#include <cstring>

namespace Glacier
{
    ZDllSound::~ZDllSound()
    {
        delete m_pSoundGraph;
        m_pSoundGraph = nullptr;
    }

    void ZDllSound::End() { FreeSynth(); }
    bool ZDllSound::AddNewPlayBuffer(ZSoundObject* _object)
    {
        return _object && AddNewPlayBuffer(SPtrToRef(_object));
    }
    bool ZDllSound::AddNewPlayBuffer(ZREF _reference)
    {
        if (!_reference || !SRefToPtr(_reference))
            return false;
        m_RefTabB.AddUnique(_reference);
        return true;
    }
    void ZDllSound::RemovePlayBuffer(ZREF _reference)
    {
        m_RefTabB.RemoveIfExists(_reference);
    }
    void ZDllSound::CalcPriority(ZSoundObject* _object)
    {
        if (!_object)
            return;

        float priority = _object->GetCalculatedVolumeAttenuation();
        const uint32_t sourceType = static_cast<uint32_t>(_object->m_eSourceType);
        if (sourceType == 0 || sourceType == 10)
        {
            const float x = m_ListenerPosition.x - _object->m_vPosition.x;
            const float y = m_ListenerPosition.y - _object->m_vPosition.y;
            const float z = m_ListenerPosition.z - _object->m_vPosition.z;
            float distance = std::sqrt(x * x + y * y + z * z);
            if (distance == 0.0f)
                distance = 1.0f;
            float minimum = _object->m_fMinDistFactor * _object->m_fMinDist;
            if (minimum == 0.0f)
                minimum = 1.0f;
            priority /= (std::max)(distance / minimum, 1.0f);
        }

        if (priority > 0.0f)
        {
            ZASSERT(_object->m_dwPriority <= 9);
            priority += static_cast<float>(_object->m_dwPriority) * 100.0f;
        }
        _object->m_fPrio = (priority + 10.0f) * 10.0f;
    }
    void ZDllSound::SetMaxGraphReflections(uint8_t _count)
    {
        if (m_pSoundGraph)
            m_pSoundGraph->SetMaxReflections(_count);
    }
    void ZDllSound::InstallSoundGraph(char* _data, int _size)
    {
        if (!_data)
            return;
        delete m_pSoundGraph;
        m_pSoundGraph = new ZSoundGraph();
        m_pSoundGraph->InstallLoadedGraph(_data, _size);
    }
    void ZDllSound::InitializeSoundGraph()
    {
        if (m_pSoundGraph)
            m_pSoundGraph->Initialize();
    }
    void ZDllSound::InstallWavesWait()
    {
        while (m_bSoundInitialized && !SynthModuleReady())
            Sleep(0);
    }
    bool ZDllSound::InstallWaves(int _size, const char* _fileName)
    {
        if (!m_bSoundInitialized)
            return true;
        if (_size == -1 || !_fileName)
            return false;
        auto* command = reinterpret_cast<SSynthCmdBase*>(m_pSynthCmd);
        command->m_lType = 0x100;
        command->m_lSize = 0x8C;
        *reinterpret_cast<int32_t*>(m_pSynthCmd + 8) = _size;
        std::strcpy(m_pSynthCmd + 12, _fileName);
        m_pSynthCmd += command->m_lSize;
        return true;
    }
    bool ZDllSound::InstallSounds(char* _data, uint32_t _size)
    {
        m_pPackedData = _data;
        m_lPackedDataSize = _size;
        return true;
    }
    bool ZDllSound::InstallStreamWaves(int _size, const char* _fileName)
    {
        if (!m_bSoundInitialized)
            return true;
        if (_size == -1 || !_fileName)
            return false;
        auto* command = reinterpret_cast<SSynthCmdBase*>(m_pSynthCmd);
        command->m_lType = 0x102;
        command->m_lSize = 0x10C;
        *reinterpret_cast<int32_t*>(m_pSynthCmd + 8) = _size;
        std::strcpy(m_pSynthCmd + 12, _fileName);
        m_pSynthCmd[140] = '\0';
        m_pSynthCmd += command->m_lSize;
        return true;
    }
    bool ZDllSound::InstallWaveHeaders(int _size, const char* _fileName)
    {
        if (!m_bSoundInitialized)
            return true;
        if (_size == -1 || !_fileName)
            return false;
        auto* command = reinterpret_cast<SSynthCmdBase*>(m_pSynthCmd);
        command->m_lType = 0x101;
        command->m_lSize = 0x8C;
        *reinterpret_cast<int32_t*>(m_pSynthCmd + 8) = _size;
        std::strcpy(m_pSynthCmd + 12, _fileName);
        m_pSynthCmd += command->m_lSize;
        return true;
    }
    void ZDllSound::InitFrame()
    {
        m_pSynthCmd = m_pSynthCmdBuffer;
        auto* end = reinterpret_cast<SSynthCmdBase*>(m_pSynthCmd);
        end->m_lType = 0x10000;
        end->m_lSize = sizeof(SSynthCmdBase);
    }
    bool ZDllSound::RenderFrame()
    {
        if (!m_bSoundInitialized)
            return false;
        if (SynthModuleReady())
            ProcessSynthCmdArray();
        auto* end = reinterpret_cast<SSynthCmdBase*>(m_pSynthCmd);
        end->m_lType = 0x10000;
        end->m_lSize = sizeof(SSynthCmdBase);
        return SendCmdFrameBuffer();
    }
    void ZDllSound::SetPlayerListener(ZGEOM& _listener) { m_pPlayerListener = &_listener; }
    ZGEOM* ZDllSound::GetPlayerListener() { return m_pPlayerListener; }
    void ZDllSound::SetCameraListener(ZGEOM& _listener) { m_pCameraListener = &_listener; }
    void ZDllSound::SetDefaultCameraListener() { m_pCameraListener = nullptr; }
    ZGEOM* ZDllSound::GetCameraListener() { return m_pCameraListener; }
    EListenerMode ZDllSound::GetListenerMode() { return m_eListenerMode; }
    void ZDllSound::SetFmvVolume(int _volume) { m_lFmvVolume = _volume; }
    ZREF ZDllSound::AddSound3d(ZGEOM* _owner, int _soundIndex,
        float* _direction, float* _position)
    {
        ZSoundObject* object = AddSound(_soundIndex, _owner, _direction, _position);
        return SPtrToRef(object);
    }
    ZREF ZDllSound::AddSound2d(int _soundIndex)
    {
        ZSoundObject* object = AddSound(_soundIndex, nullptr, nullptr, nullptr);
        if (!object)
            return 0;
        object->m_eSourceType = ZSoundObject::SOURCE_2D;
        object->m_OuterConeVolume = 0;
        object->m_fInnerConeAngle = 360.0f;
        object->m_fOuterConeAngle = 360.0f;
        return SPtrToRef(object);
    }
    bool ZDllSound::MemStreamCreate(char*) { return false; }
    void ZDllSound::MemStreamDestroy() {}
    int32_t ZDllSound::MemStreamPushData(char*, int) { return 1; }
    int32_t ZDllSound::MemStreamBufferSize() { return 0; }
    ZROOM* ZDllSound::GetListenerRoom()
    {
        ZGEOM* listener = GetPlayerListener();
        if (!listener)
            listener = GetCameraListener();
        return listener && listener->BaseGeom() ? listener->BaseGeom()->GetOwnerRoom() : nullptr;
    }
    bool ZDllSound::RenderFrame(REFTAB32& _sources)
    {
        REFTAB32 preAudible;
        REFTAB32 audible;
        REFTAB32 suspended;
        REFTAB32 deleted;
        LINKSORTREFTAB priorities(32, 0);

        InitFrame();
        GetDeletedSounds(_sources, deleted);
        GetAudiable(_sources, preAudible, audible, suspended);
        CalcPrioTab(audible, priorities);
        BuildFrame(priorities, deleted, suspended);
        return SendCmdFrameBuffer();
    }
    ZGROUP* ZDllSound::GetCurrentRoom(ZGEOM* _object)
    {
        return _object && _object->BaseGeom() ? _object->BaseGeom()->GetOwnerRoom() : nullptr;
    }
    int32_t ZDllSound::GetMemStreamPlayCursor() { return 0; }
    void ZDllSound::TransferMemStreamData(char*, int) {}
    void ZDllSound::InitAudioStream() {}
    void ZDllSound::StartAudioStream() {}
    void ZDllSound::StopAudioStream() {}
    bool ZDllSound::RunAudioStream() { return false; }
    void ZDllSound::CalcPrioTab(REFTAB32& _sources, LINKSORTREFTAB& _priorities)
    {
        RefRun run{};
        _sources.RunInitNxtRef(&run);
        for (ZREF reference = _sources.RunNxtRef(&run); run;
            reference = _sources.RunNxtRef(&run))
        {
            ZSoundObject* object = m_ObjectManager.ConvRefToPtr(reference);
            if (!object || !object->m_rSound)
                continue;
            ZAudioTypeBase::ZPackedBase* packed = GetPackedObject(object->m_rSound);
            if (!GetWave(packed))
                continue;
            CalcPriority(object);
            _priorities.AddSort(reference, -object->m_fPrio, 0);
        }
    }
    void ZDllSound::GetAudiable(REFTAB32& _sources, REFTAB32& _preAudible,
        REFTAB32& _audible, REFTAB32& _suspended)
    {
        RefRun run{};
        _sources.RunInitNxtRef(&run);
        for (ZREF reference = _sources.RunNxtRef(&run); run;
            reference = _sources.RunNxtRef(&run))
        {
            ZSoundObject* object = m_ObjectManager.ConvRefToPtr(reference);
            if (!object)
                continue;

            object->Update();
            if ((object->m_lSoundFlags & 0x800) != 0 || object->m_eState == STATE_DEPRICATED)
                continue;
            if (g_pSysInterface && g_pSysInterface->m_fRealTime.secs <
                object->m_fCreateTime.secs + static_cast<int32_t>(object->m_fDelay * 1024.0f))
            {
                object->m_eState = STATE_SUSPENDED;
                continue;
            }
            if (object->m_eState == STATE_SUSPENDED || object->m_eState == STATE_PENDING)
                object->m_eState = STATE_STARTPLAY;

            const uint32_t sourceType = static_cast<uint32_t>(object->m_eSourceType);
            if (sourceType >= ZSoundObject::SOURCE_2D_CULLED &&
                sourceType <= ZSoundObject::SOURCE_SUB)
            {
                _audible.Add(reference);
                continue;
            }

            if (sourceType == ZSoundObject::SOURCE_3D || sourceType == ZSoundObject::SOURCE_3DPAN)
            {
                const float x = object->m_vPosition.x - m_ListenerPosition.x;
                const float y = object->m_vPosition.y - m_ListenerPosition.y;
                const float z = object->m_vPosition.z - m_ListenerPosition.z;
                const float distanceSquared = x * x + y * y + z * z;
                if (object->m_iMaxDistModel &&
                    distanceSquared > static_cast<float>(object->m_iMaxDistModel * 100) *
                        static_cast<float>(object->m_iMaxDistModel * 100))
                {
                    object->m_eState = STATE_SUSPENDED;
                    if (object->BufferIdValid())
                        _suspended.Add(reference);
                    continue;
                }
            }
            _preAudible.Add(reference);
        }

        ZROOM* listenerRoom = GetListenerRoom();
        if (m_pSoundGraph && !m_bMultiListeners && listenerRoom)
            m_pSoundGraph->GetAudiable(listenerRoom, _preAudible, _audible);
        else
        {
            RefRun pending{};
            _preAudible.RunInitNxtRef(&pending);
            for (ZREF reference = _preAudible.RunNxtRef(&pending); pending;
                reference = _preAudible.RunNxtRef(&pending))
            {
                _audible.Add(reference);
            }
        }
    }
    void ZDllSound::BuildFrame(LINKSORTREFTAB& _priorities, REFTAB32& _deleted, REFTAB32& _stopped)
    {
        struct SStopSound : SSynthCmdBase
        {
            ZREF m_lSndRef;
            int32_t m_lPreempted;
            int32_t m_lBufferId;
        };

        RefRun activeRun{};
        m_RefTabB.RunInitNxtRef(&activeRun);
        for (ZREF reference = m_RefTabB.RunNxtRef(&activeRun); activeRun;
            reference = m_RefTabB.RunNxtRef(&activeRun))
        {
            ZSoundObject* object = m_ObjectManager.ConvRefToPtr(reference);
            if (!object || (object->m_lSoundFlags & 0x200000) == 0)
                continue;
            auto* command = reinterpret_cast<SCmdLPFade*>(m_pSynthCmd);
            command->m_lType = 0x45;
            command->m_lSize = sizeof(SCmdLPFade);
            command->m_lGeomRef = reference;
            command->m_lFadePct = object->m_lLowpassPct;
            m_pSynthCmd += command->m_lSize;
        }

        auto appendStops = [this](REFTAB32& _objects, bool _requireBuffer)
        {
            RefRun run{};
            _objects.RunInitNxtRef(&run);
            for (ZREF reference = _objects.RunNxtRef(&run); run;
                reference = _objects.RunNxtRef(&run))
            {
                ZSoundObject* object = m_ObjectManager.ConvRefToPtr(reference);
                if (_requireBuffer && object && !object->BufferIdValid())
                    continue;
                auto* command = reinterpret_cast<SStopSound*>(m_pSynthCmd);
                command->m_lType = 4;
                command->m_lSize = sizeof(SStopSound);
                command->m_lSndRef = reference;
                command->m_lPreempted = 0;
                command->m_lBufferId = object ? object->GetBufferId() : -1;
                if (object)
                    object->ClearBufferId();
                m_pSynthCmd += command->m_lSize;
            }
        };
        appendStops(_deleted, false);
        appendStops(_stopped, true);

        if (m_pSoundGraph && m_pSoundGraph->m_rtObsoleteChains.Count())
        {
            const int count = m_pSoundGraph->m_rtObsoleteChains.Count();
            auto* command = reinterpret_cast<SCmdRemoveChains*>(m_pSynthCmd);
            command->m_lType = 0x32;
            command->m_lSize = 12 + count * sizeof(int32_t);
            command->m_lCount = count;
            RefRun obsolete{};
            m_pSoundGraph->m_rtObsoleteChains.RunInitNxtRef(&obsolete);
            int index = 0;
            for (uint32_t chain = m_pSoundGraph->m_rtObsoleteChains.RunNxtRef(&obsolete); obsolete;
                chain = m_pSoundGraph->m_rtObsoleteChains.RunNxtRef(&obsolete))
            {
                command->m_Chains[index++] = static_cast<int32_t>(chain);
            }
            m_pSynthCmd += command->m_lSize;
        }

        AddFilterChainCommands();
        AddListenerCommands();
        AddSourceCommands(_priorities);

        auto* fmv = reinterpret_cast<SCmdSoundRef*>(m_pSynthCmd);
        fmv->m_lType = 0x46;
        fmv->m_lSize = sizeof(SCmdSoundRef);
        fmv->m_lSndRef = static_cast<uint32_t>(m_lFmvVolume);
        m_pSynthCmd += fmv->m_lSize;

        if (m_bStopMemStream)
        {
            m_bStopMemStream = false;
            auto* command = reinterpret_cast<SSynthCmdBase*>(m_pSynthCmd);
            command->m_lType = 0x301;
            command->m_lSize = sizeof(SSynthCmdBase);
            m_pSynthCmd += command->m_lSize;
        }
        if (m_bStartMemStream)
        {
            m_bStartMemStream = false;
            auto* command = reinterpret_cast<SSynthCmdBase*>(m_pSynthCmd);
            command->m_lType = 0x300;
            command->m_lSize = sizeof(SSynthCmdBase);
            m_pSynthCmd += command->m_lSize;
        }

        auto* end = reinterpret_cast<SSynthCmdBase*>(m_pSynthCmd);
        end->m_lType = 0x10000;
        end->m_lSize = sizeof(SSynthCmdBase);
    }
    void ZDllSound::AddSourceCommands(LINKSORTREFTAB& _sources)
    {
        REFTAB32 layered;
        int directCount = 0;
        RefRun run{};
        _sources.RunInitNxtRef(&run);
        for (ZREF reference = _sources.RunNxtRef(&run); run;
            reference = _sources.RunNxtRef(&run))
        {
            ZSoundObject* object = m_ObjectManager.ConvRefToPtr(reference);
            if (!object)
                continue;

            ZAudioTypeBase::ZPackedBase* packedSound = GetPackedObject(object->m_rSound);
            if (!packedSound || packedSound->m_Type != ZAudioTypes::Sound)
                continue;
            SWave* wave = GetWave(packedSound);
            if (!wave)
                continue;

            if ((object->m_lSoundFlags & 0x800000) != 0)
            {
                object->m_lSoundFlags |= 0x1000000;
                layered.Add(reference);
                continue;
            }
            if (directCount > 64)
                break;
            ++directCount;
            AppendSourceCommand(object, reference, wave);
        }

        RefRun layeredRun{};
        layered.RunInitNxtRef(&layeredRun);
        for (ZREF masterRef = layered.RunNxtRef(&layeredRun); layeredRun;
            masterRef = layered.RunNxtRef(&layeredRun))
        {
            ZSoundObject* master = m_ObjectManager.ConvRefToPtr(masterRef);
            if (!master || (master->m_lSoundFlags & 0x800000) == 0)
                continue;

            LINKSORTREFTAB group(32, 0);
            for (const SRefLink* link = m_AllocRef.GetLink(master->m_lChainIdxGroup);
                link; link = m_AllocRef.GetLink(link->m_lNext))
            {
                ZSoundObject* member = m_ObjectManager.ConvRefToPtr(link->m_rRef);
                if (member && (member->m_lSoundFlags & 0x1000000) != 0)
                {
                    member->m_lSoundFlags &= ~0x1000000u;
                    group.AddSort(link->m_rRef, -(member->m_fPrio + 10.0f), 0);
                }
            }

            auto* begin = reinterpret_cast<SGroupStart*>(m_pSynthCmd);
            begin->m_lType = 0x0A;
            begin->m_lGroupEntries = group.Count();
            begin->m_lSize = 12 + sizeof(SBufferId) * begin->m_lGroupEntries;
            int entry = 0;
            RefRun groupRun{};
            group.RunInitNxtRef(&groupRun);
            for (ZREF reference = group.RunNxtRef(&groupRun); groupRun;
                reference = group.RunNxtRef(&groupRun))
            {
                ZSoundObject* member = m_ObjectManager.ConvRefToPtr(reference);
                begin->m_Entries[entry++] = {reference, member ? member->GetBufferId() : -1};
            }
            m_pSynthCmd += begin->m_lSize;

            group.RunInitNxtRef(&groupRun);
            for (ZREF reference = group.RunNxtRef(&groupRun); groupRun;
                reference = group.RunNxtRef(&groupRun))
            {
                ZSoundObject* member = m_ObjectManager.ConvRefToPtr(reference);
                if (!member)
                    continue;
                ZAudioTypeBase::ZPackedBase* packed = GetPackedObject(member->m_rSound);
                SWave* wave = GetWave(packed);
                if (wave)
                    AppendSourceCommand(member, reference, wave);
            }

            auto* end = reinterpret_cast<SSynthCmdBase*>(m_pSynthCmd);
            end->m_lType = 0x0B;
            end->m_lSize = sizeof(SSynthCmdBase);
            m_pSynthCmd += end->m_lSize;
        }
    }
}
