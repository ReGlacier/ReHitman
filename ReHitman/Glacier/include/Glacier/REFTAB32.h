#pragma once

#include <Glacier/GlacierFWD.h>

namespace Glacier
{
    // Class definition REFTAB32
    class REFTAB32
    {
    public:
        // === members ===
        uint32_t m_pStart; //0x0004
        char pad_0008[28]; //0x0008
        int32_t m_itemsCount; //0x0024
        char pad_0028[4]; //0x0028
        uint32_t m_firstItem; //0x002C
        char pad_0030[128]; //0x0030

    public:
        virtual void Release_REFTAB32(bool); //#0000 at 00117DC0 org REFTAB32::~REFTAB32()
        virtual void Add(unsigned int); //#0001 at 00117358 org REFTAB::Add(unsigned int)
        virtual void AddUnique(unsigned int); //#0002 at 00117420 org REFTAB::AddUnique(unsigned int)
        virtual void Clear(); //#0003 at 00117480 org REFTAB::Clear(void)
        virtual void ClearThis(); //#0004 at 00117494 org REFTAB::ClearThis(void)
        virtual size_t Count(); //#0005 at 0011752C org REFTAB::Count(void)const
        virtual size_t Size(); //#0006 at 00117534 org REFTAB::Size(void)const
        virtual size_t GetEleSize(); //#0007 at 0043A0CC org REFTAB::GetEleSize(void)const
        virtual size_t PoolSize(); //#0008 at 0011753C org REFTAB::PoolSize(void)
        virtual void DelRefPtr(unsigned int *); //#0009 at 00117544 org REFTAB::DelRefPtr(unsigned int *)
        virtual bool Exists(unsigned int); //#0010 at 001175B8 org REFTAB::Exists(unsigned int)const
        virtual bool Exists(unsigned int *); //#0011 at 00117628 org REFTAB::Exists(unsigned int *)const
        virtual void* Find(unsigned int); //#0012 at 00117718 org REFTAB::Find(unsigned int)
        virtual void* GetRefNr(int); //#0013 at 00117788 org REFTAB::GetRefNr(int)const
        virtual void* GetRefPtrNr(int); //#0014 at 001177E0 org REFTAB::GetRefPtrNr(int)const
        virtual size_t GetIndex(unsigned int); //#0015 at 00117698 org REFTAB::GetIndex(unsigned int)const
        virtual void Remove(unsigned int); //#0016 at 00117834 org REFTAB::Remove(unsigned int)
        virtual void RemoveIfExists(unsigned int); //#0017 at 00117858 org REFTAB::RemoveIfExists(unsigned int)
        virtual void RunDelRef(RefRun *); //#0018 at 00117BF0 org REFTAB::RunDelRef(RefRun *)
        virtual void RunInitNxtRef(RefRun *); //#0019 at 001178DC org REFTAB::RunInitNxtRef(RefRun *)
        virtual void RunInitNxtRef(RefRun *) const; //#0020 at 00117908 org REFTAB::RunInitNxtRef(RefRun *)const
        virtual void RunInitPrevRef(RefRun *); //#0021 at 00117920 org REFTAB::RunInitPrevRef(RefRun *)
        virtual void RunInitPrevRef(RefRun *) const; //#0022 at 0011795C org REFTAB::RunInitPrevRef(RefRun *)const
        virtual void RunNxtRef(RefRun *); //#0023 at 00117988 org REFTAB::RunNxtRef(RefRun *)
        virtual void RunNxtRef(RefRun *) const; //#0024 at 001179BC org REFTAB::RunNxtRef(RefRun *)const
        virtual void RunNxtRefPtr(RefRun *); //#0025 at 001179F0 org REFTAB::RunNxtRefPtr(RefRun *)
        virtual void RunNxtRefPtr(RefRun *) const; //#0026 at 00117A64 org REFTAB::RunNxtRefPtr(RefRun *)const
        virtual void RunPrevRef(RefRun *); //#0027 at 00117AD8 org REFTAB::RunPrevRef(RefRun *)
        virtual void RunPrevRef(RefRun *) const; //#0028 at 00117B0C org REFTAB::RunPrevRef(RefRun *)const
        virtual void RunPrevRefPtr(RefRun *); //#0029 at 00117B40 org REFTAB::RunPrevRefPtr(RefRun *)
        virtual void RunPrevRefPtr(RefRun *) const; //#0030 at 00117B98 org REFTAB::RunPrevRefPtr(RefRun *)const
        virtual void* operator[](size_t index); //#0031 at 0043A0D4 org REFTAB::operator[](int)const
        virtual void RunToRefPtr(RefRun *); //#0032 at 00117D38 org REFTAB::RunToRefPtr(RefRun *)
        virtual void DeleteBlock(TabBlk *); //#0033 at 00117DFC org REFTAB32::DeleteBlock(TabBlk *)
        virtual void* NewBlock(); //#0034 at 00117E2C org REFTAB32::NewBlock(void)
    }; //End of REFTAB32 from 0054FBE0

    template <typename T> T* get(REFTAB32* reftab, size_t index)
    {
        return reftab ? reftab->operator[](index) : nullptr;
    }
}