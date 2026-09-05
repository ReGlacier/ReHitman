#include <Glacier/Geom/ZBaseGeomLists.h>
#include <Glacier/Geom/ZBaseGeom.h> // ZBaseGeom
#include <Glacier/ZSTL/ZOffsetAlloc.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>

#include <cstring>


namespace Glacier
{
    namespace
    {
        constexpr uintptr_t QLISTGEOMTYPE = 7;
        constexpr uintptr_t GEOM_STRIDE = sizeof(ZBaseGeom);

        SBaseGeomListHeader* ListFromRef(uintptr_t pList)
        {
            return reinterpret_cast<SBaseGeomListHeader*>(pList);
        }

        uintptr_t RefFromList(SBaseGeomListHeader* pList)
        {
            return reinterpret_cast<uintptr_t>(pList);
        }

        uintptr_t PackGeom(ZBaseGeom* pGeom, uint32_t lType)
        {
            return reinterpret_cast<uintptr_t>(pGeom) | lType;
        }

        ZBaseGeom* UnpackGeom(uintptr_t lGeomID)
        {
            return reinterpret_cast<ZBaseGeom*>(lGeomID & ~QLISTGEOMTYPE);
        }

        ZBaseGeom* Parent(ZBaseGeom* pGeom)
        {
            return pGeom ? pGeom->m_pParent : nullptr;
        }

        ZBaseGeom* ParentFromID(uintptr_t lGeomID)
        {
            return Parent(UnpackGeom(lGeomID));
        }

        uint32_t ListAllocSize(uint32_t lPoolSize)
        {
            return (sizeof(SBaseGeomListHeader) + sizeof(uintptr_t) * lPoolSize + 15) & ~15u;
        }

        char* Payload(SBaseGeomListHeader* pList)
        {
            return reinterpret_cast<char*>(pList) + sizeof(SBaseGeomListHeader);
        }

        uintptr_t GetEntry(SBaseGeomListHeader* pList, uint8_t index)
        {
            uintptr_t value = 0;
            std::memcpy(&value, Payload(pList) + sizeof(uintptr_t) * index, sizeof(value));
            return value;
        }

        void SetEntry(SBaseGeomListHeader* pList, uint8_t index, uintptr_t value)
        {
            std::memcpy(Payload(pList) + sizeof(uintptr_t) * index, &value, sizeof(value));
        }

        void MoveEntries(SBaseGeomListHeader* pList, uint8_t dstIndex, uint8_t srcIndex, uint8_t count)
        {
            std::memmove(
                Payload(pList) + sizeof(uintptr_t) * dstIndex,
                Payload(pList) + sizeof(uintptr_t) * srcIndex,
                sizeof(uintptr_t) * count);
        }
    }

    void SGeomPairRecursion::InitPair(uintptr_t pList)
    {
        DpInsertList = ListFromRef(pList);

        while (DpInsertList && DpInsertList->m_cUsedLen < 2)
        {
            if ((DpInsertList->m_cUsedLen & 1) == 0)
            {
                DpInsertList = nullptr;
                break;
            }

            DpInsertList = ListFromRef(GetEntry(DpInsertList, 0));
        }

        m_cCur = 0;
        m_cCurEnd = 0;
        pSingle = nullptr;

        if (DpInsertList)
        {
            m_cCur = 0;
            if (DpInsertList->m_cUsedLen & 1)
                ++m_cCur;

            m_cCurEnd = DpInsertList->m_cUsedLen;
            ZASSERT(m_cCur != m_cCurEnd);
        }
    }

    void SGeomPairRecursion::NextPair()
    {
        if (m_cCur == m_cCurEnd)
        {
            m_cCur = 0;
            m_cCurEnd = 0;

            while (DpInsertList && (DpInsertList->m_cUsedLen & 1) != 0)
            {
                DpInsertList = ListFromRef(GetEntry(DpInsertList, 0));

                if (DpInsertList && DpInsertList->m_cUsedLen >= 2)
                {
                    m_cCur = 0;
                    if (DpInsertList->m_cUsedLen & 1)
                        ++m_cCur;

                    m_cCurEnd = DpInsertList->m_cUsedLen;
                    return;
                }
            }
        }
    }

    ZBaseGeomLists::ZBaseGeomLists(uint32_t lSize, uint32_t* pFreeList) 
        : ZQStaticMemBlock(lSize, pFreeList)
    {
    }

