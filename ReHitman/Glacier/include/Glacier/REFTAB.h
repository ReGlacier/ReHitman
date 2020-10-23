#pragma once

#include <Glacier/Glacier.h>
#include <Glacier/GlacierFWD.h>

namespace Glacier
{
    class REFTAB
    {
    public:
        virtual REFTAB* Release(bool doFreeMem);   //#0 +0 .rdata:007569ec
        virtual void Add(unsigned int);            //#1 +4 .rdata:007569f0
        virtual void AddUnique(uint);              //#2 +8 .rdata:007569f4
        virtual void Clear();                      //#3 +c .rdata:007569f8
        virtual void ClearThis();                  //#4 +10 .rdata:007569fc
        virtual uint Count();                      //#5 +14 .rdata:00756a00
        virtual uint Size();                       //#6 +18 .rdata:00756a04
        virtual uint GetEleSize();                 //#7 +1c .rdata:00756a08
        virtual uint PoolSize();                   //#8 +20 .rdata:00756a0c
        virtual void DelRefPtr(uint*);             //#9 +24 .rdata:00756a10
        virtual bool Exists(uint);                 //#10 +28 .rdata:00756a14
        virtual bool Exists(uint*);                //#11 +2c .rdata:00756a18
        virtual void* Find(uint);                  //#12 +30 .rdata:00756a1c
        virtual uint GetRefNr(int);                //#13 +34 .rdata:00756a20
        virtual uint GetRefPtrNr(int);             //#14 +38 .rdata:00756a24
        virtual uint GetIndex(uint);               //#15 +3c .rdata:00756a28
        virtual void Remove(uint);                 //#16 +40 .rdata:00756a2c
        virtual void RemoveIfExists(uint);         //#17 +44 .rdata:00756a30
        virtual void RunDelRef(RefRun *);          //#18 +48 .rdata:00756a34
        virtual void RunInitNxtRef(RefRun *);      //#19 +4c .rdata:00756a38
        virtual void RunInitNxtRef_1(RefRun *);    //#20 +50 .rdata:00756a3c
        virtual void RunInitPrevRef(RefRun *);     //#21 +54 .rdata:00756a40
        virtual void RunInitPrevRef_1(RefRun *);   //#22 +58 .rdata:00756a44
        virtual void RunNxtRef(RefRun *);          //#23 +5c .rdata:00756a48
        virtual void RunNxtRef_1(RefRun *);        //#24 +60 .rdata:00756a4c
        virtual void RunNxtRefPtr(RefRun *);       //#25 +64 .rdata:00756a50
        virtual void RunNxtRefPtr_1(RefRun *);     //#26 +68 .rdata:00756a54
        virtual void RunPrevRef(RefRun *);         //#27 +6c .rdata:00756a58
        virtual void RunPrevRef_1(RefRun *);       //#28 +70 .rdata:00756a5c
        virtual void RunPrevRefPtr(RefRun *);      //#29 +74 .rdata:00756a60
        virtual void RunPrevRefPtr_1(RefRun *);    //#30 +78 .rdata:00756a64
        virtual void* operator[](int);             //#31 +7c .rdata:00756a68
        virtual void RunToRefPtr(RefRun *);        //#32 +80 .rdata:00756a6c
        virtual void DeleteBlock(TabBlk *);        //#33 +84 .rdata:00756a70
        virtual void* NewBlock(void);              //#34 +88 .rdata:00756a74
    };
}