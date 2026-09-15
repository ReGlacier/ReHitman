#include <Glacier/ZSTL/ZValTree.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    namespace
    {
        uint32_t* RefPtrFromNode(SBinTreeNode* pNode)
        {
            return reinterpret_cast<uint32_t*>(pNode) + 1;
        }
    }

    SBinTreeNode* ZValTree::Delete(SBinTreeNode* z)
    {
        SBinTreeNode* pDeletedNode = ZRBTree::Delete(z);
        m_pNodesList->DelRefPtr(RefPtrFromNode(pDeletedNode));
        return nullptr;
    }

    void ZValTree::CopyData(SBinTreeNode* x, SBinTreeNode* y)
    {
        static_cast<SValTreeNode*>(x)->m_lValue = static_cast<SValTreeNode*>(y)->m_lValue;
    }

    void ZValTree::InsertKey(int lKey, int lVal)
    {
        auto* pNodeData = m_pNodesList->Add(0);
        auto* pNode = reinterpret_cast<SValTreeNode*>(pNodeData - 1);

        pNode->m_lKey = lKey;
        pNode->m_lValue = lVal;

        Insert(pNode);
    }

    void ZValTree::DeleteKey(int lKey)
    {
        if (auto* pFound = Search(lKey, nullptr))
        {
            SBinTreeNode* pDeletedNode = ZRBTree::Delete(pFound);
            m_pNodesList->DelRefPtr(RefPtrFromNode(pDeletedNode));
        }
    }

    int ZValTree::GetKeyVal(int lKey)
    {
        if (auto* pFound = Search(lKey, nullptr))
        {
            return static_cast<SValTreeNode*>(pFound)->m_lValue;
        }
        
        return 0;
    }

    ZValTree::ZValTree(long lReftabSize)
    {
        m_pNodesList = ZUniMemory::New<STATICREFTAB>(static_cast<uint32_t>(lReftabSize), 5);
    }

    ZValTree::~ZValTree()
    {
        while (SBinTreeNode* pTopNode = GetTopNode())
        {
            Delete(pTopNode);
        }

        if (m_pNodesList)
        {
            ZUniMemory::Delete(m_pNodesList);
            m_pNodesList = nullptr;
        }
    }
}
