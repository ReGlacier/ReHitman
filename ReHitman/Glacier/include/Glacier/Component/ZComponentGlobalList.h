#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/NotCopyable.h>
#include <Glacier/Component/Component.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    class ZComponentProducerData;

    class ZComponentGlobalList : public NotCopyable
    {
    public:
        // constants
        static constexpr int MAX_NUMBER_OF_GROUPS = ZCOMPONENT_GROUP_COUNT;

        // types
        struct Iterator
        {
            // methods
            Iterator(ZComponentProducerData* p, uint32_t ComponentGroup);
            Iterator(const Iterator& copy);

            Iterator& operator++();

            ZComponentProducerData* operator->();
            ZComponentProducerData& operator*();

            bool operator==(const Iterator& rhs) const;
            bool operator!=(const Iterator& rhs) const;

            void SkipNotInGroup();
            ZComponentProducerData* Get() const { return m_p; }

            // members
            ZComponentProducerData* m_p { nullptr };
            uint32_t m_ComponentGroup { 0u };
        };

        // vtbl
        // methods
        static ZComponentGlobalList& Instance();

        ZComponentGlobalList();
        ~ZComponentGlobalList();

        void Add(ZComponentProducerData* pData);
        void Remove(ZComponentProducerData* pData);
        ZComponentProducerData* Find(ZComponentId id);
        ZComponentProducerData** FindId(ZComponentProducerData**, ZComponentId pszName);
        void CreateComponents(uint32_t componentGroup);
        void DestroyComponents(uint32_t componentGroup);
        Iterator Begin(uint32_t componentGroup) const;
        Iterator End(uint32_t componentGroup) const;
        void SortDependencies();
        void VisitDependencies(ZComponentProducerData** pSrcList, ZComponentProducerData** pSortedList);
        void RemoveChildComponents(ZComponentProducerData** pList);
        void InsertChildComponents(ZComponentProducerData** pList);
        void DestroyReverse(Iterator begin, Iterator end);

        // members
        ZComponentProducerData* m_pFirst { nullptr };
        uint32_t m_RefCounts[MAX_NUMBER_OF_GROUPS] { 0u };
    };
}
