#include <Glacier/ZSTL/ZRBTree.h>


namespace Glacier
{
    ZRBTree::ZRBTree()
        : ZBinTree()
    {
        m_NILT[0].m_bColor = 0;
        m_NILT[0].m_lKey = 0;
        m_NILT[0].m_pLeft = nullptr;
        m_NILT[0].m_pRight = nullptr;
        m_NILT[0].m_pParent = nullptr;
        NIL = m_NILT;
        m_pTopNode = NIL;
    }
    
    void ZRBTree::Insert(SBinTreeNode* z)
    {
        ZBinTree::Insert(z);

        z->m_bColor = RED;

        SBinTreeNode* x = z;
        while (x != m_pTopNode && x->m_pParent->m_bColor == RED)
        {
            if (x->m_pParent == x->m_pParent->m_pParent->m_pLeft)
            {
                SBinTreeNode* y = x->m_pParent->m_pParent->m_pRight;

                if (y->m_bColor == RED)
                {
                    x->m_pParent->m_bColor = BLACK;
                    y->m_bColor = BLACK;
                    x->m_pParent->m_pParent->m_bColor = RED;
                    x = x->m_pParent->m_pParent;
                }
                else
                {
                    if (x == x->m_pParent->m_pRight)
                    {
                        x = x->m_pParent;
                        LeftRotate(x);
                    }
                    
                    x->m_pParent->m_bColor = BLACK;
                    x->m_pParent->m_pParent->m_bColor = RED;
                    RightRotate(x->m_pParent->m_pParent);
                }
            }
            else
            {
                SBinTreeNode* y = x->m_pParent->m_pParent->m_pLeft;

                if (y->m_bColor == RED)
                {
                    x->m_pParent->m_bColor = BLACK;
                    y->m_bColor = BLACK;
                    x->m_pParent->m_pParent->m_bColor = RED;
                    x = x->m_pParent->m_pParent;
                }
                else
                {
                    if (x == x->m_pParent->m_pLeft)
                    {
                        x = x->m_pParent;
                        RightRotate(x);
                    }
                    
                    x->m_pParent->m_bColor = BLACK;
                    x->m_pParent->m_pParent->m_bColor = RED;
                    LeftRotate(x->m_pParent->m_pParent);
                }
            }
        }

        m_pTopNode->m_bColor = BLACK;
    }

    void ZRBTree::LeftRotate(SBinTreeNode* x)
    {
        SBinTreeNode* y = x->m_pRight;
        
        x->m_pRight = y->m_pLeft;
        if (y->m_pLeft != NIL)
        {
            y->m_pLeft->m_pParent = x;
        }

        y->m_pParent = x->m_pParent;
        if (x->m_pParent == NIL)
        {
            m_pTopNode = y;
        }
        else if (x == x->m_pParent->m_pLeft)
        {
            x->m_pParent->m_pLeft = y;
        }
        else
        {
            x->m_pParent->m_pRight = y;
        }

        y->m_pLeft = x;
        x->m_pParent = y;
    }

    void ZRBTree::RightRotate(SBinTreeNode* x)
    {
        SBinTreeNode* y = x->m_pLeft;

        x->m_pLeft = y->m_pRight;
        if (y->m_pRight != NIL)
        {
            y->m_pRight->m_pParent = x;
        }

        y->m_pParent = x->m_pParent;
        if (x->m_pParent == NIL)
        {
            m_pTopNode = y;
        }
        else if (x == x->m_pParent->m_pRight)
        {
            x->m_pParent->m_pRight = y;
        }
        else
        {
            x->m_pParent->m_pLeft = y;
        }

        y->m_pRight = x;
        x->m_pParent = y;
    }

    void ZRBTree::RBDeleteFixup(SBinTreeNode* x)
    {
        while (x != m_pTopNode && x->m_bColor == BLACK)
        {
            if (x == x->m_pParent->m_pLeft)
            {
                SBinTreeNode* w = x->m_pParent->m_pRight;

                if (w->m_bColor == RED)
                {
                    w->m_bColor = BLACK;
                    x->m_pParent->m_bColor = RED;
                    LeftRotate(x->m_pParent);
                    w = x->m_pParent->m_pRight;
                }

                if (w->m_pLeft->m_bColor == RED || w->m_pRight->m_bColor == RED)
                {
                    if (w->m_pRight->m_bColor == BLACK)
                    {
                        w->m_pLeft->m_bColor = BLACK;
                        w->m_bColor = RED;
                        RightRotate(w);
                        w = x->m_pParent->m_pRight;
                    }

                    w->m_bColor = x->m_pParent->m_bColor;
                    x->m_pParent->m_bColor = BLACK;
                    w->m_pRight->m_bColor = BLACK;
                    LeftRotate(x->m_pParent);
                    x = m_pTopNode;
                }
                else
                {
                    w->m_bColor = RED;
                    x = x->m_pParent;
                }
            }
            else
            {
                SBinTreeNode* w = x->m_pParent->m_pLeft;

                if (w->m_bColor == RED)
                {
                    w->m_bColor = BLACK;
                    x->m_pParent->m_bColor = RED;
                    RightRotate(x->m_pParent);
                    w = x->m_pParent->m_pLeft;
                }

                if (w->m_pRight->m_bColor == RED || w->m_pLeft->m_bColor == RED)
                {
                    if (w->m_pLeft->m_bColor == BLACK)
                    {
                        w->m_pRight->m_bColor = BLACK;
                        w->m_bColor = RED;
                        LeftRotate(w);
                        w = x->m_pParent->m_pLeft;
                    }

                    w->m_bColor = x->m_pParent->m_bColor;
                    x->m_pParent->m_bColor = BLACK;
                    w->m_pLeft->m_bColor = BLACK;
                    RightRotate(x->m_pParent);
                    x = m_pTopNode;
                }
                else
                {
                    w->m_bColor = RED;
                    x = x->m_pParent;
                }
            }
        }

        x->m_bColor = BLACK;
    }

    SBinTreeNode* ZRBTree::Delete(SBinTreeNode* z)
    {
        SBinTreeNode* y;
        SBinTreeNode* x;

        if (z->m_pLeft == NIL || z->m_pRight == NIL)
        {
            y = z;
        }
        else
        {
            y = Successor(z);
        }

        if (y->m_pLeft != NIL)
        {
            x = y->m_pLeft;
        }
        else
        {
            x = y->m_pRight;
        }

        x->m_pParent = y->m_pParent;

        if (y->m_pParent == NIL)
        {
            m_pTopNode = x;
        }
        else if (y == y->m_pParent->m_pLeft)
        {
            y->m_pParent->m_pLeft = x;
        }
        else
        {
            y->m_pParent->m_pRight = x;
        }

        if (y != z)
        {
            z->m_lKey = y->m_lKey;
            CopyData(z, y);
        }

        if (y->m_bColor == BLACK)
        {
            RBDeleteFixup(x);
        }

        if (y == NIL)
        {
            return nullptr;
        }

        return y;
    }
}