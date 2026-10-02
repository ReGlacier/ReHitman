#include <Glacier/System/ZSysMem.h>
#include <Glacier/Debug/ZMemReadOut.h>
#include <Glacier/ZUniAssert.h>

#include <Windows.h>
#include <cstdlib>
#include <cstring>
#include <mutex>


namespace Glacier
{
    namespace
    {
        static uint32_t g_iMemColor = 0;

        uint32_t ZDebugSetMemColor(uint32_t lColor)
        {
            auto lOldMemColor = g_iMemColor;
            g_iMemColor = lColor;
            return lOldMemColor;
        }

        // Process-wide lock guarding the allocator list and the debug handlers.
        // Lazily initialized so it is valid even when no ZSysMem is constructed by
        // us (the game build reuses the original game's instance, but still routes
        // through these helper methods). Initialized exactly once, thread-safely.
        CRITICAL_SECTION& SysMemCS()
        {
            static CRITICAL_SECTION cs;                 // zero-initialized storage
            static std::once_flag once;
            std::call_once(once, [] { ::InitializeCriticalSection(&cs); });
            return cs;
        }

        // The single default allocator instance shared by every EAllocType.
        // PC keeps it as a lazily constructed global freed via atexit().
        alignas(ZWin32Allocator) uint8_t g_defaultAllocatorStorage[sizeof(ZWin32Allocator)];
        ZWin32Allocator* const g_pDefaultAllocator = reinterpret_cast<ZWin32Allocator*>(g_defaultAllocatorStorage);
        bool g_bDefaultAllocatorInited = false;

        void DefaultAllocatorDtor()
        {
            g_pDefaultAllocator->~ZWin32Allocator();
        }

        constexpr uint32_t LARGE_BLOCK_MAGIC = 0xC001BABEu;  // large heap block header
        constexpr uint32_t POOL_BLOCK_FREE = 0xC001BABEu;    // pool block released marker
        constexpr uint32_t POOL_BLOCK_ALLOC = 0xCBCBCBCBu;   // pool block allocated poison
    }

#ifdef REHITMAN_TESTS
    // Test build owns its own singleton: tests construct a real ZSysMem (whose
    // ZComponentSingleton base publishes m_pInstance) and verify allocator behavior.
    template<>
    ISysMem* ZComponentSingleton<ISysMem, ZGlobalComponentBase>::m_pInstance = nullptr;
#else
    // Game build reuses the instance owned by the original game (GoG version).
    template<>
    ISysMem* ZComponentSingleton<ISysMem, ZGlobalComponentBase>::m_pInstance = reinterpret_cast<ISysMem*>(0x008208C8);
#endif

    // The singleton instance is always a concrete ZSysMem (single inheritance,
    // offset 0), so ISysMem's helpers downcast to reach the real implementation.
    void* ISysMem::New(EAllocType eMemType, int iSize)
    {
        return static_cast<ZSysMem*>(this)->New(eMemType, static_cast<unsigned int>(iSize));
    }

    void ISysMem::Delete(void* pMem)
    {
        static_cast<ZSysMem*>(this)->Delete(static_cast<char*>(pMem));
    }

    // ========================================================================
    // ZSysMem
    // ========================================================================

    void ZSysMem::InitCriticalSection()
    {
        // Ensures the process-wide lock exists; the real init is deferred to the
        // first SysMemCS() access, so repeated constructions are harmless.
        (void)SysMemCS();
    }

    void ZSysMem::EnterCriticalSection()
    {
        ::EnterCriticalSection(&SysMemCS());
    }

    void ZSysMem::LeaveCriticalSection()
    {
        ::LeaveCriticalSection(&SysMemCS());
    }

    ZSysMem::ZSysMem()
    {
        // The ZComponentSingleton<ISysMem,...> base constructor publishes m_pInstance.
        memset(m_pAllocatorList, 0, sizeof(m_pAllocatorList));
        memset(m_pDebugHandlers, 0, sizeof(m_pDebugHandlers));
        m_iNrDebugHandlers = 0;
        m_File = nullptr;
        m_Line = 0;

        InitCriticalSection();
        CreateAllocators();
    }

