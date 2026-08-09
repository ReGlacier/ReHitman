#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/LINKREFTAB.h>


namespace Glacier
{
    class LINKSORTREFTAB : public LINKREFTAB
    {
    public:
        // vtbl
        ~LINKSORTREFTAB() override;
        void Clear() override;
        void ClearThis() override;
        void DelRefPtr(uint32_t*) override;
        void RunDelRef(RefRun *) override;

        virtual ZREF* AddSort(ZREF r, float, int);
        virtual float GetSort(ZREF* pRefSrt);
        virtual void SetSortNr(ZREF* pRefSrt, float);

        // methods
        LINKSORTREFTAB(int RefsPrBlk, int Exsize);
        void PrintStatus();

        // members
        ZREF* m_pLastAddSort;
    };
    RE_VERIFY_SIZE(LINKSORTREFTAB, 0x2C);
}