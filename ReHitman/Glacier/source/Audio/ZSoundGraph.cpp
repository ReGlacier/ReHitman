#include <Glacier/Audio/ZSoundGraph.h>
#include <Glacier/Audio/ZDllSound.h>
#include <Glacier/Audio/ZSoundObject.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/System/ZSysInterface.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <new>
#include <type_traits>

namespace Glacier
{
    namespace
    {
        ZVector3 Subtract(const ZVector3& _left, const ZVector3& _right)
        {
            return {_left.x - _right.x, _left.y - _right.y, _left.z - _right.z};
        }

        float Dot(const ZVector3& _left, const ZVector3& _right)
        {
            return _left.x * _right.x + _left.y * _right.y + _left.z * _right.z;
        }

        ZVector3 Normalize(const ZVector3& _value)
        {
            const float length = std::sqrt(Dot(_value, _value));
            return length > 0.0f ? ZVector3{_value.x / length, _value.y / length, _value.z / length} : ZVector3{};
        }

        ZVector3 Cross(const ZVector3& _left, const ZVector3& _right)
        {
            return {_left.y * _right.z - _left.z * _right.y,
                _left.z * _right.x - _left.x * _right.z,
                _left.x * _right.y - _left.y * _right.x};
        }
    }

    ZSoundGraph::ZSoundGraph()
        : m_pLoadedGraphData(nullptr)
        , m_lLoadedGraphSize(0)
        , m_fMorphFactor(0.0f)
        , m_lCurrentRoomIdx(0)
        , m_lMorphRoomIdx(0)
        , m_bEnvMorph(false)
        , m_lMaxReflections(5)
        , m_lNumPathes(0)
        , m_lNumExits(0)
        , m_lNumRooms(0)
        , m_lNumExitPairs(0)
        , m_lNumConnections(0)
        , m_pExits(nullptr)
        , m_pExitPairs(nullptr)
        , m_pRooms(nullptr)
        , m_pConnections(nullptr)
        , m_pExitLookup(nullptr)
        , m_lNumLookupEntries(0)
        , m_bActivateAll(false)
    {
        std::memset(m_Pathes, 0, sizeof(m_Pathes));
    }

    ZSoundGraph::~ZSoundGraph()
    {
        CleanUp();
    }

    void ZSoundGraph::Initialize()
    {
        if (!m_pLoadedGraphData)
            return;

        const char* cursor = m_pLoadedGraphData;
        std::memcpy(&m_lNumExits, cursor, sizeof(uint32_t));
        cursor += sizeof(uint32_t);
        std::memcpy(&m_lNumExitPairs, cursor, sizeof(uint32_t));
        cursor += sizeof(uint32_t);
        std::memcpy(&m_lNumRooms, cursor, sizeof(uint32_t));
        cursor += sizeof(uint32_t);
        std::memcpy(&m_lNumConnections, cursor, sizeof(uint32_t));
        cursor += sizeof(uint32_t);
        std::memcpy(&m_lNumLookupEntries, cursor, sizeof(uint32_t));
        cursor += sizeof(uint32_t);

        auto copyArray = [&cursor](auto*& _destination, uint32_t _count)
        {
            using Element = std::remove_pointer_t<std::remove_reference_t<decltype(_destination)>>;
            if (!_count)
                return;
            _destination = new Element[_count];
            std::memcpy(_destination, cursor, sizeof(Element) * _count);
            cursor += sizeof(Element) * _count;
        };

        copyArray(m_pExits, m_lNumExits);
        copyArray(m_pExitPairs, m_lNumExitPairs);
        copyArray(m_pRooms, m_lNumRooms);
        copyArray(m_pConnections, m_lNumConnections);
        copyArray(m_pExitLookup, static_cast<uint32_t>(m_lNumLookupEntries));

        ::operator delete(m_pLoadedGraphData);
        m_pLoadedGraphData = nullptr;
        m_lLoadedGraphSize = 0;
    }

