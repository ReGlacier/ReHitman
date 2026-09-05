#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZSTL/REFTAB.h>

namespace Glacier
{
    struct ZPoolAllocator;

    struct ZPoolAllocator
    {
        // const
        static constexpr size_t MAX_ALLOCATORS_NR = 0x20;

        // types
        struct ZAllocHeader 
        {
            uint32_t size;
        };
        RE_VERIFY_SIZE(ZAllocHeader, 0x4); // Verified

        struct ZBlockHeader
        {
            unsigned int size;
            char *pointer;
        };
        RE_VERIFY_SIZE(ZBlockHeader, 0x8); // Verified

        // constructor
        ZPoolAllocator(char *pBuffer, unsigned int iSize, const char* pName, bool bDebug);
        ~ZPoolAllocator();

        // methods
        static void ResetAll();
        static void CheckAllPools();
        static int BlockHeaderSort(const ZBlockHeader* a, const ZBlockHeader* b);
        static void ReportHighWaterMarks();

        // static members
        STATIC_CLASS_VAR(ZPoolAllocator, uint8_t, s_iNumPoolAllocators);
        STATIC_CLASS_VAR_ARRAY(ZPoolAllocator, ZPoolAllocator*, s_PoolAllocators, MAX_ALLOCATORS_NR);

        // members
        void* Alloc(unsigned int iSize);
        void Free(void *p);
        void CheckPool();
        void AddBlock(char* pBlock, unsigned int iSize);
        void RemoveBlock(ZBlockHeader* pBlock);
        void ReportHighWaterMark();
        void Reset(char *pBuffer, unsigned int iSize, bool bDebug);

        // data | DronCode: Weird, but in PS2 this structure is slightly different. 
        //                  And Mini Ninjas contains a valid code.
        unsigned int m_iAllocated{0};
        unsigned int m_iHighWaterMark{0};
        const char *m_pName{nullptr};
        char *m_pBuffer{nullptr};
        unsigned int m_iSize{0};
        ZPoolAllocator::ZBlockHeader *m_pBlockDataBegin{nullptr};
        bool m_bNearMaxWarningPrinted{false};

        // private impl
    private:
        void ExpandBlockRight(ZBlockHeader* pBlockHeader, unsigned int iSize);
        void ExpandBlockLeft(ZBlockHeader* pBlockHeader, unsigned int iSize);
        void SortBlockHeaders();
        ZBlockHeader* GetPreviousHeader(ZBlockHeader* pBlockHeader);
        ZBlockHeader* GetNextHeader(ZBlockHeader* pBlockHeader);
    };
    RE_VERIFY_SIZE(ZPoolAllocator, 0x1C); // Verified

    class ZPoolAllocRefTab : public REFTAB
    {
    public:
        // methods
        ZPoolAllocRefTab(ZPoolAllocator* pAllocator, int pPoolSize, int pUserData);
        ZPoolAllocRefTab(ZPoolAllocator* pAllocator);

        // vtbl
        ~ZPoolAllocRefTab() override;
        void DeleteBlock(TabBlk *) override;
        TabBlk* NewBlock(void) override;

        // members
        ZPoolAllocator* m_pAllocator;
    };
    RE_VERIFY_SIZE(ZPoolAllocRefTab, 0x20);
}