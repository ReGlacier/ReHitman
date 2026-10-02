#pragma once

#include <cstdint>
#include <Glacier/ReGlacier.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZSTL/EAllocType.h>
#include <Glacier/Component/ZComponentSingleton.h>
#include <Glacier/Component/ZGlobalComponentBase.h>
#include <Glacier/System/ZSysmemDebugHandler.h>


namespace Glacier
{
    struct ZWin32Page;
    struct ZAllocatorBase;

    class ISysMem : public ZComponentSingleton<ISysMem, ZGlobalComponentBase>
    {
    public:
        // types
        enum AllocDirection : int
        {
            AD_FORWARD = 0x0,
            AD_BACKWARD = 0x1,
        };

        // vtbl (abstract interface; the concrete implementation is ZSysMem)
        virtual void CreateAllocators() = 0;
        virtual void DestroyAllocators() = 0;
        virtual void Reset() = 0;
        virtual void SetAllocator(EAllocType, ZAllocatorBase*) = 0;
        virtual ZAllocatorBase* GetAllocator(EAllocType) = 0;
        virtual void SetAllocDirection(AllocDirection) = 0;
        virtual void AddDebugHandler(ZSysmemDebugHandler*) = 0;
        virtual void RemoveDebugHandler(ZSysmemDebugHandler*) = 0;
        virtual void SetFileLine(const char* psFile, int iLine) = 0;
        virtual void GetFileLine(const char** ppsFile, int* piLine) = 0;

        // Non-virtual helpers. PC defines these on the concrete ZSysMem; the
        // singleton instance is always a ZSysMem, so these forward by downcast.
        void* New(EAllocType eMemType, int iSize);
        void Delete(void* pMem);
        bool Shrink(char*& pMem, unsigned int& iNewSize);
    };
    RE_VERIFY_SIZE(ISysMem, 0x10);

    struct ZAllocatorBase
    {
        // vtbl (PC base vtable @ 0x75DE50: dtor is concrete, rest are _purecall)
        virtual ~ZAllocatorBase() {}
        virtual ISysMem::AllocDirection SetAllocDirection(ISysMem::AllocDirection eDirection) = 0;
        virtual char* Alloc(unsigned int, char**, char**) = 0;
        virtual bool Free(char*) = 0;
        virtual bool Shrink(char*, unsigned int) = 0;
        virtual void Reset() = 0;
        virtual unsigned int GetFreeTotal() = 0;
        virtual unsigned int GetLargestBlock() = 0;
        virtual bool ReturnNullOnAllocFail() = 0;
    };
    RE_VERIFY_SIZE(ZAllocatorBase, 0x4);

    struct SAllocatorInfo
    {
        ZAllocatorBase* m_pAllocator;
    };
    RE_VERIFY_SIZE(SAllocatorInfo, 0x4);

    class ZSysMem : public ISysMem
    {
    public:
        // vtbl - no new slots, inherits ISysMem
        // ISysMem overrides
        void CreateAllocators() override;
        void DestroyAllocators() override;
        void Reset() override;
        void SetAllocator(EAllocType eMemType, ZAllocatorBase* pAlloc) override;
        ZAllocatorBase* GetAllocator(EAllocType eMemType) override;
        void SetAllocDirection(AllocDirection) override {} // Empty on PC (nullsub)
        void AddDebugHandler(ZSysmemDebugHandler* pHandler) override;
        void RemoveDebugHandler(ZSysmemDebugHandler* pHandler) override;
        void SetFileLine(const char* psFile, int iLine) override;
        void GetFileLine(const char** ppsFile, int* piLine) override;

        // methods (PC: ?New@ZSysMem / ZSysMem::Delete, not part of ISysMem)
        char* New(EAllocType eMemType, unsigned int iSize);
        void Delete(char* pMem);
        bool Shrink(char*& pMem, unsigned int& iNewSize); // PC sub_446940

        // const
        static constexpr size_t MAX_NR_DEBUG_HANDLERS = 0x8; // Approved by ZSysMem::AddDebugHandler

        // ctor/dtor
        ZSysMem();
        ~ZSysMem() override;

    private:
        // Process-wide lock guarding the allocator list and debug handlers.
        static void InitCriticalSection();
        static void EnterCriticalSection();
        static void LeaveCriticalSection();

    public:
        // data
        SAllocatorInfo m_pAllocatorList[EAllocType::END_OF_ALLOCATOR_TYPES];
        ZSysmemDebugHandler* m_pDebugHandlers[MAX_NR_DEBUG_HANDLERS];
        unsigned int m_iNrDebugHandlers;
        const char* m_File;
        int m_Line;
    }; // Verified size 0x54
    RE_VERIFY_SIZE(ZSysMem, 0x54);

    // Describes a 1 MiB committed chunk of the reserved VirtualAlloc region.
    // Used by the block provider (VZWin32Allocator) to hand out 64 KiB blocks.
    struct ChunkHeader
    {
        void* m_pFreeListHead; // singly linked list of freed 64 KiB blocks in this chunk
        uint16_t m_iUsedPages; // number of 64 KiB pages currently handed out
        uint16_t m_iNextPageIdx; // next 64 KiB page index (0..15) to carve if free list empty
    };
    RE_VERIFY_SIZE(ChunkHeader, 0x8);

