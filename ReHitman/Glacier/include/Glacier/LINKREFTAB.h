#pragma once

#include <Glacier/GlacierFWD.h>

namespace Glacier
{
    class LINKREFTAB
    {
    public:
        /// === vftable ===
        virtual void Release(bool);
        virtual void Add(uint);
        virtual void AddUnique(uint);
        virtual void Clear();
        virtual void ClearThis();
        virtual int Count();
        virtual int Size();
        virtual size_t GetEleSize();
        virtual size_t PoolSize();
        virtual void DelRefPtr(uint *);
        virtual bool Exists(uint);
        virtual bool Exists(uint *);
        virtual void* Find(uint);
        virtual uint GetRefNr(int) const;
        virtual void* GetRefPtrNr(int) const;
        virtual int GetIndex(uint);
        virtual void Remove(uint);
        virtual void RemoveIfExists(uint);
        virtual void RunDelRef(void*);
        virtual void RunInitNxtRef(void *);
        virtual void RunInitNxtRef(void *) const;
        virtual void RunInitPrevRef(void *);
        virtual void RunInitPrevRef(void *) const;
        virtual void RunNxtRef(void *);
        virtual void RunNxtRef(void *) const;
        virtual void RunNxtRefPtr(void *);
        virtual void RunNxtRefPtr(void *) const;
        virtual void RunPrevRef(void *);
        virtual void RunPrevRef(void *) const;
        virtual void RunPrevRefPtr(void *);
        virtual void RunPrevRefPtr(void *) const;
        virtual void* operator[](int);
        virtual void RunToRefPtr(void *);
        virtual void DeleteBlock(void *);
        virtual void* NewBlock(void);
        virtual void RemoveFreeStack(void);
        virtual void CreateFreeStack(void);
        virtual void AddStart(uint);
        virtual void AddEnd(uint);
        virtual void InsertBefore(uint *,uint);
        virtual int GetPrevRefPtr(uint *);
        virtual int GetNextRefPtr(uint *);
        virtual int GetRefPtrNr(int);
        virtual int GetRefNr(int);

        /// === members ===
        int field_4;
        int field_8;
        int field_c;
        int field_10;
        int field_14;
        int field_18;
        int field_1c;
        int field_20;
        int field_24;
    };

}