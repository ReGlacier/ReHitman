#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/STATICREFTAB.h>
#include <Glacier/ZSTL/ZRBTree.h>


namespace Glacier
{
    struct SValTreeNode : public SBinTreeNode
    {
        int32_t m_lValue{0};
    };
    RE_VERIFY_SIZE(SValTreeNode, 0x18);

    struct ZValTree : public ZRBTree
    {
        // vtbl
        SBinTreeNode* Delete(SBinTreeNode* z) override;
        void CopyData(SBinTreeNode*, SBinTreeNode*) override;
        virtual void InsertKey(int lKey, int lValue);
        virtual void DeleteKey(int lKey);
        virtual int GetKeyVal(int lKey);

        // methods
        ZValTree(long lReftabSize);
        ~ZValTree();

        // members
        STATICREFTAB* m_pNodesList;
    };
    RE_VERIFY_SIZE(ZValTree, 0x24);
}
