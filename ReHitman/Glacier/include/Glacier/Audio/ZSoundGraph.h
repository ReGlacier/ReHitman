#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/REFTAB32.h>
#include <Glacier/ZSTL/ZMath.h>

namespace Glacier
{
    class ZROOM;

    struct SFilter
    {
        uint32_t m_lIdx;
        uint32_t m_lType;
        float m_fAmount;
    };

    struct SPath
    {
        bool m_bNew;
        RE_ADD_PADDING(3);
        int32_t m_lRemapIdx;
        uint32_t m_tConnections[8];
        int32_t m_lNumConnections;
        SFilter m_Filters[8];
    };

    struct SExit
    {
        float m_fOpenness;
        float m_fAmount;
        int32_t m_lFrequency;
        uint32_t m_lRoomId;
        uint32_t m_lRoomId2;
        ZVector3 m_vPlanePos;
        ZVector3 m_vPlaneNormal;
        ZVector3 m_Verts[4];
    };

    struct SExitPair
    {
        uint32_t m_lExitIdx1;
        uint32_t m_lExitIdx2;
        float m_fDistance;
    };

    struct SGRoom
    {
        uint32_t m_lExitLookupIdx;
        uint32_t m_lConnectionCount;
        uint32_t m_lConnectionsIdx;
    };

    class ZSoundGraph
    {
    public:
        ZSoundGraph();
        virtual ~ZSoundGraph();
        virtual void Initialize();
        virtual void CleanUp();
        virtual void GetAudiable(ZROOM* _room, REFTAB32& _sources, REFTAB32& _audible);
        virtual bool TestRooms(float* _position, float* _direction, uint32_t _first, uint32_t _second, uint32_t _flags, float& _distance);
        virtual bool TestRooms(uint32_t _first, uint32_t _second, bool _testOpen);

        void InstallLoadedGraph(char* _data, int _size);
        void SetMaxReflections(uint8_t _count);
        void CopyChain(SPath* _destination, const SPath* _source);
        int32_t FindChain(const SPath* _paths, int32_t _count, const SPath* _wanted) const;

    private:
        bool TestGraph(uint32_t _first, uint32_t _second, uint32_t& _connectionCount,
            SPath* _path, float& _openness, const ZVector3& _listenerPosition);

    public:
        char* m_pLoadedGraphData;
        int32_t m_lLoadedGraphSize;
        float m_fMorphFactor;
        uint32_t m_lCurrentRoomIdx;
        uint32_t m_lMorphRoomIdx;
        bool m_bEnvMorph;
        uint8_t m_lMaxReflections;
        RE_ADD_PADDING(2);
        REFTAB32 m_rtObsoleteChains;
        int32_t m_lNumPathes;
        SPath m_Pathes[32];
        uint32_t m_lNumExits;
        uint32_t m_lNumRooms;
        uint32_t m_lNumExitPairs;
        uint32_t m_lNumConnections;
        SExit* m_pExits;
        SExitPair* m_pExitPairs;
        SGRoom* m_pRooms;
        uint32_t* m_pConnections;
        uint32_t* m_pExitLookup;
        int32_t m_lNumLookupEntries;
        bool m_bActivateAll;
        RE_ADD_PADDING(3);
    };

    RE_VERIFY_SIZE(SFilter, 0x0C);
    RE_VERIFY_SIZE(SPath, 0x8C);
    RE_VERIFY_SIZE(SExit, 0x5C);
    RE_VERIFY_SIZE(SExitPair, 0x0C);
    RE_VERIFY_SIZE(SGRoom, 0x0C);
    RE_VERIFY_OFFSET(ZSoundGraph, m_lNumPathes, 0xC8);
    RE_VERIFY_OFFSET(ZSoundGraph, m_Pathes, 0xCC);
    RE_VERIFY_OFFSET(ZSoundGraph, m_lNumExits, 0x124C);
    RE_VERIFY_SIZE(ZSoundGraph, 0x1278); // Verified PC alloc
}
