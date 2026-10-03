#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    class ZAssetFragTable
    {
    public:
        // vtbl
        virtual ~ZAssetFragTable();
        // members
        uint64_t m_iStart = 0;           // +0x08
        uint64_t m_iEnd = 0;             // +0x10
        uint64_t m_iSize = 0xDEADBEEFLL; // +0x18
    };
    RE_VERIFY_SIZE(ZAssetFragTable, 0x20); // Verified PC alloc

    class ZAssetFS
    {
    public:
        // vtbl
        virtual ~ZAssetFS();

        // members
        uint32_t m_iAssetSize;
        uint8_t m_iAssetID;
        uint8_t m_iFragCount;
        RE_ADD_PADDING(2);
        uint32_t m_pad[3];
        ZAssetFragTable* FragTable[255];
    };

    class ZAssetManager
    {
    public:
        // vtbl
        virtual ~ZAssetManager();

        // methods
        ZAssetManager();
        bool SetupManager(char* pBuffer);
        bool AssetReadFSTable();
        bool AssetWriteFSTable();
        uint8_t* AssetRead(unsigned char, uint8_t*);
        bool AssetWrite(unsigned char, long unsigned int, long unsigned int, uint8_t*);
        bool AssetExists(unsigned char);
        int GetChunkName(long unsigned int, char*);
        bool AssetAllocate(unsigned char, long unsigned int, long unsigned int);

        // members
        ZAssetFS* FileSystem[16];
        ZAssetFragTable* FreeTable[4080];
        char* m_sRootPath;
        uint16_t m_iChunkSize;
        uint8_t m_iAssetCount;
    };

    class ZAsset
    {
    public:
        // vtbl
        virtual ~ZAsset();

        // methods
        ZAsset();
        bool SetupAsset(unsigned char, long unsigned int, void*);
        bool SetupManager(ZAssetManager* pManager);
        bool AllocateAsset(uint64_t lSize);
        uint8_t* GetAsset() const;
        bool SetAsset();

        // members
        uint8_t m_iAssetID;
        uint64_t m_iAssetSize;
        uint64_t m_iAssetAllocation;
        uint8_t* m_pAssetData;
        ZAssetManager* m_pAssetManager;
    };
}
