#pragma once

#include <cstdint>
#include <Glacier/ReGlacier.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZSTL/EAllocType.h>
#include <Glacier/Component/ZComponentSingleton.h>
#include <Glacier/Component/ZGlobalComponentBase.h>


namespace Glacier
{
    struct ZAllocatorBase;

    struct ZSysmemDebugHandler : public ZGlobalComponentBase
    {
        virtual void PreAllocChange(unsigned int*);
        virtual void PostAllocChange(char**, unsigned int*);
        virtual void PreFreeChange(char**);
        virtual void PostAlloc(char*, unsigned int, char*, char*);
        virtual void PreFree(char*);
        virtual void PreShrinkChange(char**, unsigned int*);
        virtual void Reset();
    };

    class ISysMem : public ZComponentSingleton<ISysMem, ZGlobalComponentBase>
    {
    public:
        // types
        enum AllocDirection : int
        {
            AD_FORWARD = 0x0,
            AD_BACKWARD = 0x1,
        };

        // vtbl
        virtual void CreateAllocators();
        virtual void DestroyAllocators();
        virtual void Reset();
        virtual void SetAllocator(EAllocType, ZAllocatorBase*);
        virtual ZAllocatorBase* GetAllocator(EAllocType);
        virtual void SetAllocDirection(AllocDirection);
        virtual void AddDebugHandler(ZSysmemDebugHandler*);
        virtual void RemoveDebugHandler(ZSysmemDebugHandler*);
        virtual void SetFileLine(const char* psFile, int iLine);
        virtual void GetFileLine(const char** ppsFile, int* piLine);

        // methods; DronCode: I think it's better to move it to ZSysMem, but we will have weird casts :(
        void* New(EAllocType eMemType, int iSize);
        void Delete(void* pMem);
    };
    RE_VERIFY_SIZE(ISysMem, 0x10);

    struct ZAllocatorBase
    {
        // vtbl
        virtual ~ZAllocatorBase();
        virtual ISysMem::AllocDirection SetAllocDirection(ISysMem::AllocDirection eDirection);
        virtual char* Alloc(unsigned int, char**, char**);
        virtual bool Free(char*);
        virtual bool Shrink(char*, unsigned int);
        virtual void Reset();
        virtual unsigned int GetFreeTotal();
        virtual unsigned int GetLargestBlock();
        virtual bool ReturnNullOnAllocFail();
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
        // vtbl - no changes
        // const
        static constexpr size_t MAX_NR_DEBUG_HANDLERS = 0x8; // Approved by ZSysMem::AddDebugHandler

        // data
        SAllocatorInfo m_pAllocatorList[EAllocType::END_OF_ALLOCATOR_TYPES];
        ZSysmemDebugHandler* m_pDebugHandlers[MAX_NR_DEBUG_HANDLERS];
        unsigned int m_iNrDebugHandlers;
        const char* m_File;
        int m_Line;
    }; // Verified size 0x54
    RE_VERIFY_SIZE(ZSysMem, 0x54);

    struct ChunkHeader 
    {
        void* free_list_head;
        uint16_t used_pages;
        uint16_t next_page_idx;
    };
    RE_VERIFY_SIZE(ChunkHeader, 0x8);

    struct FineGrainedPool
    {
        RE_ADD_PADDING(0x1C);
    };
    RE_VERIFY_SIZE(FineGrainedPool, 0x1C);

    class IMemoryBlockProvider 
    {
    public:
        virtual void Unknown(); 
        virtual void* AllocateBlock(size_t size, size_t alignment, size_t granularity, size_t flags, size_t* outBlockSize);
        virtual void FreeBlock(int unused, void* ptr);
    };

    class VZWin32Allocator : public IMemoryBlockProvider
    {
    public:
        FineGrainedPool pools[32]; 
        ChunkHeader chunk_headers[256];
        uint32_t allocated_chunks_count;
        uint32_t current_size_or_flags;
        void* virtual_alloc_ptr_base;
        void* win32_heap_handle;
    };

    class ZWin32Allocator : public ZAllocatorBase, public VZWin32Allocator
    {};

    uint32_t SetMemColor(uint32_t lColor);
}