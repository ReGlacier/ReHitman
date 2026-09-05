#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    struct SBinTreeNode
    {
        struct SBinTreeNode* m_pParent { nullptr };
        struct SBinTreeNode* m_pLeft { nullptr };
        struct SBinTreeNode* m_pRight { nullptr };
        int32_t m_lKey{0};
        uint8_t m_bColor{0u}; // in original it was bool
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(SBinTreeNode, 0x14);

    struct ZBinTree
    {
        // vtbl
        virtual void Insert(SBinTreeNode* z);
        virtual SBinTreeNode* Delete(SBinTreeNode* x) = 0;
        virtual SBinTreeNode* Search(int k, SBinTreeNode* x);
        virtual SBinTreeNode* SearchLargerOrSame(int k, SBinTreeNode* x);
        virtual SBinTreeNode* Minimum(SBinTreeNode* x);
        virtual SBinTreeNode* Maximum(SBinTreeNode* x);
        virtual SBinTreeNode* Successor(SBinTreeNode* x);
        virtual int Depth(SBinTreeNode* x);
        virtual void CheckLinks(SBinTreeNode* x);
        virtual SBinTreeNode* Next(SBinTreeNode* pCurrent);
        virtual void CopyData(SBinTreeNode*, SBinTreeNode*);

        // methods
        ZBinTree();

        SBinTreeNode* GetTopNode();

        // members
        SBinTreeNode* m_pTopNode { nullptr };
        SBinTreeNode* NIL { nullptr };
    };
    RE_VERIFY_SIZE(ZBinTree, 0xC);
}