    // Header placed at the start of a 64 KiB block that backs a FineGrainedPool.
    // The block data (block size m_iBlockSize) starts at this header + 0x20.
    struct ZWin32Page
    {
        uint32_t m_iBlockSize; // size of each managed block
        uint32_t m_iUsed; // number of blocks currently allocated from this page
        uint32_t m_iNextBlockIdx; // index of the next never-used block to carve
        uint32_t m_iCapacity; // number of blocks that fit in the page (0x10000 - header)
        void* m_pFreeListHead; // singly linked list of freed blocks
        ZWin32Page* m_pNext; // next page in the pool's page chain
        ZWin32Page* m_pPrev; // previous page in the pool's page chain
        uint32_t m_iPad;
    };
    RE_VERIFY_SIZE(ZWin32Page, 0x20);

    // One bucket of the small-object allocator. Bucket i (0..31) manages blocks
    // of size 8 * (i + 1). Blocks are carved out of 64 KiB pages obtained from
    // the owning allocator through IMemoryBlockProvider.
    struct FineGrainedPool
    {
        uint8_t m_bUsed; // initialized to 1; cleared on teardown
        uint8_t m_pad[3];
        class IMemoryBlockProvider* m_pBlockProvider; // owner (VZWin32Allocator subobject)
        uint32_t m_iBlockSize; // block size used when requesting pages (== 8 * (i + 1))
        uint32_t m_iBlockStride; // block size used for indexing inside a page
        ZWin32Page* m_pCurrentPage; // page used for the next allocation
        ZWin32Page* m_pPageHead; // first page of the chain
        ZWin32Page* m_pPageTail; // last page of the chain
    };
    RE_VERIFY_SIZE(FineGrainedPool, 0x1C);

    class IMemoryBlockProvider
    {
    public:
        virtual void Unknown() = 0;
        virtual void* AllocateBlock(size_t size, size_t alignment, size_t granularity, size_t flags, size_t* outBlockSize) = 0;
        virtual void FreeBlock(int unused, void* ptr) = 0;
    };
    RE_VERIFY_SIZE(IMemoryBlockProvider, 0x4);

    // Backing store for ZWin32Allocator. Reserves a large VirtualAlloc region,
    // commits it 1 MiB at a time, and hands out 64 KiB blocks via the
    // IMemoryBlockProvider interface. Small blocks are managed by FineGrainedPool.
    class VZWin32Allocator : public IMemoryBlockProvider
    {
    public:
        // IMemoryBlockProvider overrides (PC: sub_44D0F0 / sub_44D130)
        void Unknown() override;
        void* AllocateBlock(size_t size, size_t alignment, size_t granularity, size_t flags, size_t* outBlockSize) override;
        void FreeBlock(int unused, void* ptr) override;

        // 64 KiB sub-allocation from the committed 1 MiB chunks.
        void* GetBlockFromChunks(); // PC: sub_44CE80
        void* CommitNewChunk();     // PC: sub_44CF00

        // members (data starts at +0x4 after the IMemoryBlockProvider vptr)
        FineGrainedPool pools[32];
        ChunkHeader chunk_headers[256];
        uint32_t m_iNrAllocatedChunks;
        void* m_pVirtualAllocBase;
        void* m_pWin32Heap;
    };
    RE_VERIFY_SIZE(VZWin32Allocator, 0xB90);

    class ZWin32Allocator : public ZAllocatorBase, public VZWin32Allocator
    {
    public:
        // ZAllocatorBase overrides (PC vtable @ 0x75EC4C)
        ~ZWin32Allocator() override;
        ISysMem::AllocDirection SetAllocDirection(ISysMem::AllocDirection eDirection) override;
        char* Alloc(unsigned int uiSize, char** ppRealStart, char** ppRealEnd) override;
        bool Free(char* pMem) override;
        bool Shrink(char*, unsigned int) override;
        void Reset() override;
        unsigned int GetFreeTotal() override;
        unsigned int GetLargestBlock() override;
        bool ReturnNullOnAllocFail() override;

        ZWin32Allocator();

    private:
        // Fine-grained pool helpers (PC: sub_44D210 / sub_44D2C0 / sub_44CF90 /
        // sub_44D020 / sub_44D0A0).
        static void* PoolAlloc(FineGrainedPool* pPool);
        static void PoolFree(FineGrainedPool* pPool, ZWin32Page* pPage, char* pBlock);
        static ZWin32Page* PoolNewPage(FineGrainedPool* pPool);
        static void PoolReleasePage(FineGrainedPool* pPool, ZWin32Page* pPage);
        static void PoolReset(FineGrainedPool* pPool);
    };
    RE_VERIFY_SIZE(ZWin32Allocator, 0xB94);

    // Object-relative offsets verified against the PC constructor (sub_44D450):
    // the VZWin32Allocator subobject starts at +0x4 (its IMemoryBlockProvider vptr),
    // so pools land at +0x8 and the trailing bookkeeping at +0xB88/+0xB8C/+0xB90.
    RE_VERIFY_OFFSET(ZWin32Allocator, pools, 0x8);
    RE_VERIFY_OFFSET(ZWin32Allocator, chunk_headers, 0x388);
    RE_VERIFY_OFFSET(ZWin32Allocator, m_iNrAllocatedChunks, 0xB88);
    RE_VERIFY_OFFSET(ZWin32Allocator, m_pVirtualAllocBase, 0xB8C);
    RE_VERIFY_OFFSET(ZWin32Allocator, m_pWin32Heap, 0xB90);

    uint32_t SetMemColor(uint32_t lColor);
}