    bool ZBaseGeomLists::Exists(uintptr_t pList, ZBaseGeom* pGeom, uint32_t lType)
    {
        SGeomPairRecursion recursion;
        recursion.InitPair(pList);

        const uintptr_t lGeom = PackGeom(pGeom, lType);

        while (recursion.DpInsertList && recursion.m_cCur != recursion.m_cCurEnd)
        {
            uintptr_t lFirstGeom = GetEntry(recursion.DpInsertList, recursion.m_cCur++);
            uintptr_t lLastGeom = GetEntry(recursion.DpInsertList, recursion.m_cCur++);

            if ((lGeom & ~QLISTGEOMTYPE) >= (lFirstGeom & ~QLISTGEOMTYPE) &&
                (lGeom & ~QLISTGEOMTYPE) <= (lLastGeom & ~QLISTGEOMTYPE))
            {
                return true;
            }

            recursion.NextPair();
        }

        return false;
    }

    uintptr_t ZBaseGeomLists::AddBaseGeoms(uintptr_t pList, ZBaseGeom* pFirstGeom, ZBaseGeom* pLastGeom, uint32_t lType, uint32_t lMinPoolSize)
    {
        uint32_t lPoolSize = (lMinPoolSize + 7) & ~7u;
        ZASSERT(4 <= lPoolSize);
        ZASSERT(lPoolSize < 256);
        ZASSERT((lType & ~QLISTGEOMTYPE) == 0);

        uintptr_t lFirstGeom = PackGeom(pFirstGeom, lType);
        uintptr_t lLastGeom = PackGeom(pLastGeom, lType);
        ZBaseGeom* pParent = ParentFromID(lFirstGeom);
        uintptr_t pRootList = pList;
        SBaseGeomListHeader* pInsertList = ListFromRef(pList);

        if (pInsertList && pInsertList->m_cUsedLen >= 2)
        {
            SBaseGeomListHeader* pCurrentList = pInsertList;

            while (pCurrentList)
            {
                SBaseGeomListHeader* pThisList = pCurrentList;
                uint8_t cCur = 0;
                uint8_t cCurEnd = pCurrentList->m_cUsedLen;
                pCurrentList = nullptr;

                if (pThisList->m_cUsedLen & 1)
                    pCurrentList = ListFromRef(GetEntry(pThisList, cCur++));

                while (cCur != cCurEnd)
                {
                    uint8_t cPairFirst = cCur;
                    uintptr_t lExistingFirst = GetEntry(pThisList, cCur++);
                    uintptr_t lExistingLast = GetEntry(pThisList, cCur++);

                    ZASSERT((lExistingFirst & QLISTGEOMTYPE) == (lExistingLast & QLISTGEOMTYPE));

                    if ((lExistingFirst & QLISTGEOMTYPE) == lType && ParentFromID(lExistingFirst) == pParent)
                    {
                        if (lLastGeom + GEOM_STRIDE == lExistingFirst)
                        {
                            // cCur was advanced past this pair; cPairFirst is its first geom id.
                            SetEntry(pThisList, cPairFirst, lFirstGeom);
                            return pRootList;
                        }

                        if (lFirstGeom - GEOM_STRIDE == lExistingLast)
                        {
                            // cCur was advanced past this pair; cPairFirst + 1 is its last geom id.
                            SetEntry(pThisList, static_cast<uint8_t>(cPairFirst + 1), lLastGeom);
                            return pRootList;
                        }
                    }
                }
            }
        }

        if (pInsertList && pInsertList->m_cPoolLen - pInsertList->m_cUsedLen < 2)
        {
            SBaseGeomListHeader* pCurrentList = pInsertList;

            while (pCurrentList && pCurrentList->m_cPoolLen - pCurrentList->m_cUsedLen < 2)
            {
                if (pCurrentList->m_cUsedLen & 1)
                    pCurrentList = ListFromRef(GetEntry(pCurrentList, 0));
                else
                    pCurrentList = nullptr;
            }

            if (pCurrentList)
                pInsertList = pCurrentList;
        }

        if (!pInsertList || pInsertList->m_cPoolLen - pInsertList->m_cUsedLen < 2)
        {
            uint32_t lAllocSize = ListAllocSize(lPoolSize);
            void* pOffset = m_pOffsetAlloc->Alloc(lAllocSize, true);
            ZASSERT(pOffset != reinterpret_cast<void*>(-1));

            pInsertList = static_cast<SBaseGeomListHeader*>(GetPtr(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(pOffset))));
            std::memset(pInsertList, 0, lAllocSize);
            pInsertList->m_cPoolLen = static_cast<uint8_t>(lPoolSize);

            if (pRootList)
            {
                pInsertList->m_cUsedLen = 1;
                SetEntry(pInsertList, 0, pRootList);
            }

            pRootList = RefFromList(pInsertList);
        }

