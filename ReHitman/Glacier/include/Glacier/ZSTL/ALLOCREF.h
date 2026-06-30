#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Glacier.h>

namespace Glacier
{
    class ALLOCREF
    {
    public:
        // vtbl
        virtual void Release(bool);
        virtual Glacier::ZREF NewRef();
        virtual int GetActiveRefs(REFTAB* out);
        virtual int GetNrActiveRefs();
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