    ZSysMem::~ZSysMem()
    {
        EnterCriticalSection();
        DestroyAllocators();
        LeaveCriticalSection();
        // ~ZComponentSingleton clears m_pInstance after this body.
    }

    void ZSysMem::CreateAllocators()
    {
        if (!g_bDefaultAllocatorInited)
        {
            g_bDefaultAllocatorInited = true;
            znew_placement(g_pDefaultAllocator);
            atexit(DefaultAllocatorDtor);
        }

        ZWin32Allocator* pDefault = g_pDefaultAllocator;
        SetAllocator(DEFAULT_MEM, pDefault);
        SetAllocator(SLOW_MEM, pDefault);
        SetAllocator(FAST_MEM, pDefault);
        SetAllocator(STATIC_MEM, pDefault);
        SetAllocator(RENDERCPU_MEM, pDefault);
        SetAllocator(RENDERPRIMACCESS_MEM, pDefault);
    }

    void ZSysMem::DestroyAllocators()
    {
        Reset();

        for (int i = 0; i < EAllocType::END_OF_ALLOCATOR_TYPES; ++i)
            m_pAllocatorList[i].m_pAllocator = nullptr;
    }

    void ZSysMem::Reset()
    {
        EnterCriticalSection();

        for (int i = 0; i < EAllocType::END_OF_ALLOCATOR_TYPES; ++i)
        {
            ZAllocatorBase* pAlloc = m_pAllocatorList[i].m_pAllocator;
            if (pAlloc)
                pAlloc->Reset();
        }

        for (unsigned int i = 0; i < m_iNrDebugHandlers; ++i)
            m_pDebugHandlers[i]->Reset();

        LeaveCriticalSection();
    }

    void ZSysMem::SetAllocator(EAllocType eMemType, ZAllocatorBase* pAlloc)
    {
        EnterCriticalSection();
        m_pAllocatorList[eMemType].m_pAllocator = pAlloc;
        LeaveCriticalSection();
    }

    ZAllocatorBase* ZSysMem::GetAllocator(EAllocType eMemType)
    {
        return m_pAllocatorList[eMemType].m_pAllocator;
    }

    void ZSysMem::AddDebugHandler(ZSysmemDebugHandler* pHandler)
    {
        EnterCriticalSection();
        // PC breaks when the handler table is already full.
        ZASSERT(m_iNrDebugHandlers < MAX_NR_DEBUG_HANDLERS);
        if (m_iNrDebugHandlers < MAX_NR_DEBUG_HANDLERS)
        {
            m_pDebugHandlers[m_iNrDebugHandlers] = pHandler;
            ++m_iNrDebugHandlers;
        }
        LeaveCriticalSection();
    }

    void ZSysMem::RemoveDebugHandler(ZSysmemDebugHandler* pHandler)
    {
        EnterCriticalSection();
        ZASSERT(m_iNrDebugHandlers != 0);

        unsigned int idx = 0;
        while (idx < m_iNrDebugHandlers && m_pDebugHandlers[idx] != pHandler)
            ++idx;

        // PC breaks when the handler is not present.
        ZASSERT(idx != m_iNrDebugHandlers);
        if (idx != m_iNrDebugHandlers)
        {
            // Remove by swapping the last handler into the freed slot.
            m_iNrDebugHandlers--;
            m_pDebugHandlers[idx] = m_pDebugHandlers[m_iNrDebugHandlers];
        }

        LeaveCriticalSection();
    }

    void ZSysMem::SetFileLine(const char* psFile, int iLine)
    {
        m_File = psFile;
        m_Line = iLine;
    }

    void ZSysMem::GetFileLine(const char** ppsFile, int* piLine)
    {
        *ppsFile = m_File;
        *piLine = m_Line;
    }

