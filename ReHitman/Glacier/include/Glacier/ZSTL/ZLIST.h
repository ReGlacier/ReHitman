#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZUniAssert.h>
#include <cstddef>
#include <cstdint>

namespace Glacier
{
    template <typename T, size_t N>
    struct ZListNode;

    /**
     * @brief Intrusive doubly-linked list node used by Glacier list containers.
     *
     * A node is linked when both m_Next and m_Prev are non-null. Standalone nodes keep
     * both pointers null. List heads are sentinel nodes whose next/prev pointers point
     * back to the head when the list is empty.
     */
    template <typename T>
    struct ZListNodeBase
    {
        template <typename, size_t>
        friend struct ZListIterator;

        template <typename>
        friend struct ZListBase;

        template <typename, bool, int>
        friend struct ZList;

    private:
        static ZListNodeBase<T>* LinkToNode(ZListNodeBase<T>* pLink);
        static const ZListNodeBase<T>* LinkToNode(const ZListNodeBase<T>* pLink);
        static ZListNodeBase<T>* NodeToLink(ZListNodeBase<T>* pNode);
        static const ZListNodeBase<T>* NodeToLink(const ZListNodeBase<T>* pNode);
        static T* NodeToObject(ZListNodeBase<T>* pNode);
        static const T* NodeToObject(const ZListNodeBase<T>* pNode);

    public:
        // members
        ZListNodeBase<T>* m_Next;
        ZListNodeBase<T>* m_Prev;

        // methods
        ZListNodeBase() : m_Next(nullptr), m_Prev(nullptr) {}
        ZListNodeBase(ZListNodeBase<T>* pNext, ZListNodeBase<T>* pPrev) : m_Next(pNext), m_Prev(pPrev) {}
        ~ZListNodeBase() { Unlink(); }

        bool IsLinked() const 
        { 
            return m_Next != nullptr; 
        }

        void Unlink()
        {
            if (m_Next && m_Prev)
            {
                LinkToNode(m_Next)->m_Prev = m_Prev;
                LinkToNode(m_Prev)->m_Next = m_Next;
                m_Next = nullptr;
                m_Prev = nullptr;
            }
        }

        void AddPrev(ZListNodeBase<T>* pNode)
        {
            ZASSERT(!pNode->IsLinked());

            pNode->m_Next = NodeToLink(this);
            pNode->m_Prev = m_Prev;
            LinkToNode(m_Prev)->m_Next = NodeToLink(pNode);
            m_Prev = NodeToLink(pNode);
        }

        void AddNext(ZListNodeBase<T>* pNode)
        {
            ZASSERT(!pNode->IsLinked());

            pNode->m_Next = m_Next;
            pNode->m_Prev = NodeToLink(this);
            LinkToNode(m_Next)->m_Prev = NodeToLink(pNode);
            m_Next = NodeToLink(pNode);
        }
    };
    RE_VERIFY_SIZE(ZListNodeBase<int>, 0x8);

    template <typename T, size_t N>
    struct ZListNode : ZListNodeBase<T>
    {
        // methods
        ZListNode() = default;
        ~ZListNode() = default;

        // members
    };

    template <typename T>
    inline ZListNodeBase<T>* ZListNodeBase<T>::LinkToNode(ZListNodeBase<T>* pLink)
    {
        return pLink ? static_cast<ZListNodeBase<T>*>(static_cast<ZListNode<T, 0>*>(reinterpret_cast<T*>(pLink))) : nullptr;
    }

    template <typename T>
    inline const ZListNodeBase<T>* ZListNodeBase<T>::LinkToNode(const ZListNodeBase<T>* pLink)
    {
        return pLink ? static_cast<const ZListNodeBase<T>*>(static_cast<const ZListNode<T, 0>*>(reinterpret_cast<const T*>(pLink))) : nullptr;
    }

    template <typename T>
    inline ZListNodeBase<T>* ZListNodeBase<T>::NodeToLink(ZListNodeBase<T>* pNode)
    {
        return pNode ? reinterpret_cast<ZListNodeBase<T>*>(static_cast<T*>(static_cast<ZListNode<T, 0>*>(pNode))) : nullptr;
    }

    template <typename T>
    inline const ZListNodeBase<T>* ZListNodeBase<T>::NodeToLink(const ZListNodeBase<T>* pNode)
    {
        return pNode ? reinterpret_cast<const ZListNodeBase<T>*>(static_cast<const T*>(static_cast<const ZListNode<T, 0>*>(pNode))) : nullptr;
    }

    template <typename T>
    inline T* ZListNodeBase<T>::NodeToObject(ZListNodeBase<T>* pNode)
    {
        return pNode ? static_cast<T*>(static_cast<ZListNode<T, 0>*>(pNode)) : nullptr;
    }

    template <typename T>
    inline const T* ZListNodeBase<T>::NodeToObject(const ZListNodeBase<T>* pNode)
    {
        return pNode ? static_cast<const T*>(static_cast<const ZListNode<T, 0>*>(pNode)) : nullptr;
    }

    template <typename T, size_t U0>
    struct ZListIterator
    {
        // methods
        ZListIterator(ZListNode<T,U0>* pNode) : m_Node(pNode) {}
        ZListIterator(const ZListIterator<T,U0>& copy) : m_Node(copy.m_Node) {}

        const T& operator*() { return *NodeToObject(m_Node); }
        const T* operator->() { return NodeToObject(m_Node); }
        bool operator==(const ZListIterator<T, U0>& rhs) const { return m_Node == rhs.m_Node; }
        bool operator!=(const ZListIterator<T, U0>& rhs) const { return m_Node != rhs.m_Node; }
        ZListIterator<T, U0>& operator++()
        {
            m_Node = NodeToNode(m_Node ? ZListNodeBase<T>::LinkToNode(m_Node->m_Next) : nullptr);
            return *this;
        }

