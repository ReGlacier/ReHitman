#include <Glacier/PF4/ZData.h>
#include <Glacier/PF4/ZOpenNodeList.h>
#include <Glacier/ZUniAssert.h>

#include <cstring>


namespace Glacier::PF4
{
    namespace
    {
        constexpr float kInfinity = 100000.0f; // unreachable cost sentinel (PC pf4runtime)
    }

    // ZOpenNodeList heap helpers (PC 004DA520 ResetUsedNodeData, 004DA580 SetNodeData,
    // 004DA600 Swap, 004DA6B0 DecreaseKey, 004DA770 ExtractMin). Nodes that have never
    // been opened have ZNodeData::iOpenNode == -1; open nodes point into m_aList[0..m_iOpenNodeCount);
    // closed nodes keep pointing at their entry copied to the tail (m_aList[1000 - m_iClosedNodeCount]).

    float ZOpenNodeList::GetKey(int iNode) const
    {
        ZASSERT(m_pNodeData != nullptr);
        const int iOpen = m_pNodeData[iNode].iOpenNode;
        return (iOpen == -1) ? kInfinity : m_aList[iOpen].fKey;
    }

    float ZOpenNodeList::GetCost(int iNode) const
    {
        ZASSERT(m_pNodeData != nullptr);
        const int iOpen = m_pNodeData[iNode].iOpenNode;
        return (iOpen == -1) ? kInfinity : m_aList[iOpen].fCost;
    }

    int ZOpenNodeList::GetLinkIndex(int iNode) const
    {
        ZASSERT(m_pNodeData != nullptr);
        const int iOpen = m_pNodeData[iNode].iOpenNode;
        return (iOpen == -1) ? -1 : m_aList[iOpen].iLinkIndex;
    }

    bool ZOpenNodeList::GetVisited(int iNode) const
    {
        ZASSERT(m_pNodeData != nullptr);
        const int iOpen = m_pNodeData[iNode].iOpenNode;
        return iOpen != -1 && m_aList[iOpen].bVisited;
    }

    void ZOpenNodeList::SetVisited(int iNode, bool bVisited)
    {
        ZASSERT(m_pNodeData != nullptr);
        const int iOpen = m_pNodeData[iNode].iOpenNode;
        if (iOpen != -1)
        {
            m_aList[iOpen].bVisited = bVisited;
        }
    }

    void ZOpenNodeList::SetNodeData(ZNodeData* pNodeData, int iNodeCount)
    {
        m_iNodeCount = iNodeCount;
        m_pNodeData = pNodeData;
        std::memset(pNodeData, 0, 2 * iNodeCount);
        for (int i = 0; i < m_iNodeCount; ++i)
        {
            m_pNodeData[i].iOpenNode = 0xFFFF;
        }

        std::memset(m_aList, 0, sizeof(m_aList));
        for (int i = 0; i < 1000; ++i)
        {
            m_aList[i].iNode = 0xFFFF;
            m_aList[i].iParent = 0xFFFF;
        }

        m_iOpenNodeCount = 0;
        m_iClosedNodeCount = 0;
    }

    void ZOpenNodeList::ResetUsedNodeData()
    {
        for (int i = 0; i < m_iOpenNodeCount; ++i)
        {
            m_pNodeData[m_aList[i].iNode].iOpenNode = 0xFFFF;
        }
        for (int i = 0; i < m_iClosedNodeCount; ++i)
        {
            m_pNodeData[m_aList[999 - i].iNode].iOpenNode = 0xFFFF;
        }
        m_iOpenNodeCount = 0;
        m_iClosedNodeCount = 0;
    }

    void ZOpenNodeList::Swap(int iOpenNodeA, int iOpenNodeB)
    {
        ZOpenNode tmp = m_aList[iOpenNodeA];
        m_aList[iOpenNodeA] = m_aList[iOpenNodeB];
        m_aList[iOpenNodeB] = tmp;

        m_pNodeData[m_aList[iOpenNodeA].iNode].iOpenNode = iOpenNodeA;
        m_pNodeData[m_aList[iOpenNodeB].iNode].iOpenNode = iOpenNodeB;
    }

    int ZOpenNodeList::ExtractMin()
    {
        if (m_iOpenNodeCount == 0)
        {
            return -1;
        }

        ++m_iClosedNodeCount;
        const int iNode = m_aList[0].iNode;

        // Move the popped entry to the closed tail and remember its position.
        m_aList[1000 - m_iClosedNodeCount] = m_aList[0];
        m_pNodeData[iNode].iOpenNode = 1000 - m_iClosedNodeCount;

        const int iLast = m_iOpenNodeCount - 1;
        m_iOpenNodeCount = iLast;
        if (iLast != 0)
        {
            m_aList[0] = m_aList[iLast];
            m_pNodeData[m_aList[0].iNode].iOpenNode = 0;
        }

        // Sift the new root down.
        int iParent = 0;
        int iChild = 1;
        int iOther = 2;
        while (iChild < m_iOpenNodeCount)
        {
            int iBest = iChild;
            if (iOther < m_iOpenNodeCount && m_aList[iOther].fKey < m_aList[iChild].fKey)
            {
                iBest = iOther;
            }
            if (m_aList[iBest].fKey >= m_aList[iParent].fKey)
            {
                break;
            }
            Swap(iBest, iParent);
            iParent = iBest;
            iChild = 2 * iBest + 1;
            iOther = 2 * iBest + 2;
        }

        return iNode;
    }

    void ZOpenNodeList::DecreaseKey(
        int iParent, int iNode, int iLinkIndex, int iGate0, int iGate1, float fKey, float fCost)
    {
        ZASSERT(m_pNodeData != nullptr);
        ZASSERT(iNode >= 0 && iNode < m_iNodeCount);

        int iOpen = m_pNodeData[iNode].iOpenNode;
        if (iOpen == -1)
        {
            iOpen = m_iOpenNodeCount;
            m_aList[iOpen].iNode = static_cast<int16_t>(iNode);
            m_aList[iOpen].bVisited = false;
            m_pNodeData[iNode].iOpenNode = static_cast<int16_t>(iOpen);
            ++m_iOpenNodeCount;
        }
        else if (m_aList[iOpen].fKey <= fKey)
        {
            return;
        }

        m_aList[iOpen].fKey = fKey;
        m_aList[iOpen].fCost = fCost;
        m_aList[iOpen].iParent = static_cast<int16_t>(iParent);
        m_aList[iOpen].iLinkIndex = static_cast<int16_t>(iLinkIndex);
        m_aList[iOpen].iGate[0] = static_cast<int16_t>(iGate0);
        m_aList[iOpen].iGate[1] = static_cast<int16_t>(iGate1);

        // Sift the entry up while its parent has a larger key.
        while (iOpen > 0)
        {
            const int iParentOpen = (iOpen - 1) >> 1;
            if (m_aList[iParentOpen].fKey < m_aList[iOpen].fKey)
            {
                break;
            }
            Swap(iParentOpen, iOpen);
            iOpen = iParentOpen;
        }
    }
}