    char* ZSysMem::New(EAllocType eMemType, unsigned int iSize)
    {
        EnterCriticalSection();

        // Fall back to the default allocator when no allocator is bound to the type.
        if (!m_pAllocatorList[eMemType].m_pAllocator)
            eMemType = DEFAULT_MEM;

        // PreAllocChange pass: handlers may rewrite the requested size (forward order).
        for (unsigned int i = 0; i < m_iNrDebugHandlers; ++i)
            m_pDebugHandlers[i]->PreAllocChange(iSize);

        ZAllocatorBase* pAlloc = m_pAllocatorList[eMemType].m_pAllocator;

        char* pRealStart = nullptr;
        char* pRealEnd = nullptr;
        char* pMem = pAlloc->Alloc(iSize, &pRealStart, &pRealEnd);

        if (pMem || !pAlloc->ReturnNullOnAllocFail())
        {
            // PostAllocChange then PostAlloc, both in reverse handler order (PC).
            for (int i = static_cast<int>(m_iNrDebugHandlers) - 1; i >= 0; --i)
                m_pDebugHandlers[i]->PostAllocChange(pMem, iSize);

            for (int i = static_cast<int>(m_iNrDebugHandlers) - 1; i >= 0; --i)
                m_pDebugHandlers[i]->PostAlloc(pMem, iSize, pRealStart, pRealEnd);

            LeaveCriticalSection();
            return pMem;
        }

        LeaveCriticalSection();
        return nullptr;
    }

    void ZSysMem::Delete(char* pMem)
    {
        if (!pMem)
            return;

        EnterCriticalSection();

        // PreFree pass (forward order).
        for (unsigned int i = 0; i < m_iNrDebugHandlers; ++i)
            m_pDebugHandlers[i]->PreFree(pMem);

        // PreFreeChange pass: handlers may rewrite the pointer (forward order).
        for (unsigned int i = 0; i < m_iNrDebugHandlers; ++i)
            m_pDebugHandlers[i]->PreFreeChange(pMem);

        // Hand the block to whichever allocator recognizes it.
        for (int i = 0; i < EAllocType::END_OF_ALLOCATOR_TYPES; ++i)
        {
            ZAllocatorBase* pAlloc = m_pAllocatorList[i].m_pAllocator;
            if (pAlloc && pAlloc->Free(pMem))
                break;
        }

        LeaveCriticalSection();
    }

    // ========================================================================
    // ZWin32Allocator
    // ========================================================================

    ZWin32Allocator::ZWin32Allocator()
    {
        // Reserve up to 256 MiB (read/write), halving the request until it lands.
        SIZE_T reserve = 0x10000000;
        m_pVirtualAllocBase = VirtualAlloc(nullptr, reserve, MEM_RESERVE, PAGE_READWRITE);
        while (!m_pVirtualAllocBase)
        {
            if (reserve < 0x8000000)
                break;
            reserve >>= 1;
            m_pVirtualAllocBase = VirtualAlloc(nullptr, reserve, MEM_RESERVE, PAGE_READWRITE);
        }
        // The reserved base must be non-null and 64 KiB aligned.
        ZASSERT(m_pVirtualAllocBase != nullptr);
        ZASSERT((reinterpret_cast<uintptr_t>(m_pVirtualAllocBase) & 0xFFFF) == 0);

        m_iNrAllocatedChunks = 0;

        IMemoryBlockProvider* pProvider = static_cast<IMemoryBlockProvider*>(this);
        for (int i = 0; i < 32; ++i)
        {
            FineGrainedPool& pool = pools[i];
            pool.m_bUsed = 1;
            pool.m_pBlockProvider = pProvider;
            pool.m_iBlockSize = static_cast<uint32_t>(8 * (i + 1));
            pool.m_iBlockStride = static_cast<uint32_t>(8 * (i + 1));
            pool.m_pCurrentPage = nullptr;
            pool.m_pPageHead = nullptr;
            pool.m_pPageTail = nullptr;
        }

        m_pWin32Heap = HeapCreate(HEAP_NO_SERIALIZE, 0x1400000, 0);
        ZASSERT(m_pWin32Heap != nullptr);
    }

    ZWin32Allocator::~ZWin32Allocator()
    {
        // PC releases each pool's pages but leaves the reserved region and the
        // Win32 heap alive (the default allocator lives for the whole process).
        for (int i = 0; i < 32; ++i)
            PoolReset(&pools[i]);
    }

    ISysMem::AllocDirection ZWin32Allocator::SetAllocDirection(ISysMem::AllocDirection)
    {
        return ISysMem::AD_FORWARD;
    }

