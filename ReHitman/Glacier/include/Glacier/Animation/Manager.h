#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/Animation/Fwd.h>
#include <Glacier/Animation/ZPose.h>
#include <Glacier/Animation/ZBone.h>
#include <Glacier/Animation/StreamPacker.h>
#include <Glacier/Animation/ZNameList.h>
#include <cstdint>


namespace Glacier::Animation
{
    struct ZMetaKey
    {
        uint32_t lFrame;
        uint32_t lValue;
        uint32_t length;
        uint32_t lStringOffset;

        const char* GetString();
    };
    RE_VERIFY_SIZE(ZMetaKey, 0x10);

    class CrowdHeader
    {
        char m_szName[32];
        int m_iOffset;
        int m_iFrames;
    };
    RE_VERIFY_SIZE(CrowdHeader, 0x28);

    struct StateCacheEntry
    {
        StreamPacker::BlockCache m_Entry;
        int32_t m_Used;
        struct StateCacheEntry* m_Next;
        float m_Data[936];
    };
    RE_VERIFY_SIZE(StateCacheEntry, 0xEB8);

    struct StateCache
    {
        // methods
        StateCache(int lAnimCount, int lCacheLines);
        ~StateCache();
        StateCacheEntry* FindEntry(int lCacheEntry, int16_t lBlock);
        StateCacheEntry* AllocEntry(int lNextCacheEntry, int16_t lBlock);
        void Update();

        // members
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
        // types
        enum EResult : int32_t { eUndefinedError = 0x1 };

        // methods
        Manager();
        ~Manager();

        /**
         * Allocates and initializes an animation manager from an ANM data block.
         *
         * This is the common operation performed by the engine after it has
         * resolved the ANM chunk from the packed animation file. The returned
         * manager owns its object allocation, while the loaded animation data
         * remains borrowed from pBlock and must outlive the manager's use of
         * that data. Destroy the returned manager with ZUniMemory::Delete.
         *
         * @param pBlock Pointer to the ANM payload, excluding its CHUNKFILE
         *               header.
         * @param lSize Size of the ANM payload in bytes.
         * @return A newly allocated and loaded manager, or nullptr if object
         *         allocation fails.
         */
        static Manager* CreateFromDataBlock(void* pBlock, int lSize);

        /**
         * Loads the animation manager's serialized data block.
         *
         * The block starts with the animation counts and byte sizes used by
         * the manager, followed by an array of animation headers and the
         * associated data, bone-name, animation-name, and pose-name buffers.
         * The manager points into this caller-owned memory; it does not copy
         * the block and marks the loaded buffers as non-owned.
         *
         * If packed animation data is available through the engine data base,
         * this method also resolves chunks 8 and 9 as the metadata key data
         * and metadata key string data, respectively. Missing packed animation
         * data leaves both metadata pointers null. The state cache is reset to
         * null after loading, matching the original PC implementation.
         *
         * @param pBlock Pointer to the serialized animation data block.
         * @param lSize Size of the serialized block in bytes. The original
         *              implementation accepts this value but does not use it
         *              for bounds validation.
         * @return Zero on completion.
         */
        int LoadDataBlock(void* pBlock, int lSize);
        char* GetAnimName(int id);
        BoneID GetBoneID(const char* pszAnimName);
        void* GetArray(void*& pBuffer, int lOffset);
        void GetInt32(void*& pBuffer, int32_t& result);
        Header* FromIndex(int lIndex);
        int ToIndex(const Header* pHeader);
        PoseID GetPoseID(const char* pszPoseName);
        const char* GetMetaKeyDataStrings();
        bool GetPlayUncompressed() const;
        int Clear();
        uint32_t GetMetaKeyDataLength(int lMetaKey);
        ZMetaKey* GetMetaKeyData(int lMetaKey);
        ZMetaKey* GetMetaKeyAtFrame(int lMetaKey, uint32_t lFrame);
        int32_t GetFrameFromMetaValue(int, uint32_t, int32_t*);
        int32_t GetFrameFromMetaString(int, const char*, uint32_t);
        void SetCompressionRatio(float ratio);
        void SetCompressionRatioPC();


        // members
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

    STATIC_GLOBAL_CLASS_INSTANCE(Manager*, instance);
    extern bool printDebugInfo;
}
