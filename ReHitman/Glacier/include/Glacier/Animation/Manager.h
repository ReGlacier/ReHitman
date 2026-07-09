#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Animation/Fwd.h>
#include <cstdint>

namespace Glacier::Animation
{
    class CrowdHeader
    {
        char m_szName[32];
        int m_iOffset;
        int m_iFrames;
    };
    RE_VERIFY_SIZE(CrowdHeader, 0x28);

    struct ZNameList
    {
        char* m_Names;
        int m_Size;
        int m_Count;

        int GetId(const char* pszAnimName, int iNoneIndex);
        const char* GetName(int id, int none);
    };
    RE_VERIFY_SIZE(ZNameList, 0xC);

    struct BlockCache 
    {
        int16_t m_Block;
        int16_t m_Size;
        uint16_t* m_Ids;
        uint8_t* m_Lps;
        float* m_Data;
    };
    RE_VERIFY_SIZE(BlockCache, 0x10);

    struct StateCacheEntry 
    {
        BlockCache m_Entry;
        int32_t m_Used;
        struct StateCacheEntry* m_Next;
        float m_Data[936];
    };
    RE_VERIFY_SIZE(StateCacheEntry, 0xEB8);

    struct StateCache 
    {
        StateCacheEntry** m_CacheEntry;
        StateCacheEntry*  m_Data;
        int32_t           m_CacheLines;
        StateCacheEntry** m_AllocStack;
        int32_t           m_AllocPos;
        int32_t           m_AnimCount;
        int32_t           m_Hits;
        int32_t           m_Misses;
    };
    RE_VERIFY_SIZE(StateCache, 0x20);
    
    class Manager
    {
    public:
        // Types
        enum EResult : int32_t { eUndefinedError = 0x1 };

        // Data
        Header* m_Headers;
        int m_Animcount;
        CrowdHeader* m_pCrowdHeaders;
        int32_t m_iCrowdAnimCount;
        uint8_t* m_Data;
        int32_t m_Pos;
        int32_t m_MaxPos;
        int m_OwnsBuffers;
        int m_StateSize;
        int m_HumanQuatSize;
        int m_QuatSize;
        int m_PoseSize;
        bool m_PlayUncompressed;
        RE_ADD_PADDING(3);
        ZNameList m_BoneNames;
        ZNameList m_AnimNames;
        ZNameList m_PoseNames;
        StateCache* m_Cache;
        int m_SizeUncompressed;
        char* m_pMetaKeyData;
        char* m_pMetaKeyDataStrings;
    };
    RE_VERIFY_SIZE(Manager, 0x68); // Verified by ZEngineDataBase::AllocSequence method
}