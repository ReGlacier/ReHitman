#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZUniMemory.h>
#include <cstdint>

namespace Glacier
{
    template <typename T, int A0>
    struct ZSListNodeBase
    {
        // methods
        ZSListNodeBase()
            : m_Next{nullptr}
        {
        }

        ZSListNodeBase<T, A0>* GetNext()
        {
            return reinterpret_cast<ZSListNodeBase<T, A0>*>(m_Next);
        }

        // members
        T* m_Next{nullptr};
    };

    template <typename T, int A0>
    class ZSListBase
    {
    public:
        // methods
        ZSListBase()
        {
            m_First = nullptr;
            m_Last = nullptr;
        }

        ~ZSListBase()
        {
            Unlink();
        }

        void Unlink()
        {
            if (m_First)
            {
                using NodePtr = ZSListNodeBase<T, A0>*;

                NodePtr pCurrent = reinterpret_cast<NodePtr>(m_First);
                NodePtr pLast    = reinterpret_cast<NodePtr>(m_Last);

                while (pCurrent)
                {
                    NodePtr pNext = reinterpret_cast<NodePtr>(pCurrent->m_Next);
                    
                    pCurrent->m_Next = nullptr;

                    if (pCurrent == pLast)
                        break;

                    pCurrent = pNext;
                }

                this->m_First = nullptr;
                this->m_Last  = nullptr;
            }
        }

        void AddFirst(ZSListNodeBase<T, A0>* pEntry)
        {
            pEntry->m_Next = m_First;

            if (!m_First)
            {
                m_Last = reinterpret_cast<T*>(pEntry);
            }

            m_First = reinterpret_cast<T*>(pEntry);
        }

        void AddLast(ZSListNodeBase<T, A0>* pEntry)
        {
            auto pLast = reinterpret_cast<ZSListNodeBase<T, A0>*>(m_Last);

            if (pLast)
            {
                pLast->m_Next = reinterpret_cast<T*>(pEntry);
            }
            else
            {
                m_First = reinterpret_cast<T*>(pEntry);
            }

            m_Last = reinterpret_cast<T*>(pEntry);
            pEntry->m_Next = nullptr;
        }

        int Count() const
        {
            int count = 0;
            auto* pCurrent = reinterpret_cast<const ZSListNodeBase<T, A0>*>(m_First);

            while (pCurrent)
            {
                ++count;
                pCurrent = reinterpret_cast<const ZSListNodeBase<T, A0>*>(pCurrent->m_Next);
            }

            return count;
        }

        ZSListNodeBase<T, A0>& GetFirst()
        {
            return *reinterpret_cast<ZSListNodeBase<T, A0>*>(m_First);
        }

        ZSListNodeBase<T, A0>& GetLast()
        {
            return *reinterpret_cast<ZSListNodeBase<T, A0>*>(m_Last);
        }

        // members
        T* m_First;
        T* m_Last;
    };

    template <typename T, int A0>
    struct ZSListNode : public ZSListNodeBase<T, A0>
    {
        // methods
        ZSListNode()
            : ZSListNodeBase<T, A0>()
        {
        }
    };

    template <typename T, int A0>
    struct ZSListIterator
    {
        // methods
        ZSListIterator(ZSListNode<T, A0>* pNode = nullptr)
            : m_Node(pNode)
        {
        }

        ZSListIterator(const ZSListIterator<T, A0>& copy)
            : m_Node(copy.m_Node)
        {
        }

        bool operator==(const ZSListIterator<T, A0>& rhs) const
        {
            return m_Node == rhs.m_Node;
        }

        bool operator!=(const ZSListIterator<T, A0>& rhs) const
        {
            return m_Node != rhs.m_Node;
        }

        // Префиксный ++
        ZSListIterator<T, A0>& operator++()
        {
            if (m_Node)
            {
                m_Node = reinterpret_cast<ZSListNode<T, A0>*>(m_Node->m_Next);
            }
            return *this;
        }

        ZSListIterator<T, A0> operator++(int)
        {
            ZSListIterator<T, A0> temp = *this;
            if (m_Node)
            {
                m_Node = reinterpret_cast<ZSListNode<T, A0>*>(m_Node->m_Next);
            }
            return temp;
        }

        T& operator*()
        {
            return *reinterpret_cast<T*>(m_Node);
        }

        T* operator->()
        {
            return reinterpret_cast<T*>(m_Node);
        }

        // members
        ZSListNode<T, A0>* m_Node{nullptr};
    };

    template <typename T, bool A0, int A1>
    class ZSList : public ZSListBase<T, A1>
    {
    public:
        // methods
        ZSList() = default;
        ~ZSList() = default;

        ZSListIterator<T, A1> begin()
        {
            return ZSListIterator<T, A1>(reinterpret_cast<ZSListNode<T, A1>*>(this->m_First));
        }

        ZSListIterator<T, A1> end()
        {
            return ZSListIterator<T, A1>(nullptr);
        }

        bool IsEmpty() const
        {
            return this->m_First == nullptr;
        }

        void AddFirst(T* pEntry)
        {
            ZSListBase<T, A1>::AddFirst(reinterpret_cast<ZSListNodeBase<T, A1>*>(pEntry));
        }

        void AddLast(T* pEntry)
        {
            ZSListBase<T, A1>::AddLast(reinterpret_cast<ZSListNodeBase<T, A1>*>(pEntry));
        }

        void DeleteAll()
        {
            T* pCurrent = this->m_First;

            while (pCurrent)
            {
                auto* pNext = pCurrent->m_Next;
                ZUniMemory::Delete(pCurrent);

                if (pCurrent == this->m_Last)
                    break;

                pCurrent = pNext;
            }

            this->m_First = nullptr;
            this->m_Last = nullptr;
        }
    };
}