    void ZSoundGraph::CleanUp()
    {
        delete[] m_pExits;
        delete[] m_pExitPairs;
        delete[] m_pRooms;
        delete[] m_pConnections;
        delete[] m_pExitLookup;
        ::operator delete(m_pLoadedGraphData);
        m_pExits = nullptr;
        m_pExitPairs = nullptr;
        m_pRooms = nullptr;
        m_pConnections = nullptr;
        m_pExitLookup = nullptr;
        m_pLoadedGraphData = nullptr;
        m_lNumExits = 0;
        m_lNumRooms = 0;
        m_lNumExitPairs = 0;
        m_lNumConnections = 0;
        m_lNumLookupEntries = 0;
        m_lLoadedGraphSize = 0;
        m_lNumPathes = 0;
        m_bActivateAll = false;
        m_rtObsoleteChains.Clear();
    }

    void ZSoundGraph::GetAudiable(ZROOM* _room, REFTAB32& _sources, REFTAB32& _audible)
    {
        if (!_room || !g_pSysInterface || !g_pSysInterface->m_pSoundDll)
            return;
        auto* soundDll = static_cast<ZDllSound*>(g_pSysInterface->m_pSoundDll);
        m_lCurrentRoomIdx = _room->m_lSoundGraphId;
        SPath temporary[32]{};
        int count = 1;
        temporary[0].m_lNumConnections = 1;
        temporary[0].m_tConnections[0] = _room->m_lAudioFilter;
        temporary[0].m_Filters[0] = {static_cast<uint32_t>(_room->m_lAudioFilter),
            _room->m_lAudioFilter ? static_cast<uint32_t>(m_bEnvMorph ? 5 : 2) : 3u, 0.0f};

        RefRun run{};
        _sources.RunInitNxtRef(&run);
        for (ZREF reference = _sources.RunNxtRef(&run); run;
            reference = _sources.RunNxtRef(&run))
        {
            ZSoundObject* object = soundDll->SRefToPtr(reference);
            if (!object)
                continue;
            object->ResetPathInfo();
            ZGEOM* geom = ZGEOM::RefToPtr(object->m_rGeomRef);
            ZROOM* owner = geom && geom->BaseGeom() ? geom->BaseGeom()->GetOwnerRoom() : nullptr;
            if (!owner || owner == _room || m_bActivateAll)
            {
                object->AddPath(0);
                _audible.Add(reference);
                continue;
            }

            SPath path{};
            uint32_t connections = 0;
            float openness = 0.0f;
            if (!TestGraph(owner->m_lSoundGraphId, _room->m_lSoundGraphId, connections,
                    &path, openness, soundDll->m_ListenerPosition))
                continue;
            if (owner->m_lAudioFilter && connections < 8)
            {
                path.m_tConnections[connections] = 0;
                path.m_Filters[connections++] = {static_cast<uint32_t>(owner->m_lAudioFilter), 2, 0.0f};
            }
            path.m_lNumConnections = static_cast<int32_t>(connections);
            int index = FindChain(temporary, count, &path);
            if (index < 0 && count < 32)
            {
                const int old = FindChain(m_Pathes, m_lNumPathes, &path);
                path.m_bNew = old < 0;
                path.m_lRemapIdx = old;
                index = count;
                CopyChain(&temporary[count++], &path);
            }
            if (index >= 0)
            {
                object->AddPath(index);
                _audible.Add(reference);
            }
        }

        m_rtObsoleteChains.Clear();
        for (int i = 0; i < m_lNumPathes; ++i)
            if (FindChain(temporary, count, &m_Pathes[i]) < 0)
                m_rtObsoleteChains.Add(i);
        for (int i = 0; i < count; ++i)
            CopyChain(&m_Pathes[i], &temporary[i]);
        m_lNumPathes = count;
    }

