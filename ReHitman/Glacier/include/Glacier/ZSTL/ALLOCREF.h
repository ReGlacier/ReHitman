#pragma once

#include <Glacier/Glacier.h>

namespace Glacier
{
    class ALLOCREF
    {
    public:
        //vftable
        virtual void Release(bool);
        virtual Glacier::ZREF NewRef();
        virtual int GetActiveRefs(REFTAB* out);
        virtual int GetNrActiveRefs();
        virtual void FreeRef(Glacier::ZREF ref);
        virtual bool CheckRefActive(Glacier::ZREF ref);
        virtual void PrintRef(Glacier::ZREF ref); // Do nothing :(
        virtual REFTAB* GetRefStack();
        virtual Glacier::ZREF* GetUsedRefs();
        virtual void SetRefStack(REFTAB*);
        virtual void SetUsedRefs(Glacier::ZREF* refs, unsigned int count);

        //data
        int* m_usedRefs;
        REFTAB* m_refStack;
        int m_fieldC;
        int m_count;
        int m_capacity;
        bool m_field18;
        bool m_field18;
        bool m_field19;
        bool m_field1A;
        bool m_field1B;
    };
}