    char* ZWin32Allocator::Alloc(unsigned int uiSize, char** ppRealStart, char** ppRealEnd)
    {
        uint32_t aligned = uiSize ? ((uiSize + 7u) & ~7u) : 8u;

        if (aligned <= 0x100)
        {
            FineGrainedPool* pPool = &pools[(aligned >> 3) - 1];
            char* pBlock = static_cast<char*>(PoolAlloc(pPool));
            *ppRealStart = pBlock;
            *ppRealEnd = pBlock + aligned;
            return pBlock;
        }

        // Large object: a dedicated Win32 heap block with an 8-byte header.
        char* pRaw = static_cast<char*>(HeapAlloc(static_cast<HANDLE>(m_pWin32Heap), 0, aligned + 8));
        ZASSERT(pRaw != nullptr);
        *reinterpret_cast<uint32_t*>(pRaw + 4) = aligned;
        *reinterpret_cast<uint32_t*>(pRaw) = LARGE_BLOCK_MAGIC;
        *ppRealStart = pRaw;
        *ppRealEnd = pRaw + aligned + 8;
        return pRaw + 8;
    }

    bool ZWin32Allocator::Free(char* pMem)
    {
        uintptr_t base = reinterpret_cast<uintptr_t>(m_pVirtualAllocBase);
        uintptr_t addr = reinterpret_cast<uintptr_t>(pMem);

        if (addr < base || addr >= base + (static_cast<uintptr_t>(m_iNrAllocatedChunks) << 20))
        {
            // Outside the reserved region -> large heap block.
            if (*reinterpret_cast<uint32_t*>(pMem - 8) == LARGE_BLOCK_MAGIC)
            {
                HeapFree(static_cast<HANDLE>(m_pWin32Heap), 0, pMem - 8);
                return true;
            }
            return false;
        }

        // Inside the reserved region -> fine-grained pool block.
        // Pages are 64 KiB aligned, so the page header is the block address with
        // its low 16 bits cleared.
        ZWin32Page* pPage = reinterpret_cast<ZWin32Page*>(addr & 0xFFFF0000);
        ZASSERT(pPage->m_iBlockSize != 0);
        ZASSERT(pPage->m_iBlockSize <= 0x100);

        FineGrainedPool* pPool = &pools[(pPage->m_iBlockSize >> 3) - 1];
        PoolFree(pPool, pPage, pMem);
        return true;
    }

    bool ZWin32Allocator::Shrink(char*, unsigned int)
    {
        return false;
    }

    void ZWin32Allocator::Reset()
    {
        // Intentionally empty on PC.
    }

    unsigned int ZWin32Allocator::GetFreeTotal()
    {
        return 0xF000000u;
    }

    unsigned int ZWin32Allocator::GetLargestBlock()
    {
        return 0xF000000u;
    }

    bool ZWin32Allocator::ReturnNullOnAllocFail()
    {
        return false;
    }

    // ---- Fine-grained pool helpers ---------------------------------------

    ZWin32Page* ZWin32Allocator::PoolNewPage(FineGrainedPool* pPool)
    {
        size_t outBlockSize = 0;
        void* pMem = pPool->m_pBlockProvider->AllocateBlock(
            pPool->m_iBlockSize, 0x10000, 0x10000, 0x10000, &outBlockSize);
        if (!pMem)
            return nullptr;
        ZASSERT(outBlockSize == 0x10000);

        ZWin32Page* pPage = reinterpret_cast<ZWin32Page*>(pMem);
        pPage->m_iBlockSize = pPool->m_iBlockSize;
        pPage->m_iUsed = 0;
        pPage->m_iNextBlockIdx = 0;
        pPage->m_iCapacity = static_cast<uint32_t>((outBlockSize - sizeof(ZWin32Page)) / pPool->m_iBlockStride);
        pPage->m_pFreeListHead = nullptr;
        pPage->m_pNext = nullptr;
        pPage->m_pPrev = pPool->m_pPageTail;

        if (pPool->m_pPageTail)
            pPool->m_pPageTail->m_pNext = pPage;
        else
            pPool->m_pPageHead = pPage;
        pPool->m_pPageTail = pPage;

        return pPage;
    }

