#pragma once

#include <Glacier/GlacierFWD.h>

namespace Glacier
{
    class STRREFTAB
    {
    public:
        // Data
        int32_t m_field4; //0x0004
        int32_t m_field8; //0x0008
        int32_t m_fieldC; //0x000C
        int32_t m_field10; //0x0010
        int32_t m_field14; //0x0014
        int32_t m_field18; //0x0018
        int32_t m_field1C; //0x001C

        // VFTable
        // On PS2 build we have virtual constructor, but in PC only virtual destructor :thinkingface:
        virtual void Destroy(bool);
        virtual void Add(unsigned int);
        virtual void AddUnique(uint);
        virtual void Clear();
        virtual void ClearThis();
        virtual uint Count();
        virtual uint Size();
        virtual uint GetEleSize();
        virtual uint PoolSize();
        virtual void DelRefPtr(uint*);
        virtual bool Exists(uint);
        virtual bool Exists(uint*);
        virtual void* Find(uint);
        virtual uint GetRefNr(int);
        virtual uint GetRefPtrNr(int);
        virtual uint GetIndex(uint);
        virtual void Remove(uint);
        virtual void RemoveIfExists(uint);
        virtual void RunDelRef(RefRun *);
        virtual void RunInitNxtRef(RefRun *);
        virtual void RunInitNxtRef(RefRun *) const;
        virtual void RunInitPrevRef(RefRun *);
        virtual void RunInitPrevRef(RefRun *) const;
        virtual void RunNxtRef(RefRun *);
        virtual void RunNxtRef_1(RefRun *);
        virtual void RunNxtRefPtr(RefRun *);
        virtual void RunNxtRefPtr(RefRun *) const;
        virtual void RunPrevRef(RefRun *);
        virtual void RunPrevRef(RefRun *) const;
        virtual void RunPrevRefPtr(RefRun *);
        virtual void RunPrevRefPtr(RefRun *) const;
        virtual void* operator[](int);
        virtual void RunToRefPtr(RefRun *);
        virtual void DeleteBlock(TabBlk *);
        virtual void* NewBlock(void);
        virtual void* FindStr(const char*);
        virtual void AddStr(const char*);
        virtual void AddAlways(const char*);
        virtual void RemoveStr(const char*);
        virtual bool Exists(const char*);
        virtual void Sort();
    };
}