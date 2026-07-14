#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZUniAssert.h>
#include <cstddef>
#include <cstdint>

namespace Glacier
{
    /**
     * @brief Intrusive doubly-linked list node used by Glacier list containers.
     *
     * A node is linked when both m_Next and m_Prev are non-null. Standalone nodes keep
     * both pointers null. List heads are sentinel nodes whose next/prev pointers point
     * back to the head when the list is empty.
     */
    struct ZListNodeBase
    {
        ZListNodeBase* m_Next;
        ZListNodeBase* m_Prev;

        ZListNodeBase() : m_Next(nullptr), m_Prev(nullptr) {}
        ZListNodeBase(ZListNodeBase* pNext, ZListNodeBase* pPrev) : m_Next(pNext), m_Prev(pPrev) {}
        ~ZListNodeBase() { Unlink(); }

        bool IsLinked() const { return m_Next != nullptr; }

        void Unlink()
        {
            if (m_Next && m_Prev)
            {
                m_Next->m_Prev = m_Prev;
                m_Prev->m_Next = m_Next;
                m_Next = nullptr;
                m_Prev = nullptr;
            }
        }

        void AddPrev(ZListNodeBase* pNode)
        {
            ZASSERT(!pNode->IsLinked());

            pNode->m_Next = this;
            pNode->m_Prev = m_Prev;
            m_Prev->m_Next = pNode;
            m_Prev = pNode;
        }

        void AddNext(ZListNodeBase* pNode)
        {
            ZASSERT(!pNode->IsLinked());

            pNode->m_Next = m_Next;
            pNode->m_Prev = this;
            m_Next->m_Prev = pNode;
            m_Next = pNode;
        }
    };
    RE_VERIFY_SIZE(ZListNodeBase, 0x8);

    template <typename T, size_t N>
    struct ZListNode : ZListNodeBase
    {
    };

    template <typename T, size_t U0>
    struct ZListIterator
    {
        ZListNode<T, U0>* m_Node;
    };

    /**
     * @brief Intrusive circular doubly-linked list with a sentinel head node.
     *
     * The list does not own its nodes. Destroying or clearing the list only unlinks
     * nodes from this list; it does not destroy the objects that contain them.
     */
    struct ZListBase
    {
        ZListNodeBase m_Head;

        ZListBase() : m_Head(reinterpret_cast<ZListNodeBase*>(this), reinterpret_cast<ZListNodeBase*>(this)) {}
        ~ZListBase() { UnlinkAll(); }

        void UnlinkAll()
        {
            while (m_Head.m_Next != reinterpret_cast<ZListNodeBase*>(this))
                m_Head.m_Next->Unlink();
        }

        int Count() const
        {
            int result = 0;
            for (auto* pNode = m_Head.m_Next; pNode != reinterpret_cast<const ZListNodeBase*>(this); pNode = pNode->m_Next)
                ++result;

            return result;
        }

        ZListNodeBase* GetFirst()
        {
            return m_Head.m_Next != reinterpret_cast<ZListNodeBase*>(this) ? m_Head.m_Next : nullptr;
        }

        const ZListNodeBase* GetFirst() const
        {
            return m_Head.m_Next != reinterpret_cast<const ZListNodeBase*>(this) ? m_Head.m_Next : nullptr;
        }

        ZListNodeBase* GetLast()
        {
            return m_Head.m_Prev != reinterpret_cast<ZListNodeBase*>(this) ? m_Head.m_Prev : nullptr;
        }

        const ZListNodeBase* GetLast() const
        {
            return m_Head.m_Prev != reinterpret_cast<const ZListNodeBase*>(this) ? m_Head.m_Prev : nullptr;
        }
    };
    RE_VERIFY_SIZE(ZListBase, 0x8);

    template <typename T, bool A, int B>
    struct ZList : ZListBase
    {
    };
}