    void* ZWin32Allocator::PoolAlloc(FineGrainedPool* pPool)
    {
        ZASSERT(pPool->m_bUsed);

        if (!pPool->m_pCurrentPage)
            pPool->m_pCurrentPage = PoolNewPage(pPool);

        ZWin32Page* pPage = pPool->m_pCurrentPage;

        // When the current page is full, find another page with room or make one.
        while (pPage->m_iUsed == pPage->m_iCapacity)
        {
            ZWin32Page* pFound = nullptr;
            for (ZWin32Page* p = pPool->m_pPageHead; p; p = p->m_pNext)
            {
                if (p->m_iUsed < p->m_iCapacity)
                {
                    pFound = p;
                    break;
                }
            }
            if (!pFound)
                pFound = PoolNewPage(pPool);

            pPage = pFound;
            pPool->m_pCurrentPage = pFound;
            if (pPage->m_pFreeListHead)
                break;
        }

        char* pBlock;
        if (pPage->m_pFreeListHead)
        {
            pBlock = static_cast<char*>(pPage->m_pFreeListHead);
            pPage->m_pFreeListHead = *reinterpret_cast<void**>(pBlock);
            ZASSERT(*reinterpret_cast<uint32_t*>(pBlock + 4) == POOL_BLOCK_FREE);
        }
        else
        {
            ZASSERT(pPage->m_iNextBlockIdx < pPage->m_iCapacity);
            pBlock = reinterpret_cast<char*>(pPage) + sizeof(ZWin32Page)
                   + pPage->m_iNextBlockIdx * pPool->m_iBlockStride;
            ++pPage->m_iNextBlockIdx;
        }

        ++pPage->m_iUsed;
        *reinterpret_cast<uint32_t*>(pBlock + 4) = POOL_BLOCK_ALLOC;
        return pBlock;
    }

    void ZWin32Allocator::PoolReleasePage(FineGrainedPool* pPool, ZWin32Page* pPage)
    {
        if (pPage->m_pPrev)
            pPage->m_pPrev->m_pNext = pPage->m_pNext;
        else
            pPool->m_pPageHead = pPage->m_pNext;

        if (pPage->m_pNext)
            pPage->m_pNext->m_pPrev = pPage->m_pPrev;
        else
            pPool->m_pPageTail = pPage->m_pPrev;

        if (pPool->m_pCurrentPage == pPage)
            pPool->m_pCurrentPage = nullptr;

        pPool->m_pBlockProvider->FreeBlock(pPool->m_iBlockSize, pPage);
    }

    void ZWin32Allocator::PoolFree(FineGrainedPool* pPool, ZWin32Page* pPage, char* pBlock)
    {
        ZASSERT(pBlock);
        ZASSERT(pPage->m_iBlockSize != 0);
        ZASSERT(*reinterpret_cast<uint32_t*>(pBlock + 4) != POOL_BLOCK_FREE);

        // Push the block back onto its page's free list.
        *reinterpret_cast<void**>(pBlock) = pPage->m_pFreeListHead;
        pPage->m_pFreeListHead = pBlock;
        *reinterpret_cast<uint32_t*>(pBlock + 4) = POOL_BLOCK_FREE;

        if (--pPage->m_iUsed == 0)
            PoolReleasePage(pPool, pPage);
    }

    void ZWin32Allocator::PoolReset(FineGrainedPool* pPool)
    {
        if (!pPool->m_bUsed)
            return;

        ZWin32Page* pPage = pPool->m_pPageHead;
        while (pPage)
        {
            ZWin32Page* pNext = pPage->m_pNext;
            pPool->m_pBlockProvider->FreeBlock(pPool->m_iBlockSize, pPage);
            pPage = pNext;
        }

        pPool->m_pCurrentPage = nullptr;
        pPool->m_pPageHead = nullptr;
        pPool->m_pPageTail = nullptr;
        pPool->m_bUsed = 0;
    }

    // ========================================================================
    // VZWin32Allocator (IMemoryBlockProvider)
    // ========================================================================

    void VZWin32Allocator::Unknown()
    {
        // PC implements this slot as an unimplemented hook that breaks.
        ZASSERT(false);
    }

