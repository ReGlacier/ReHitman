#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/REFTAB.h>

namespace Glacier
{
    struct PrevNext
    {
        PrevNext *Prev;
        PrevNext *Next;
    };

    class LINKREFTAB : public REFTAB
    {
    public:
        // methods
        LINKREFTAB(int RefsPrBlk, int Exsize);

        // vtbl override
        virtual ~LINKREFTAB();
        uint32_t* Add(uint32_t) override;
        void Clear() override;
        void ClearThis() override;
        int Count() const override;
        void DelRefPtr(uint32_t*) override;
        void RunDelRef(RefRun *) override;
        void RunInitNxtRef(RefRun *) const override;
        void RunInitNxtRef(RefRun *) override;
        void RunInitPrevRef(RefRun *) const override;
        void RunInitPrevRef(RefRun *) override;
        const uint32_t* RunNxtRefPtr(RefRun *) const override;
        uint32_t* RunNxtRefPtr(RefRun *) override;
        const uint32_t* RunPrevRefPtr(RefRun *) const override;
        uint32_t* RunPrevRefPtr(RefRun *) override;
        const uint32_t* RunToRefPtr(RefRun *) const override;
        // vtbl new methods
        virtual void RemoveFreeStack();
        virtual void CreateFreeStack();
        virtual uint32_t* AddStart(uint32_t);
        virtual uint32_t* AddEnd(uint32_t);
        virtual uint32_t* InsertBefore(uint32_t *pBefore, uint32_t rRef);
        virtual uint32_t* GetPrevRefPtr(uint32_t *);
        virtual uint32_t* GetNextRefPtr(uint32_t *);
        virtual uint32_t* GetRefPtrNr(int);
        virtual uint32_t  GetRefNr(int);

        /// === members ===
        PrevNext *First;
        PrevNext *Last;
        REFTAB *FreeStack;

    private:
        PrevNext* GetPrevNext(uint32_t* pRecord)
        {
            return reinterpret_cast<PrevNext*>(pRecord + EleSize - 2);
        }

        uint32_t* GetRecord(PrevNext* pNode)
        {
            return reinterpret_cast<uint32_t*>(pNode) + (2 - EleSize);
        }

        uint32_t* AllocateRecord(uint32_t rRef);
    };
    RE_VERIFY_SIZE(LINKREFTAB, 0x28); // Verified
}
