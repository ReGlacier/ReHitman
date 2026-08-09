#include <Glacier/ZSTL/LINKSORTREFTAB.h>

#include <cstring>
#include <cstdio>


namespace Glacier
{
    LINKSORTREFTAB::LINKSORTREFTAB(int RefsPrBlk, int Exsize)
        : LINKREFTAB(RefsPrBlk, Exsize + 1), m_pLastAddSort(nullptr)
    {
    }

    LINKSORTREFTAB::~LINKSORTREFTAB() = default;

    void LINKSORTREFTAB::DelRefPtr(uint32_t* pRef)
    {
        if (m_pLastAddSort == pRef)
            m_pLastAddSort = nullptr;

        LINKREFTAB::DelRefPtr(pRef);
    }

    void LINKSORTREFTAB::RunDelRef(RefRun* pRefRun)
    {
        if (m_pLastAddSort == RunToRefPtr(pRefRun))
            m_pLastAddSort = nullptr;

        LINKREFTAB::RunDelRef(pRefRun);
    }

    ZREF* LINKSORTREFTAB::AddSort(ZREF rRef, float sort, int mode)
    {
        ZREF* pBefore = m_pLastAddSort;

        if (pBefore)
        {
            while (pBefore && sort < GetSort(pBefore))
                pBefore = GetPrevRefPtr(pBefore);

            if (mode == 1)
            {
                while (pBefore && sort <= GetSort(pBefore))
                    pBefore = GetPrevRefPtr(pBefore);
            }

            if (!pBefore)
            {
                m_pLastAddSort = reinterpret_cast<ZREF*>(AddStart(rRef)) - 1;
                reinterpret_cast<float*>(m_pLastAddSort)[EleSize - 3] = sort;
                return m_pLastAddSort + 1;
            }
        }
        else if (Last)
        {
            pBefore = reinterpret_cast<ZREF*>(reinterpret_cast<uint32_t*>(Last) + (2 - EleSize));
        }

        while (pBefore && GetSort(pBefore) < sort)
            pBefore = GetNextRefPtr(pBefore);

        if (mode == 2)
        {
            while (pBefore && GetSort(pBefore) <= sort)
                pBefore = GetNextRefPtr(pBefore);
        }

        if (pBefore)
            m_pLastAddSort = reinterpret_cast<ZREF*>(InsertBefore(pBefore, rRef)) - 1;
        else
            m_pLastAddSort = reinterpret_cast<ZREF*>(AddEnd(rRef)) - 1;

        reinterpret_cast<float*>(m_pLastAddSort)[EleSize - 3] = sort;
        return m_pLastAddSort + 1;
    }

    float LINKSORTREFTAB::GetSort(ZREF* pRefSrt)
    {
        return reinterpret_cast<float*>(pRefSrt)[EleSize - 3];
    }

    void LINKSORTREFTAB::SetSortNr(ZREF* pRefSrt, float sort)
    {
        ZREF* pNewRef = AddSort(*pRefSrt, sort, 0) - 1;

        std::memcpy(pNewRef, pRefSrt, (EleSize - 4) * sizeof(uint32_t));
        DelRefPtr(pRefSrt);
    }

    void LINKSORTREFTAB::Clear()
    {
        ClearThis();
        LINKREFTAB::Clear();
    }

    void LINKSORTREFTAB::ClearThis()
    {
        m_pLastAddSort = nullptr;
        LINKREFTAB::ClearThis();
    }

    void LINKSORTREFTAB::PrintStatus()
    {
        RefRun run;
        RunInitNxtRef(&run);

        while (ZREF* pRef = reinterpret_cast<ZREF*>(RunNxtRefPtr(&run)))
            std::printf("Ref %x Sort Value %f\n", *pRef, GetSort(pRef));
    }
}