        ZListIterator<T, U0> operator++(int)
        {
            ZListIterator<T, U0> result(*this);
            ++*this;
            return result;
        }

        ZListIterator<T, U0>& operator--()
        {
            m_Node = NodeToNode(m_Node ? ZListNodeBase<T>::LinkToNode(m_Node->m_Prev) : nullptr);
            return *this;
        }

        ZListIterator<T, U0> operator--(int)
        {
            ZListIterator<T, U0> result(*this);
            --*this;
            return result;
        }
        operator const T*() const { return NodeToObject(m_Node); }
        operator T*() { return NodeToObject(m_Node); }

        // members
        ZListNode<T, U0>* m_Node;

    private:
        static T* NodeToObject(ZListNode<T, U0>* pNode)
        {
            return pNode ? static_cast<T*>(pNode) : nullptr;
        }

        static const T* NodeToObject(const ZListNode<T, U0>* pNode)
        {
            return pNode ? static_cast<const T*>(pNode) : nullptr;
        }

        static ZListNode<T, U0>* ObjectToNode(T* pObject)
        {
            return pObject ? static_cast<ZListNode<T, U0>*>(pObject) : nullptr;
        }

        static ZListNode<T, U0>* NodeToNode(ZListNodeBase<T>* pNode)
        {
            return pNode ? static_cast<ZListNode<T, U0>*>(pNode) : nullptr;
        }
    };

    /**
     * @brief Intrusive circular doubly-linked list with a sentinel head node.
     *
     * The list does not own its nodes. Destroying or clearing the list only unlinks
     * nodes from this list; it does not destroy the objects that contain them.
     */
    template <typename T>
    struct ZListBase
    {
        ZListNodeBase<T> m_Head;

        ZListBase() : m_Head(ZListNodeBase<T>::NodeToLink(reinterpret_cast<ZListNodeBase<T>*>(this)), ZListNodeBase<T>::NodeToLink(reinterpret_cast<ZListNodeBase<T>*>(this))) {}
        ~ZListBase() { UnlinkAll(); }

        void UnlinkAll()
        {
            while (m_Head.m_Next != ZListNodeBase<T>::NodeToLink(reinterpret_cast<ZListNodeBase<T>*>(this)))
                ZListNodeBase<T>::LinkToNode(m_Head.m_Next)->Unlink();
        }

        int Count() const
        {
            int result = 0;
            const auto* pEnd = ZListNodeBase<T>::NodeToLink(reinterpret_cast<const ZListNodeBase<T>*>(this));
            for (auto* pLink = m_Head.m_Next; pLink != pEnd; pLink = ZListNodeBase<T>::LinkToNode(pLink)->m_Next)
                ++result;

            return result;
        }

        ZListNodeBase<T>* GetFirst()
        {
            return m_Head.m_Next != ZListNodeBase<T>::NodeToLink(reinterpret_cast<ZListNodeBase<T>*>(this)) ? ZListNodeBase<T>::LinkToNode(m_Head.m_Next) : nullptr;
        }

        const ZListNodeBase<T>* GetFirst() const
        {
            return m_Head.m_Next != ZListNodeBase<T>::NodeToLink(reinterpret_cast<const ZListNodeBase<T>*>(this)) ? ZListNodeBase<T>::LinkToNode(m_Head.m_Next) : nullptr;
        }

        ZListNodeBase<T>* GetLast()
        {
            return m_Head.m_Prev != ZListNodeBase<T>::NodeToLink(reinterpret_cast<ZListNodeBase<T>*>(this)) ? ZListNodeBase<T>::LinkToNode(m_Head.m_Prev) : nullptr;
        }

        const ZListNodeBase<T>* GetLast() const
        {
            return m_Head.m_Prev != ZListNodeBase<T>::NodeToLink(reinterpret_cast<const ZListNodeBase<T>*>(this)) ? ZListNodeBase<T>::LinkToNode(m_Head.m_Prev) : nullptr;
        }
    };
    RE_VERIFY_SIZE(ZListBase<int>, 0x8);

    template <typename T, bool A, int B>
    struct ZList : ZListBase<T>
    {
        // methods
        const ZListIterator<T, B> Begin() const
        {
            return ZListIterator<T, B>(static_cast<ZListNode<T, B>*>(const_cast<ZListNodeBase<T>*>(ZListNodeBase<T>::LinkToNode(this->m_Head.m_Next))));
        }

        ZListIterator<T, B> Begin()
        {
            return ZListIterator<T, B>(static_cast<ZListNode<T, B>*>(ZListNodeBase<T>::LinkToNode(this->m_Head.m_Next)));
        }

        const ZListIterator<T, B> End() const
        {
            return ZListIterator<T, B>(reinterpret_cast<ZListNode<T, B>*>(const_cast<ZList<T, A, B>*>(this)));
        }

        ZListIterator<T, B> End()
        {
            return ZListIterator<T, B>(reinterpret_cast<ZListNode<T, B>*>(this));
        }

        void AddFirst(ZListNode<T, B>* pNode)
        {
            this->m_Head.AddNext(pNode);
        }

        void AddLast(ZListNode<T, B>* pNode)
        {
            this->m_Head.AddPrev(pNode);
        }

        void DeleteAll()
        {
            while (ZListNodeBase<T>* pNode = this->GetFirst())
            {
                T* pObject = ZListNodeBase<T>::NodeToObject(pNode);
                delete pObject;
            }
        }
    };
}