    bool ZSoundGraph::TestRooms(uint32_t _first, uint32_t _second, bool _testOpen)
    {
        if (_first == _second)
            return true;
        if (!m_pRooms || !m_pConnections || _first >= m_lNumRooms || _second >= m_lNumRooms)
            return false;

        struct Pair { uint32_t first; uint32_t second; };
        Pair pending[2]{{_first, _second}, {0, 0}};
        int pendingCount = 1;
        int current = 0;
        bool open = false;

        while (pendingCount)
        {
            uint32_t first = pending[current].first;
            uint32_t second = pending[current].second;
            const uint32_t low = (std::min)(first, second);
            const uint32_t high = (std::max)(first, second);
            if (low >= m_lNumRooms || high >= m_lNumRooms)
                return false;

            const SGRoom& room = m_pRooms[low];
            if (!room.m_lConnectionCount ||
                room.m_lConnectionsIdx + room.m_lConnectionCount > m_lNumConnections)
                return _testOpen ? false : open;
            const uint32_t* connections = m_pConnections + room.m_lConnectionsIdx;
            const auto target = [](uint32_t _connection) { return (_connection >> 16) & 0x3FFF; };
            if (high < target(connections[0]) || high > target(connections[room.m_lConnectionCount - 1]))
                return _testOpen ? false : open;
            if (_testOpen)
                return true;

            for (uint32_t i = 0; i < room.m_lConnectionCount; ++i)
            {
                const uint32_t connection = connections[i];
                if (target(connection) != high)
                    continue;
                const uint32_t payload = connection & 0xFFFF;
                switch (connection >> 30)
                {
                case 0:
                    if (payload < m_lNumExits)
                        open = open || m_pExits[payload].m_fOpenness != 0.0f;
                    ++current;
                    break;
                case 1:
                    if (payload < m_lNumExitPairs)
                    {
                        const SExitPair& pair = m_pExitPairs[payload];
                        if (pair.m_lExitIdx1 < m_lNumExits && pair.m_lExitIdx2 < m_lNumExits)
                        {
                            open = open || (m_pExits[pair.m_lExitIdx1].m_fOpenness != 0.0f &&
                                m_pExits[pair.m_lExitIdx2].m_fOpenness != 0.0f);
                        }
                    }
                    ++current;
                    break;
                case 2:
                    pending[0] = {payload, low};
                    pending[1] = {payload, high};
                    current = 0;
                    pendingCount = 3;
                    break;
                default:
                    ZASSERT(false);
                    ++current;
                    break;
                }
                break;
            }
            --pendingCount;
        }
        return open;
    }

