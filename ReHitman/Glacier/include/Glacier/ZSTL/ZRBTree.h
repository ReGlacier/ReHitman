#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZBinTree.h>


namespace Glacier
{
    struct ZRBTree : public ZBinTree
    {
        // types
        enum NodeColor : uint8_t 
        {
            BLACK = 0,
            RED = 1
        };

        // vtbl
        void Insert(SBinTreeNode* z) override;
        SBinTreeNode* Delete(SBinTreeNode* x) override;
        
        // methods
        ZRBTree();
        void RBDeleteFixup(SBinTreeNode* pNode);
        void LeftRotate(SBinTreeNode* x);
        void RightRotate(SBinTreeNode* x);

        // members
        SBinTreeNode m_NILT[1]{};
    };
    RE_VERIFY_SIZE(ZRBTree, 0x20);
}