        SetEntry(pInsertList, pInsertList->m_cUsedLen, lFirstGeom);
        SetEntry(pInsertList, static_cast<uint8_t>(pInsertList->m_cUsedLen + 1), lLastGeom);
        pInsertList->m_cUsedLen += 2;
        ++pInsertList->m_cTypeLens[lType];

        return pRootList;
    }

    uintptr_t ZBaseGeomLists::RemoveBaseGeoms(uintptr_t pList, ZBaseGeom* pFirstGeom, ZBaseGeom* pLastGeom, uint32_t lType)
    {
        if (!pList)
            return 0;

        uintptr_t pRootList = pList;
        uintptr_t lFirstGeom = PackGeom(pFirstGeom, lType);
        uintptr_t lLastGeom = PackGeom(pLastGeom, lType);
        SBaseGeomListHeader* pCurrentList = ListFromRef(pList);
        SBaseGeomListHeader* pPrevList = nullptr;

        while (true)
        {
            ZASSERT(pCurrentList != nullptr);

            SBaseGeomListHeader* pThisList = pCurrentList;
            uint8_t cCur = 0;
            uint8_t cCurEnd = pThisList->m_cUsedLen;
            pCurrentList = nullptr;

            if (pThisList->m_cUsedLen & 1)
                pCurrentList = ListFromRef(GetEntry(pThisList, cCur++));

            while (cCur != cCurEnd)
            {
                uint8_t cPairFirst = cCur;
                uintptr_t lExistingFirst = GetEntry(pThisList, cCur++);
                uintptr_t lExistingLast = GetEntry(pThisList, cCur++);

                ZASSERT((lExistingFirst & QLISTGEOMTYPE) == (lExistingLast & QLISTGEOMTYPE));

                if ((lExistingFirst & QLISTGEOMTYPE) != lType || lFirstGeom < lExistingFirst || lExistingLast < lLastGeom)
                    continue;

                if (lExistingFirst == lFirstGeom && lExistingLast == lLastGeom)
                {
                    --pThisList->m_cTypeLens[lType];
                    pThisList->m_cUsedLen -= 2;

                    if (cCur == cCurEnd)
                    {
                        if (pThisList->m_cUsedLen < 2)
                        {
                            uintptr_t pNextList = 0;

                            if (pThisList->m_cUsedLen == 1)
                                pNextList = GetEntry(pThisList, 0);

                            if (pPrevList)
                                SetEntry(pPrevList, 0, pNextList);
                            else
                                pRootList = pNextList;

                            uint32_t lFreeOffset = static_cast<uint32_t>(reinterpret_cast<char*>(pThisList) - m_pStart);
                            m_pOffsetAlloc->Free(lFreeOffset, ListAllocSize(pThisList->m_cPoolLen));
                        }
                    }
                    else
                    {
                        MoveEntries(pThisList, cPairFirst, cCur, static_cast<uint8_t>(cCurEnd - cCur));
                    }

                    return pRootList;
                }

                if (lExistingFirst == lFirstGeom && lLastGeom < lExistingLast)
                {
                    // cCur was advanced past this pair; cPairFirst is its first geom id.
                    SetEntry(pThisList, cPairFirst, lLastGeom + GEOM_STRIDE);
                    return pRootList;
                }

                if (lExistingFirst < lFirstGeom && lLastGeom == lExistingLast)
                {
                    // cCur was advanced past this pair; cPairFirst + 1 is its last geom id.
                    SetEntry(pThisList, static_cast<uint8_t>(cPairFirst + 1), lFirstGeom - GEOM_STRIDE);
                    return pRootList;
                }

                ZASSERT(lExistingFirst < lFirstGeom && lLastGeom < lExistingLast);

                // cCur was advanced past this pair; cPairFirst + 1 is its last geom id.
                SetEntry(pThisList, static_cast<uint8_t>(cPairFirst + 1), lFirstGeom - GEOM_STRIDE);
                return AddBaseGeoms(pRootList, UnpackGeom(lLastGeom + GEOM_STRIDE), UnpackGeom(lExistingLast), lType, 8);
            }

            pPrevList = pThisList;
        }
    }
}