    bool ZSoundGraph::TestRooms(float* _listener, float* _source, uint32_t _first,
        uint32_t _second, uint32_t _flags, float&)
    {
        if (_first == _second)
            return true;
        if (!_listener || !_source || !m_pRooms || !m_pConnections ||
            _first >= m_lNumRooms || _second >= m_lNumRooms)
            return false;

        struct Gate
        {
            ZVector3 vertices[4];
            ZVector3 position;
            ZVector3 normal;
        };
        struct Candidate
        {
            Gate gates[2];
            int count;
        };
        Candidate candidates[32]{};
        int candidateCount = 0;

        const uint32_t low = (std::min)(_first, _second);
        const uint32_t high = (std::max)(_first, _second);
        const SGRoom& room = m_pRooms[low];
        if (!room.m_lConnectionCount || room.m_lConnectionsIdx + room.m_lConnectionCount > m_lNumConnections)
            return false;

        auto addGate = [](Candidate& _candidate, const SExit& _exit, uint32_t _room, bool _reverseBase)
        {
            Gate& gate = _candidate.gates[_candidate.count++];
            gate.position = _exit.m_vPlanePos;
            gate.normal = _exit.m_vPlaneNormal;
            bool reverse = _reverseBase;
            if (_room != _exit.m_lRoomId)
                reverse = !reverse;
            if (reverse)
            {
                for (int i = 0; i < 4; ++i)
                    gate.vertices[i] = _exit.m_Verts[i];
                gate.normal = {-gate.normal.x, -gate.normal.y, -gate.normal.z};
            }
            else
            {
                for (int i = 0; i < 4; ++i)
                    gate.vertices[i] = _exit.m_Verts[3 - i];
            }
        };

        const uint32_t* connections = m_pConnections + room.m_lConnectionsIdx;
        for (uint32_t i = 0; i < room.m_lConnectionCount && candidateCount < 32; ++i)
        {
            const uint32_t connection = connections[i];
            if (((connection >> 16) & 0x3FFF) != high)
                continue;
            const uint32_t payload = connection & 0xFFFF;
            if ((connection >> 30) == 0 && payload < m_lNumExits)
            {
                const SExit& exit = m_pExits[payload];
                if (exit.m_fOpenness != 0.0f || (_flags & 2))
                    addGate(candidates[candidateCount++], exit, high, false);
            }
            else if ((connection >> 30) == 1 && payload < m_lNumExitPairs)
            {
                const SExitPair& pair = m_pExitPairs[payload];
                if (pair.m_lExitIdx1 >= m_lNumExits || pair.m_lExitIdx2 >= m_lNumExits)
                    continue;
                const SExit& firstExit = m_pExits[pair.m_lExitIdx1];
                const SExit& secondExit = m_pExits[pair.m_lExitIdx2];
                if ((firstExit.m_fOpenness != 0.0f && secondExit.m_fOpenness != 0.0f) || (_flags & 2))
                {
                    Candidate& candidate = candidates[candidateCount++];
                    addGate(candidate, firstExit, low, true);
                    addGate(candidate, secondExit, high, false);
                }
            }
            else if ((connection >> 30) == 3)
                ZASSERT(false);
        }

        const ZVector3 listener{_listener[0], _listener[1], _listener[2]};
        const ZVector3 source{_source[0], _source[1], _source[2]};
        const ZVector3 ray = Subtract(source, listener);
        for (int candidateIndex = 0; candidateIndex < candidateCount; ++candidateIndex)
        {
            int failures = 0;
            for (int gateIndex = 0; gateIndex < candidates[candidateIndex].count; ++gateIndex)
            {
                const Gate& gate = candidates[candidateIndex].gates[gateIndex];
                bool passed = Dot(Subtract(source, gate.position), gate.normal) > 0.0f;
                for (int edge = 0; passed && edge < 4; ++edge)
                {
                    const ZVector3 first = Subtract(listener, gate.vertices[edge]);
                    const ZVector3 second = Subtract(listener, gate.vertices[(edge + 1) & 3]);
                    passed = Dot(ray, Normalize(Cross(first, second))) < 0.0f;
                }
                if (!passed)
                    ++failures;
            }
            if ((_flags & 1) ? failures <= m_lMaxReflections : failures == 0)
                return true;
        }
        return false;
    }

    void ZSoundGraph::InstallLoadedGraph(char* _data, int _size)
    {
        m_pLoadedGraphData = _data;
        m_lLoadedGraphSize = _size;
    }

    void ZSoundGraph::SetMaxReflections(uint8_t _count)
    {
        m_lMaxReflections = _count;
    }

    void ZSoundGraph::CopyChain(SPath* _destination, const SPath* _source)
    {
        if (!_destination || !_source)
            return;
        _destination->m_bNew = _source->m_bNew;
        _destination->m_lRemapIdx = _source->m_lRemapIdx;
        _destination->m_lNumConnections = _source->m_lNumConnections;
        for (int i = 0; i < _source->m_lNumConnections; ++i)
        {
            _destination->m_tConnections[i] = _source->m_tConnections[i];
            _destination->m_Filters[i] = _source->m_Filters[i];
        }
    }

    int32_t ZSoundGraph::FindChain(const SPath* _paths, int32_t _count, const SPath* _wanted) const
    {
        if (!_paths || !_wanted)
            return -1;
        for (int32_t i = 0; i < _count; ++i)
        {
            if (_paths[i].m_lNumConnections != _wanted->m_lNumConnections)
                continue;
            int connection = 0;
            while (connection < _wanted->m_lNumConnections &&
                _paths[i].m_tConnections[connection] == _wanted->m_tConnections[connection])
            {
                ++connection;
            }
            if (connection == _wanted->m_lNumConnections)
                return i;
        }
        return -1;
    }

