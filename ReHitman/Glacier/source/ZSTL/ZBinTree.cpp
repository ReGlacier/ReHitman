#include <Glacier/ZSTL/ZBinTree.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    ZBinTree::ZBinTree()
    {
        NIL = nullptr;
        m_pTopNode = NIL;
    }

    SBinTreeNode* ZBinTree::GetTopNode()
    {
        if (m_pTopNode == NIL)
        {
            return nullptr;
        }

        return m_pTopNode;
    }

    void ZBinTree::Insert(SBinTreeNode* z)
    {
        z->m_pLeft = NIL;
        z->m_pRight = NIL;
        z->m_pParent = NIL;

        auto* pFound = NIL;
        auto* pTop = m_pTopNode;

        while (pTop != NIL)
        {
            pFound = pTop;
            if (z->m_lKey >= pTop->m_lKey)
            {
                pTop = pTop->m_pRight;
            }
            else
            {
                pTop = pTop->m_pLeft;
            }
        }

        z->m_pParent = pFound;
        if (pFound == NIL)
        {
            m_pTopNode = z;
        }
        else if (z->m_lKey >= pFound->m_lKey)
        {
            pFound->m_pRight = z;
        }
        else
        {
            pFound->m_pLeft = z;
        }
    }

    SBinTreeNode* ZBinTree::Search(int k, SBinTreeNode* x)
    {
        SBinTreeNode* pTop = x;

        if (!x)
        {
            pTop = m_pTopNode;
        }

        while (pTop != NIL && k != pTop->m_lKey)
        {
            if (k > pTop->m_lKey)
            {
                pTop = pTop->m_pRight;
            }
            else
            {
                pTop = pTop->m_pLeft;
            }
        }

        if (pTop == NIL)
        {
            return nullptr;
        }

        return pTop;
    }

    SBinTreeNode* ZBinTree::SearchLargerOrSame(int k, SBinTreeNode* x)
    {
        if (!x)
        {
            x = m_pTopNode;
        }

        if (!x)
        {
            return nullptr;
        }

        if (k == x->m_lKey)
        {
            return (x == NIL) ? nullptr : x;
        }

        if (k > x->m_lKey)
        {
            if (x->m_pRight)
            {
                auto* pRightRes = SearchLargerOrSame(k, x->m_pRight);
                if (pRightRes && pRightRes->m_lKey >= k)
                {
                    return pRightRes;
                }
            }
            return nullptr;
        }

        if (x->m_pLeft)
        {
            auto* pLeftRes = SearchLargerOrSame(k, x->m_pLeft);
            if (pLeftRes && pLeftRes->m_lKey >= k)
            {
                return pLeftRes;
            }
        }

        if (x == NIL)
        {
            return nullptr;
        }

        return x;
    }

    SBinTreeNode* ZBinTree::Minimum(SBinTreeNode* x)
    {
        while (x->m_pLeft != NIL)
            x = x->m_pLeft;
        
        return x;
    }

    SBinTreeNode* ZBinTree::Maximum(SBinTreeNode* x)
    {
        while (x->m_pRight != NIL)
            x = x->m_pRight;

        return x;
    }

    SBinTreeNode* ZBinTree::Successor(SBinTreeNode* x)
    {
        auto* pCurrent = x;

        if (x->m_pRight == NIL)
        {
            auto* i = x->m_pParent;
            for ( ; i != NIL && pCurrent == i->m_pRight; i = i->m_pParent)
            {
                pCurrent = i;
            }

            return i;
        }
        else
        {
            return Minimum(x->m_pRight);
        }
    }

    int ZBinTree::Depth(SBinTreeNode* x)
    {
        if (!x)
        {
            x = m_pTopNode;
        }

        if (!x || x == NIL)
        {
            return 0;
        }

        int lDepth = 1;
        int rDepth = 1;

        if (x->m_pLeft != NIL)
        {
            lDepth = Depth(x->m_pLeft) + 1;
        }

        if (x->m_pRight != NIL)
        {
            rDepth = Depth(x->m_pRight) + 1;
        }

        return (lDepth < rDepth) ? rDepth : lDepth;
    }

    void ZBinTree::CheckLinks(SBinTreeNode* x)
    {
        SBinTreeNode* pCurrent = x;

        if (!pCurrent)
        {
            pCurrent = m_pTopNode;
        }

        
        ZASSERT(pCurrent != nullptr);
        if (pCurrent != NIL)
        {
            if (pCurrent->m_pLeft != NIL)
            {
                ZASSERT(pCurrent->m_pLeft->m_pParent == pCurrent);
                CheckLinks(pCurrent->m_pLeft);
            }

            if (pCurrent->m_pRight != NIL)
            {
                ZASSERT(pCurrent->m_pRight->m_pParent == pCurrent);
                CheckLinks(pCurrent->m_pRight);
            }
        }
    }

    SBinTreeNode* ZBinTree::Next(SBinTreeNode* pCurrent)
    {
        if (pCurrent->m_pLeft != NIL)
        {
            return pCurrent->m_pLeft;
        }

        if (pCurrent->m_pRight != NIL)
        {
            return pCurrent->m_pRight;
        }

        auto* pCurr = pCurrent;
        while (pCurrent->m_pParent != NIL)
        {
            if (pCurr->m_pParent->m_pRight != NIL && pCurr->m_pParent->m_pRight != pCurr)
            {
                return pCurr->m_pParent->m_pRight;
            }

            pCurr = pCurr->m_pParent;
        }

        return nullptr;
    }

    void ZBinTree::CopyData(SBinTreeNode*, SBinTreeNode*)
    {
        // Nothing
    }
}