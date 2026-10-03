#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Glacier.h>

namespace Glacier
{
    class ALLOCREF
    {
    public:
        // methods
        ALLOCREF(uint32_t lRefShift, uint32_t lNrActiveRefs);
        
        // vtbl
        virtual ~ALLOCREF();
        virtual Glacier::ZREF NewRef();
        virtual uint32_t GetActiveRefs(REFTAB* out);
        virtual uint32_t GetNrActiveRefs();
        virtual bool FreeRef(Glacier::ZREF ref);
        virtual bool CheckRefActive(Glacier::ZREF ref);
        virtual void PrintRef(Glacier::ZREF ref); // Do nothing :(
        virtual REFTAB* GetRefStack();
        virtual Glacier::ZREF* GetUsedRefs();
        virtual void SetRefStack(REFTAB*);
        virtual void SetUsedRefs(Glacier::ZREF* refs, unsigned int count);

        //data
        unsigned int *m_pUsedRefs;
        REFTAB *m_pRefStack;
        unsigned int m_lActRefNr;
        unsigned int m_lRefAnd;
        unsigned int m_lRefShift;
        bool m_bCheckLock;
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(ALLOCREF, 0x1C);
}