    bool ZSoundGraph::TestGraph(uint32_t _first, uint32_t _second, uint32_t& _connectionCount,
        SPath* _path, float&, const ZVector3& _listenerPosition)
    {
        if (!_path || !m_pRooms || !m_pConnections || _first >= m_lNumRooms || _second >= m_lNumRooms)
            return false;
        const uint32_t low = (std::min)(_first, _second);
        const uint32_t high = (std::max)(_first, _second);
        const SGRoom& room = m_pRooms[low];
        if (!room.m_lConnectionCount || room.m_lConnectionsIdx + room.m_lConnectionCount > m_lNumConnections)
            return false;

        const uint32_t* connections = m_pConnections + room.m_lConnectionsIdx;
        int best = -1;
        int nearest = -1;
        float bestOpenness = 0.0f;
        float nearestDistance = 1.0e38f;
        for (uint32_t i = 0; i < room.m_lConnectionCount; ++i)
        {
            const uint32_t connection = connections[i];
            if (((connection >> 16) & 0x3FFF) != high)
                continue;
            const uint32_t payload = connection & 0xFFFF;
            float openness = 0.0f;
            float distance = 1.0e38f;
            if ((connection >> 30) == 0 && payload < m_lNumExits)
            {
                const SExit& exit = m_pExits[payload];
                openness = exit.m_fOpenness;
                distance = Dot(Subtract(_listenerPosition, exit.m_vPlanePos),
                    Subtract(_listenerPosition, exit.m_vPlanePos));
            }
            else if ((connection >> 30) == 1 && payload < m_lNumExitPairs)
            {
                const SExitPair& pair = m_pExitPairs[payload];
                if (pair.m_lExitIdx1 >= m_lNumExits || pair.m_lExitIdx2 >= m_lNumExits)
                    continue;
                const SExit& first = m_pExits[pair.m_lExitIdx1];
                const SExit& second = m_pExits[pair.m_lExitIdx2];
                openness = first.m_fOpenness * second.m_fOpenness;
                const float firstDistance = Dot(Subtract(_listenerPosition, first.m_vPlanePos),
                    Subtract(_listenerPosition, first.m_vPlanePos));
                const float secondDistance = Dot(Subtract(_listenerPosition, second.m_vPlanePos),
                    Subtract(_listenerPosition, second.m_vPlanePos));
                distance = (std::min)(firstDistance, secondDistance);
            }
            else
                continue;

            if (distance < nearestDistance)
            {
                nearestDistance = distance;
                nearest = static_cast<int>(i);
            }
            if (openness >= bestOpenness)
            {
                bestOpenness = openness;
                best = static_cast<int>(i);
            }
        }

        const int selected = bestOpenness == 0.0f ? nearest : best;
        if (selected < 0)
            return false;
        const uint32_t connection = connections[selected];
        const uint32_t payload = connection & 0xFFFF;
        if ((connection >> 30) == 1)
        {
            const SExitPair& pair = m_pExitPairs[payload];
            if (_connectionCount + 2 > 8)
                return bestOpenness != 0.0f;
            _path->m_tConnections[_connectionCount] = payload;
            _path->m_Filters[_connectionCount++] = {pair.m_lExitIdx1, 1, 0.0f};
            _path->m_tConnections[_connectionCount] = payload;
            _path->m_Filters[_connectionCount++] = {pair.m_lExitIdx2, 1, 0.0f};
        }
        else if ((connection >> 30) == 0)
        {
            if (_connectionCount >= 8)
                return bestOpenness != 0.0f;
            const SExit& exit = m_pExits[payload];
            _path->m_tConnections[_connectionCount] = payload;
            _path->m_Filters[_connectionCount++] = {payload, 1, 0.0f};
            if (bestOpenness != 0.0f && _connectionCount < 8)
            {
                const ZVector3 direction = Normalize(Subtract(_listenerPosition, exit.m_vPlanePos));
                _path->m_tConnections[_connectionCount] = payload;
                _path->m_Filters[_connectionCount++] = {payload, 7,
                    std::fabs(Dot(direction, exit.m_vPlaneNormal))};
            }
        }
        return bestOpenness != 0.0f;
    }
}
