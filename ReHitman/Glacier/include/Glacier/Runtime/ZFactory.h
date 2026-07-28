#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/Runtime/ZGEOMCLASSINFO.h>
#include <Glacier/Runtime/ZROUTCLASSINFO.h>
#include <type_traits>
#include <cstdint>
#include <cstring>


namespace Glacier
{
    struct NotCopyable
    {
        NotCopyable() = default;
        NotCopyable(const NotCopyable&) = delete;
        NotCopyable& operator=(const NotCopyable&) = delete;
    };

    #define DECLARE_FACTORY(classInfo_t, typeId_t, creatorTrampoline_t) \
        using ClassInfo_t = classInfo_t; \
        using ProducerId_t = typeId_t; \
        using CreatorTrampoline_t = creatorTrampoline_t;

    template <typename T>
    struct ZFactory : public NotCopyable
    {
        // constants
        static constexpr uint32_t MAX_ENTRIES_IN_BUCKET = 16;

        // type helpers
        using ClassInfo = typename T::ClassInfo_t;
        using ProducerId = typename T::ProducerId_t;
        using CreatorTrampoline = typename T::CreatorTrampoline_t;

        // types
        struct ProducerData : public ClassInfo
        {
            // types
            using CreatorFunction_t = T*(*)(const ClassInfo&);

            // methods
            ~ProducerData() = default;
            ProducerData(ProducerId producerId, CreatorFunction_t pCreator, const ClassInfo& classInfo)
                : ClassInfo(classInfo)
                , m_pCreator(pCreator)
                , m_ProducerId(producerId)
            {}

            // members
            ProducerId m_ProducerId;
            CreatorFunction_t m_pCreator;
            ProducerData* m_pNext;
        };

        struct Iterator
        {
            // methods
            Iterator(ProducerData* p, ZFactory<T>* pFactory, uint32_t hashIndex)
                : m_p(p)
                , m_HashIndex(hashIndex)
                , m_Factory(pFactory)
            {
            }


            Iterator(const Iterator& copy)
                : m_p(copy.m_p)
                , m_HashIndex(copy.m_HashIndex)
                , m_Factory(copy.m_Factory)
            {
            }

            Iterator& operator++()
            {
                m_Factory->Next(m_p, m_HashIndex);
                return *this;
            }

            ProducerData* operator->()
            {
                return m_p;
            }


            ProducerData* operator*()
            {
                return m_p;
            }

            ProducerData* operator*() const
            {
                return m_p;
            }

            bool operator==(const Iterator& rhs) const
            {
                return m_p == rhs.m_p;
            }

            bool operator!=(const Iterator& rhs) const
            {
                return m_p != rhs.m_p;
            }

            // members
            ProducerData* m_p;
            uint32_t m_HashIndex;
            ZFactory<T>* m_Factory;
        };

        // methods
        ~ZFactory() = default;
        ZFactory()
            : m_DataMap{}
        {
        }

        bool Add(ZFactory<T>::ProducerData* pData)
        {
            auto* pFound = Find(pData->m_ProducerId);
            ZASSERT(pFound == nullptr);

            if (pFound)
            {
                return false;
            }

            auto*& pHead = GetData(pData->m_ProducerId);
            pData->m_pNext = pHead;
            pHead = pData;

            return true;
        }

        T* Create(ProducerId id)
        {
            auto* pProducer = Find(id);
            if (pProducer)
            {
                return pProducer->m_pCreator(*pProducer);
            }

            return nullptr;
        }
        
        ProducerData* Find(ProducerId id)
        {
            for (auto* pCurrent = GetData(id); pCurrent; pCurrent = pCurrent->m_pNext)
            {
                if (Equals(pCurrent->m_ProducerId, id))
                {
                    return pCurrent;
                }
            }

            return nullptr;
        }
        
        int32_t Hash(ProducerId id) const
        {
            if constexpr (std::is_same_v<ProducerId, uint32_t>)
            {
                return id;
            }
            else
            {
                int hash = 0;

                while (*id)
                {
                    hash = 5 * hash + static_cast<char>(*id++);
                }

                return hash;
            }
        }

        ZFactory<T>::ProducerData*& GetData(ProducerId id)
        {
            return m_DataMap[Hash(id) & (MAX_ENTRIES_IN_BUCKET - 1)];
        }

        bool Equals(ProducerId k1, ProducerId k2) const 
        { 
            if constexpr (std::is_same_v<ProducerId, uint32_t>)
            {
                return k1 == k2;
            }
            else
            {
               return std::strcmp(k1, k2) == 0;
            }
        }

        Iterator Begin() const
        {
            uint32_t i = 0;

            while (i < MAX_ENTRIES_IN_BUCKET && !m_DataMap[i])
            {
                ++i;
            }

            if (i >= MAX_ENTRIES_IN_BUCKET)
            {
                return End();
            }

            return Iterator(m_DataMap[i], const_cast<ZFactory<T>*>(this), i);
        }

        Iterator End() const
        {
            return Iterator(nullptr, const_cast<ZFactory<T>*>(this), MAX_ENTRIES_IN_BUCKET);
        }

        Iterator begin() const
        {
            return Begin();
        }

        Iterator end() const
        {
            return End();
        }

        void Next(ZFactory<T>::ProducerData*& p, uint32_t& l)
        {
            ZASSERT(p);

            p = p->m_pNext;
            if (!p)
            {
                ++l;

                while (l < 0x10 && !m_DataMap[l])
                {
                    ++l;
                }

                if (l < 0x10)
                {
                    p = m_DataMap[l];
                }
            }
        }

        // members
        ProducerData* m_DataMap[MAX_ENTRIES_IN_BUCKET];
    };

    //RE_VERIFY_SIZE(ZFactory<ZGEOM>, 0x40);
    //RE_VERIFY_SIZE(ZFactory<ZBaseConRout>, 0x40);

    template <typename T, typename TProduced = T>
    struct ZFactoryProducer
    {
        // type helpers
        using ClassInfo = typename ZFactory<T>::ClassInfo;
        using ProducerId = typename ZFactory<T>::ProducerId;
        using CreatorTrampoline = typename ZFactory<T>::CreatorTrampoline;

        // methods
        static T* Create(const ClassInfo& classInfo)
        {
            static_assert(std::is_base_of_v<T, TProduced>, "TProduced must be derived from T");
            return CreatorTrampoline::Create::template Do<TProduced>(classInfo);
        }

        ZFactoryProducer(ProducerId id, const ClassInfo& classInfo)
            : m_Data(id, ZFactoryProducer<T, TProduced>::Create, classInfo)
        {
            T::GetFactory().Add(&m_Data);
        }

        ~ZFactoryProducer() = default;

        // embers
        ZFactory<T>::ProducerData m_Data;
    };
}