    void* VZWin32Allocator::GetBlockFromChunks()
    {
        int c = 0;
        while (c < static_cast<int>(m_iNrAllocatedChunks) && chunk_headers[c].m_iUsedPages >= 0x10)
            ++c;
        if (c >= static_cast<int>(m_iNrAllocatedChunks))
            return nullptr;

        ChunkHeader& chunk = chunk_headers[c];
        void* pBlock;
        if (chunk.m_pFreeListHead)
        {
            pBlock = chunk.m_pFreeListHead;
            chunk.m_pFreeListHead = *reinterpret_cast<void**>(pBlock);
            ++chunk.m_iUsedPages;
        }
        else
        {
            uint16_t idx = chunk.m_iNextPageIdx;
            pBlock = static_cast<char*>(m_pVirtualAllocBase) + ((static_cast<uint32_t>(idx) + 16u * c) << 16);
            chunk.m_iNextPageIdx = idx + 1;
            ++chunk.m_iUsedPages;
        }
        return pBlock;
    }

    void* VZWin32Allocator::CommitNewChunk()
    {
        ZASSERT(m_iNrAllocatedChunks < 0x100);

        void* pChunk = VirtualAlloc(
            static_cast<char*>(m_pVirtualAllocBase) + (static_cast<uintptr_t>(m_iNrAllocatedChunks) << 20),
            0x100000, MEM_COMMIT, PAGE_READWRITE);
        ZASSERT(pChunk != nullptr);

        ChunkHeader& chunk = chunk_headers[m_iNrAllocatedChunks];
        chunk.m_pFreeListHead = nullptr;
        chunk.m_iUsedPages = 1;
        chunk.m_iNextPageIdx = 1;
        ++m_iNrAllocatedChunks;

        return pChunk;
    }

    void* VZWin32Allocator::AllocateBlock(size_t size, size_t alignment, size_t granularity, size_t flags, size_t* outBlockSize)
    {
        (void)size;
        ZASSERT(alignment == 0x10000);
        ZASSERT(granularity == 0x10000);
        ZASSERT(flags == 0x10000);

        *outBlockSize = 0x10000;

        void* pBlock = GetBlockFromChunks();
        if (!pBlock)
        {
            pBlock = CommitNewChunk();
            ZASSERT(pBlock != nullptr);
        }
        return pBlock;
    }

    void VZWin32Allocator::FreeBlock(int unused, void* ptr)
    {
        (void)unused;
        uintptr_t base = reinterpret_cast<uintptr_t>(m_pVirtualAllocBase);
        uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
        ZASSERT(addr >= base && addr < base + (static_cast<uintptr_t>(m_iNrAllocatedChunks) << 20));

        uint32_t c = static_cast<uint32_t>((addr - base) >> 20);
        ChunkHeader& chunk = chunk_headers[c];

        --chunk.m_iUsedPages;
        *reinterpret_cast<void**>(ptr) = chunk.m_pFreeListHead;
        chunk.m_pFreeListHead = ptr;

        // Return fully-free trailing chunks to the OS (PC/iOS shrink behavior).
        while (m_iNrAllocatedChunks >= 2
               && c == static_cast<uint32_t>(m_iNrAllocatedChunks - 1)
               && chunk.m_iUsedPages == 0)
        {
            uint32_t last = m_iNrAllocatedChunks - 1;
            void* pChunkAddr = static_cast<char*>(m_pVirtualAllocBase) + (static_cast<uintptr_t>(last) << 20);
            if (!VirtualFree(pChunkAddr, 0x100000, MEM_DECOMMIT))
                break;

            chunk_headers[last].m_pFreeListHead = nullptr;
            chunk_headers[last].m_iUsedPages = 0;
            chunk_headers[last].m_iNextPageIdx = 0;
            --m_iNrAllocatedChunks;
        }
    }

    uint32_t SetMemColor(uint32_t lNewColor)
    {
        auto& pMemReadOut = ZMemReadOut::Instance();

        auto lOldColor = pMemReadOut.SetAllocColor(lNewColor);
        ZDebugSetMemColor(lNewColor);

        return lOldColor;
    }
}
