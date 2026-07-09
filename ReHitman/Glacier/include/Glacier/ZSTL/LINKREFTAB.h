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
        /// === vftable ===
        virtual void RemoveFreeStack(void);
        virtual void CreateFreeStack(void);
        virtual unsigned int * AddStart(unsigned int);
        virtual unsigned int * AddEnd(unsigned int);
        virtual unsigned int * InsertBefore(unsigned int *, unsigned int);
        virtual unsigned int * GetPrevRefPtr(unsigned int *);
        virtual unsigned int * GetNextRefPtr(uint *);
        virtual unsigned int * GetRefPtrNr(int) override;
        virtual unsigned int   GetRefNr(int) const override;

        /// === members ===
        PrevNext *First;
        PrevNext *Last;
        REFTAB *FreeStack;
    };
    RE_VERIFY_SIZE(LINKREFTAB, 0x28); // Verified
}