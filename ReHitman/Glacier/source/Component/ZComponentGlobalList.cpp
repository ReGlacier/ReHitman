#include <Glacier/Component/ZComponentProducerData.h>
#include <Glacier/Component/ZComponentGlobalList.h>
#include <Glacier/Component/ZComponentBase.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    ZComponentGlobalList::Iterator::Iterator(ZComponentProducerData* p, uint32_t ComponentGroup)
        : m_p(p)
        , m_ComponentGroup(ComponentGroup)
    {
        SkipNotInGroup();
    }

    ZComponentGlobalList::Iterator::Iterator(const ZComponentGlobalList::Iterator& copy)
        : m_p(copy.m_p)
        , m_ComponentGroup(copy.m_ComponentGroup)
    {
    }

    ZComponentGlobalList::Iterator& ZComponentGlobalList::Iterator::operator++()
    {
        m_p = m_p->GetNext();
        SkipNotInGroup();

        return *this;
    }

    ZComponentProducerData* ZComponentGlobalList::Iterator::operator->()
    {
        return m_p;
    }

    ZComponentProducerData& ZComponentGlobalList::Iterator::operator*()
    {
        ZASSERT(m_p != nullptr);
        return *m_p;
    }

    bool ZComponentGlobalList::Iterator::operator==(const ZComponentGlobalList::Iterator& rhs) const
    {
        return m_p == rhs.m_p;
    }

    bool ZComponentGlobalList::Iterator::operator!=(const ZComponentGlobalList::Iterator& rhs) const
    {
        return m_p != rhs.m_p;
    }

    void ZComponentGlobalList::Iterator::SkipNotInGroup()
    {
        while (m_p && m_p->GetComponentGroup() != m_ComponentGroup)
        {
            m_p = m_p->GetNext();
        }
    }

    ZComponentGlobalList& ZComponentGlobalList::Instance()
    {
        static ZComponentGlobalList instance;
        return instance;
    }

    ZComponentGlobalList::ZComponentGlobalList() = default;
    ZComponentGlobalList::~ZComponentGlobalList() = default;

    void ZComponentGlobalList::Add(ZComponentProducerData* pData)
    {
        auto lComponentId = pData->GetComponentId();
        auto* pFound = Find(lComponentId);
        
        if (pFound)
        {
            printf("Duplicate component %s\n", pData->GetComponentId());
            ZASSERT(pFound == nullptr);
            return;
        }

        pData->m_pNext = m_pFirst;
        m_pFirst = pData;
    }

    void ZComponentGlobalList::Remove(ZComponentProducerData* pData)
    {
        if (!pData) return;

        auto lComponentId = pData->GetComponentId();
        auto** ppFound = FindId(&m_pFirst, lComponentId);

        if (ppFound && *ppFound == pData)
        {
            *ppFound = pData->m_pNext;
            pData->m_pNext = nullptr;
        }
    }

    ZComponentProducerData* ZComponentGlobalList::Find(ZComponentId id)
    {
        auto* pFound = FindId(&m_pFirst, id);
        if (pFound)
        {
            return *pFound;
        }

        return nullptr;
    }

    ZComponentProducerData** ZComponentGlobalList::FindId(ZComponentProducerData** list, ZComponentId id)
    {
        while (list)
        {
            auto* pComponentId = (*list)->GetComponentId();
            if (ZComponentProducerData::CompareIds(id, pComponentId))
            {
                break;
            }

            list = &(*list)->m_pNext;
        }

        if (*list)
        {
            return list;
        }

        return nullptr;
    }

    void ZComponentGlobalList::CreateComponents(uint32_t componentGroup)
    {
        constexpr auto SYSTEM_GROUP = MAX_NUMBER_OF_GROUPS - 1;
        
        ZASSERT(componentGroup < MAX_NUMBER_OF_GROUPS);
        if (componentGroup >= MAX_NUMBER_OF_GROUPS) return;

        SortDependencies();
        ++m_RefCounts[componentGroup];

        for (auto it = Begin(componentGroup); it != End(componentGroup); ++it)
        {
            if (!it->IsInitialized())
            {
                if (it->IsCopy())
                {
                    it->SetInstance();
                }
            }
        }

        for (auto it = Begin(SYSTEM_GROUP); it != End(SYSTEM_GROUP); ++it)
        {
            if (!it->IsInitialized())
            {
                if (!it->IsCopy())
                {
                    auto* pCreated = it->Create();
                    it->SetComponent(pCreated);
                    it->GetComponent()->InitializeComponent();
                }
            }
        }
    }

    void ZComponentGlobalList::DestroyComponents(uint32_t componentGroup)
    {
        if (componentGroup >= MAX_NUMBER_OF_GROUPS)
        {
            ZASSERT(componentGroup < MAX_NUMBER_OF_GROUPS);
            return;
        }

        if (!m_RefCounts[componentGroup])
        {
            ZASSERT(m_RefCounts[componentGroup] != 0u);
            return;            
        }

        --m_RefCounts[componentGroup];

        if (!m_RefCounts[componentGroup])
        {
            DestroyReverse(Begin(componentGroup), End(componentGroup));
        }
    }

    ZComponentGlobalList::Iterator ZComponentGlobalList::Begin(uint32_t componentGroup) const
    {
        return { m_pFirst, componentGroup };
    }

    ZComponentGlobalList::Iterator ZComponentGlobalList::End(uint32_t componentGroup) const
    {
        return { nullptr, componentGroup };
    }

    void ZComponentGlobalList::SortDependencies()
    {
        ZComponentProducerData* pData { nullptr };
        ZComponentProducerData* pData1 { nullptr };
        ZComponentProducerData* pTempData { nullptr };

        RemoveChildComponents(&pData);

        while (m_pFirst)
        {
            VisitDependencies(&m_pFirst, &pData1);
        }

        while (pData1)
        {
            pTempData = pData1;
            pData1 = pData1->m_pNext;
            pTempData->m_pNext = m_pFirst;
            m_pFirst = pTempData;
        }

        InsertChildComponents(&pData);
    }

    void ZComponentGlobalList::VisitDependencies(ZComponentProducerData** pSrcList, ZComponentProducerData** pSortedList)
    {
        ZComponentProducerData* pCurrent = *pSrcList;
        *pSrcList = pCurrent->m_pNext;

        uint16_t depCount = pCurrent->DependencySize();
        for (uint16_t i = 0; i < depCount; ++i)
        {
            const char* dependId = pCurrent->GetDependency(i);

            ZASSERT(dependId != nullptr && *dependId != '\0');

            if (ZComponentProducerData::CompareIds(dependId, pCurrent->GetComponentId()))
            {
                printf("Component: %s depends on it self\n", pCurrent->GetComponentId());
            }
            else
            {
                ZComponentProducerData** pFoundInSrc = FindId(&m_pFirst, dependId);
                if (pFoundInSrc != nullptr)
                {
                    VisitDependencies(pFoundInSrc, pSortedList);
                }
                else if (FindId(pSortedList, dependId) == nullptr)
                {
                    printf("Component: %s depends on a nonexisting component: %s\n", 
                            pCurrent->GetComponentId(), 
                            dependId);
                }
            }
        }

        pCurrent->m_pNext = *pSortedList;
        *pSortedList = pCurrent;
    }

    void ZComponentGlobalList::RemoveChildComponents(ZComponentProducerData** pList)
    {
        ZComponentProducerData** ppCurr = &m_pFirst;

        while (*ppCurr)
        {
            if ((*ppCurr)->GetType() == ZComponentProducerData::Types::CHILD)
            {
                ZComponentProducerData* pChild = *ppCurr;
                *ppCurr = pChild->m_pNext; 
                
                pChild->m_pNext = *pList;
                *pList = pChild;
            }
            else
            {
                ppCurr = &(*ppCurr)->m_pNext;
            }
        }
    }

    void ZComponentGlobalList::InsertChildComponents(ZComponentProducerData** pList)
    {
        for (ZComponentProducerData* i = *pList; i != nullptr; )
        {
            ZComponentProducerData* pNext = i->GetNext();
            ZASSERT(i->GetType() == ZComponentProducerData::Types::CHILD);

            ZComponentId pParentId = i->GetParent();
            ZComponentProducerData* pParentNode = Find(pParentId);

            if (pParentNode)
            {
                i->m_pNext = pParentNode->GetNext();
                pParentNode->m_pNext = i;
            }
            else
            {
                printf("Nonexisting parent component: %s for %s\n", i->GetParent(), i->GetComponentId());
            }

            i = pNext;
        }
    }

    void ZComponentGlobalList::DestroyReverse(Iterator begin, Iterator end)
    {
        if (begin == end) return;
        
        ZComponentGlobalList::Iterator next = begin;
        ++next;

        DestroyReverse(next, end);

        ZComponentProducerData* pData = begin.Get();
        if (pData)
        {
            ZComponentBase* pComponent = pData->GetComponent();
            pData->Destroy(pComponent);
            pData->SetComponent(nullptr);
        }
    }
}