#include <Glacier/ZSTL/CListUser.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>
#include <algorithm>


namespace Glacier
{
    struct SGenBufferHeader
    {
        uint32_t uSizeOfStaticBuffer;
        uint32_t uSizeOfDynamicBuffer;
        uint32_t uSizeOfRuntimeBuffer;
    };

    namespace
    {
        constexpr uint32_t LIST_END = 1u;
        constexpr uint32_t LIST_RANGE = 2u;
        constexpr uint32_t LIST_ATTRIBS = 7u;
        constexpr uint32_t LIST_VALUE_MASK = 0xFFFFFFF8u;
        constexpr uint32_t LIST_ID_MASK = 0x00FFFFFFu;
        constexpr uint32_t LIST_REF_BIAS = 0x60u;
        constexpr uint32_t LIST_REF_FLAG = 0x40000000u;
        constexpr uint32_t ZCUPDATELIGHT = 0x01000000u;

        uint32_t ListValue(uint32_t entry)
        {
            return entry & LIST_VALUE_MASK;
        }

        uint32_t ListAttribs(uint32_t entry)
        {
            return entry & LIST_ATTRIBS;
        }

        bool IsListEnd(uint32_t entry)
        {
            return (entry & LIST_END) != 0;
        }

        bool IsListRange(uint32_t entry)
        {
            return (entry & LIST_RANGE) != 0;
        }

        uint32_t PackListValue(uintptr_t value, uint32_t attribs)
        {
            return static_cast<uint32_t>(value) | (attribs & LIST_ATTRIBS);
        }

        uint32_t& NodeListWord(void* pNode)
        {
            return *reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(pNode) + 0x54);
        }

        uint32_t NodeListWord(const void* pNode)
        {
            return *reinterpret_cast<const uint32_t*>(static_cast<const uint8_t*>(pNode) + 0x54);
        }

        uint32_t NodeListId(const void* pNode)
        {
            return NodeListWord(pNode) & LIST_ID_MASK;
        }

        uint8_t NodeListWordHighByte(const void* pNode)
        {
            return *(static_cast<const uint8_t*>(pNode) + 0x57);
        }

        void ResetRuntimeListWord(void* pNode)
        {
            NodeListWord(pNode) = NodeListWordHighByte(pNode);
        }

        void MarkNodeListDirty(void* pNode)
        {
            static_cast<ZBaseGeom*>(pNode)->m_lControl |= ZCUPDATELIGHT;
        }

        uint32_t* DynamicBlockNext(uint32_t* pBlock)
        {
            return reinterpret_cast<uint32_t*>(pBlock[3]);
        }

        void SetDynamicBlockNext(uint32_t* pBlock, uint32_t* pNext)
        {
            pBlock[3] = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(pNext));
        }

        void ClearDynamicBlock(uint32_t* pBlock)
        {
            pBlock[0] = 0;
            pBlock[1] = 0;
            pBlock[2] = 0;
            pBlock[3] = 0;
        }
    }

    void CListUser::AnalyzeCatch(uint32_t uLengthCol, void* pCallerNode) 
    {
        ZASSERT(uLengthCol <= m_uMaxLength);

        if (uLengthCol >= 2)
            std::sort(m_pCollidedWith, m_pCollidedWith + uLengthCol);

        for (uint32_t i = 0; i < uLengthCol; ++i)
        {
            ZASSERT(NodeListId(pCallerNode) != (reinterpret_cast<ZBaseGeom*>(m_pCollidedWith[i])->ListId() & LIST_ID_MASK));
            if (i + 1 < uLengthCol)
                ZASSERT(m_pCollidedWith[i] != m_pCollidedWith[i + 1]);
        }

        const uint32_t uListID = NodeListId(pCallerNode);
        ZASSERT(uListID != 0);

        const uint32_t uCounts = PriUnfoldList(m_pInternalListUnfold, uListID);
        const uint32_t uDynamicCount = uCounts >> 16;
        const uint32_t uStaticCount = uCounts & 0xFFFFu;

        if (uDynamicCount >= 2)
            std::sort(m_pInternalListUnfold + uStaticCount, m_pInternalListUnfold + uStaticCount + uDynamicCount);

        std::merge(
            m_pInternalListUnfold,
            m_pInternalListUnfold + uStaticCount,
            m_pInternalListUnfold + uStaticCount,
            m_pInternalListUnfold + uStaticCount + uDynamicCount,
            m_pInternalOutBuf);

        const uint32_t uTotalCount = uStaticCount + uDynamicCount;
        uint32_t* pCurrentMember = m_pInternalOutBuf;
        uint32_t* pCurrentCollision = m_pCollidedWith;
        uint32_t* pCollisionEnd = m_pCollidedWith + uLengthCol;

        while (pCurrentMember < m_pInternalOutBuf + uTotalCount)
        {
            auto* pMember = reinterpret_cast<ZBaseGeom*>(*pCurrentMember);
            pMember->m_lControl |= 0x1000000u;

            if (pCurrentCollision != pCollisionEnd && *pCurrentMember >= *pCurrentCollision)
            {
                if (*pCurrentMember == *pCurrentCollision)
                {
                    *pCurrentCollision = 0;
                    ++pCurrentMember;
                }

                ++pCurrentCollision;
            }
            else
            {
                const bool bDisconnectedFromMember = DisconnectFromList(pMember->ListId(), pCallerNode);
                const bool bDisconnectedFromCaller = DisconnectFromList(uListID, pMember);
                ZASSERT(bDisconnectedFromMember && bDisconnectedFromCaller);
                ++pCurrentMember;
            }
        }

        uint32_t uNumNodesLeft = 0;
        for (uint32_t i = 0; i < uLengthCol; ++i)
        {
            if (m_pCollidedWith[i])
            {
                if (i != uNumNodesLeft)
                    m_pCollidedWith[uNumNodesLeft] = m_pCollidedWith[i];

                ++uNumNodesLeft;
            }
        }

        if (uNumNodesLeft)
        {
            const uint32_t uLenSuccess = ConnectBatchToList(uListID, m_pCollidedWith, uNumNodesLeft);
            ZASSERT(uLenSuccess == uNumNodesLeft);
        }

        for (uint32_t i = 0; i < uNumNodesLeft; ++i)
        {
            auto* pMember = reinterpret_cast<ZBaseGeom*>(m_pCollidedWith[i]);
            const bool bConnected = ConnectToList(pMember->ListId(), pCallerNode);
            pMember->m_lControl |= 0x1000000u;
            ZASSERT(bConnected == true);
        }

        MarkNodeListDirty(pCallerNode);
    }

    bool CListUser::IsNodeInList(uint32_t uListID, void* pNode) 
    {
        ZASSERT(uListID != 0);
        return IsNodeInDynamicList(uListID, pNode) || IsNodeInStaticList(uListID, pNode);
    }

    uint32_t* CListUser::UnfoldList(uint32_t* pListLength, uint32_t uListID) 
    {
        if (!uListID)
            return nullptr;

        const uint32_t uCounts = PriUnfoldList(m_pInternalListUnfold, uListID);
        *pListLength = (uCounts >> 16) + (uCounts & 0xFFFFu);
        return m_pInternalListUnfold;
    }

    uint32_t* CListUser::GetCatchBuffer(uint32_t* pMaxInput)
    {
        *pMaxInput = m_uMaxLength;
        return m_pCollidedWith;
    }

    void CListUser::DisconnectFromAllMembers(void* pCallerNode) 
    {
        const uint32_t uListID = NodeListId(pCallerNode);
        ZASSERT(uListID != 0);

        const uint32_t uCounts = PriUnfoldList(m_pInternalListUnfold, uListID);
        const uint32_t uStaticCount = uCounts & 0xFFFFu;
        const uint32_t uDynamicCount = uCounts >> 16;
        const uint32_t uTotalCount = uStaticCount + uDynamicCount;

        for (uint32_t i = 0; i < uTotalCount; ++i)
        {
            auto* pMember = reinterpret_cast<ZBaseGeom*>(m_pInternalListUnfold[i]);
            MarkNodeListDirty(pMember);
            DisconnectFromList(pMember->ListId(), pCallerNode);
        }

        auto* pList = &m_pStaticBuf[uListID];
        pList[1] = 1;

        auto* pDynamicBlock = reinterpret_cast<uint32_t*>(pList[0]);
        while (pDynamicBlock)
        {
            auto* pNextBlock = DynamicBlockNext(pDynamicBlock);
            DeAlloc(pDynamicBlock);
            pDynamicBlock = pNextBlock;
        }

        pList[0] = 0;
        MarkNodeListDirty(pCallerNode);
    }

    void CListUser::NotifyAllMembers(void* pCallerNode) 
    {
        const uint32_t uListID = NodeListId(pCallerNode);
        ZASSERT(uListID != 0);

        const uint32_t uCounts = PriUnfoldList(m_pInternalListUnfold, uListID);
        const uint32_t uStaticCount = uCounts & 0xFFFFu;
        const uint32_t uDynamicCount = uCounts >> 16;
        const uint32_t uTotalCount = uStaticCount + uDynamicCount;

        for (uint32_t i = 0; i < uTotalCount; ++i)
        {
            auto* pMember = reinterpret_cast<ZBaseGeom*>(m_pInternalListUnfold[i]);
            MarkNodeListDirty(pMember);
        }

        MarkNodeListDirty(pCallerNode);
    }

    bool CListUser::DisconnectNodeFromNode(ZBaseGeom* pNode1, ZBaseGeom* pNode2) 
    {
        bool bDisconnectedFromNode1 = false;
        if (pNode1->ListId() != 0)
            bDisconnectedFromNode1 = DisconnectFromList(pNode1->ListId(), pNode2) != 0;

        bool bDisconnectedFromNode2 = false;
        if (pNode2->ListId() != 0)
            bDisconnectedFromNode2 = DisconnectFromList(pNode2->ListId(), pNode1) != 0;

        ZASSERT(bDisconnectedFromNode1 == bDisconnectedFromNode2);
        return bDisconnectedFromNode1;
    }

    void CListUser::ConvertOffsetsToRefs(const uint32_t* pRemapTable) 
    {
        auto** pBuf = reinterpret_cast<uint32_t**>(m_pStaticBuf + 4);
        auto* pEnd = m_pStaticBuf + m_uSizeOfStaticBuf;

        while (reinterpret_cast<uint32_t*>(pBuf) < pEnd)
        {
            ConvertInDynamicBuf(*pBuf++, pRemapTable);

            bool bEnd = false;
            while (!bEnd)
            {
                uint32_t uMember = reinterpret_cast<uint32_t>(*pBuf);
                bEnd = (uMember & 1u) != 0;
                FixMember(reinterpret_cast<uint32_t*>(pBuf++), uMember, pRemapTable);
            }
        }

        ZASSERT(reinterpret_cast<uint32_t*>(pBuf) == pEnd);
    }

    uint32_t CListUser::GetTotalBufferSize() const 
    {
        return m_uSizeOfStaticBuf + m_uSizeOfDynamicBuf + m_uSizeOfRuntimeBuf;
    }

    uint32_t* CListUser::GetFullBuffer() const 
    {
        return m_pStaticBuf;
    }

    uint32_t CListUser::AddRuntimeMember(void* pNode) 
    {
        ZASSERT((reinterpret_cast<uintptr_t>(pNode) & 7u) == 0);

        void* pRuntimeEntry = m_pGetDynEntryPool->Alloc();
        ZASSERT(pRuntimeEntry != nullptr);

        auto* pRuntimeWords = static_cast<uint32_t*>(pRuntimeEntry);
        pRuntimeWords[0] = 0;
        pRuntimeWords[1] = 1;

        const auto uListID = static_cast<uint32_t>((static_cast<uint8_t*>(pRuntimeEntry) - reinterpret_cast<uint8_t*>(m_pStaticBuf)) >> 2);
        ZASSERT(uListID < (1u << 24));

        return uListID;
    }

    void CListUser::RemoveRuntimeMember(void* pNode) 
    {
        DisconnectFromAllMembers(pNode);
        m_pGetDynEntryPool->DeAlloc(reinterpret_cast<uint8_t*>(m_pStaticBuf) + ((NodeListWord(pNode) >> 6) & 0x3FFFFFCu));
        ResetRuntimeListWord(pNode);
    }

    // methods
    CListUser::CListUser(void* pGenBuf)
        : CMemPool()
    {
        if (!pGenBuf)
        {
            m_pCollidedWith = nullptr;
            return;
        }

        auto* pGenBufHeader = reinterpret_cast<SGenBufferHeader*>(pGenBuf);

        m_uSizeOfStaticBuf = pGenBufHeader->uSizeOfStaticBuffer;
        m_uSizeOfDynamicBuf = pGenBufHeader->uSizeOfDynamicBuffer;
        m_uSizeOfRuntimeBuf = pGenBufHeader->uSizeOfRuntimeBuffer;
        m_pStaticBuf = reinterpret_cast<uint32_t*>(pGenBuf);
        ZASSERT(m_pStaticBuf != nullptr);

        m_pGetDynEntryPool = nullptr;
        m_pGetDynEntryPool = ZUniMemory::New<CMemPool>();
        ZASSERT(m_pGetDynEntryPool != nullptr);

        Init(sizeof(SDynBlock), (m_uSizeOfDynamicBuf >> 2) & 0xFFFFFFFu, &m_pStaticBuf[m_uSizeOfRuntimeBuf + m_uSizeOfStaticBuf], 1);
        m_pGetDynEntryPool->Init(8, (m_uSizeOfRuntimeBuf >> 1) & 0x1FFFFFFFu, &m_pStaticBuf[m_uSizeOfStaticBuf], 1);

        m_pCollidedWith = nullptr;
        m_uMaxLength = 1024;
        m_pDynamicBuf = static_cast<SDynBlock*>(GetPtrFromBlockNum(0));

        m_pCollidedWith = reinterpret_cast<uint32_t*>(ZUniMemory::Allocate(sizeof(uint32_t) * 0xC00));
        m_pInternalListUnfold = &m_pCollidedWith[m_uMaxLength];
        m_pInternalOutBuf = &m_pInternalListUnfold[m_uMaxLength];

    }
    
    bool CListUser::IsRuntimeMember(uint32_t iMember) const
    {
        return (iMember * sizeof(uint32_t)) >= m_uSizeOfStaticBuf;
    }
    
    void CListUser::FixMember(uint32_t* pMember, uint32_t uMember, const uint32_t* pRemapTable)
    {
        (void)pRemapTable;

        const uint32_t uAttribs = ListAttribs(uMember);
        if (ListValue(uMember) == 0)
            return;

        const uint32_t uRef = (ListValue(uMember) + LIST_REF_BIAS) | LIST_REF_FLAG;
        const uintptr_t uPtr = reinterpret_cast<uintptr_t>(ZBaseGeom::RefToPtr(uRef));
        ZASSERT((uPtr & 7u) == 0);

        *pMember = PackListValue(uPtr, uAttribs);
    }
    
    void CListUser::ConvertInDynamicBuf(uint32_t* uDynAddr, const uint32_t* pRemapTable)
    {
        // TODO: Think about rewrite to SDynBlock usage
        // TODO: Also, check this code twice

        while (uDynAddr != nullptr)
        {
            FixMember(&uDynAddr[0], uDynAddr[0], pRemapTable);
            FixMember(&uDynAddr[1], uDynAddr[1], pRemapTable);
            FixMember(&uDynAddr[2], uDynAddr[2], pRemapTable);

            uDynAddr = reinterpret_cast<uint32_t*>(uDynAddr[3]);
        }
    }
    
    void CListUser::ClearHoleInStaticList(uint32_t* pData)
    {
        if (!pData)
            return;

        do
        {
            pData[0] = pData[1];
            ++pData;
        } 
        while (!IsListEnd(pData[0]));
    }
    
    uint32_t CListUser::TrackDownInStaticList(uint32_t uListID, void* pNode, bool bPermitDelete)
    {
        ZASSERT(pNode != nullptr);

        uint32_t uResult = 1;
        uint32_t* pEntry = &m_pStaticBuf[uListID + 1];
        bool bFound = false;
        bool bEndOfList = false;
        bool bIsFromTo = false;

        do
        {
            bEndOfList = IsListEnd(*pEntry);
            bIsFromTo = IsListRange(*pEntry);
            const uintptr_t uMember = ListValue(*pEntry);
            const uintptr_t uNode = reinterpret_cast<uintptr_t>(pNode);

            ZASSERT(!(bIsFromTo && bEndOfList));

            if (bIsFromTo)
            {
                const uintptr_t uNextMember = ListValue(pEntry[1]);
                if (uNode < uMember || uNextMember < uNode)
                {
                    ZASSERT(!bEndOfList);
                    bEndOfList = IsListEnd(pEntry[1]);
                    pEntry += 2;
                }
                else
                {
                    bFound = true;
                }
            }
            else if (uMember == uNode)
            {
                bFound = true;
            }
            else
            {
                ++pEntry;
            }
        }
        while (!bEndOfList && !bFound);

        if (bFound && bPermitDelete)
            return QuickStaticDelete(uListID, pEntry, pNode, bIsFromTo);

        if (bFound)
            return 0;

        return uResult;
    }
    
    uint32_t CListUser::TrackDownInDynamicList(uint32_t uListID, void* pNode, bool bPermitDelete)
    {
        ZASSERT(pNode != nullptr);

        auto** pListHead = reinterpret_cast<uint32_t**>(&m_pStaticBuf[uListID]);
        auto* pPrevBlock = static_cast<uint32_t*>(nullptr);
        auto* pBlock = *pListHead;
        auto* pFoundBlock = static_cast<uint32_t*>(nullptr);
        int iFoundSlot = -1;
        const uint32_t uNode = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(pNode));

        while (pBlock && !pFoundBlock)
        {
            for (int i = 0; i < 3; ++i)
            {
                if (pBlock[i] == uNode)
                {
                    pFoundBlock = pBlock;
                    iFoundSlot = i;
                    break;
                }
            }

            if (!pFoundBlock)
            {
                pPrevBlock = pBlock;
            pBlock = DynamicBlockNext(pBlock);
            }
        }

        if (pFoundBlock && bPermitDelete)
        {
            auto* pWriteBlock = pFoundBlock;
            int iWriteSlot = iFoundSlot;
            auto* pReadBlock = pFoundBlock;
            int iReadSlot = iFoundSlot + 1;

            while (pReadBlock)
            {
                while (iReadSlot < 3)
                {
                    pWriteBlock[iWriteSlot] = pReadBlock[iReadSlot];
                    pWriteBlock = pReadBlock;
                    iWriteSlot = iReadSlot;
                    ++iReadSlot;
                }

                pReadBlock = DynamicBlockNext(pReadBlock);
                iReadSlot = 0;
            }

            pWriteBlock[iWriteSlot] = 0;

            auto* pLastPrev = static_cast<uint32_t*>(nullptr);
            auto* pLast = *pListHead;
            while (pLast && DynamicBlockNext(pLast))
            {
                pLastPrev = pLast;
                pLast = DynamicBlockNext(pLast);
            }

            if (pLast && pLast[0] == 0 && pLast[1] == 0 && pLast[2] == 0)
            {
                if (pLastPrev)
                    SetDynamicBlockNext(pLastPrev, nullptr);
                else
                    *pListHead = nullptr;

                DeAlloc(pLast);
            }

            return 0;
        }

        if (pFoundBlock)
            return 0;

        return 1;
    }
    
    uint32_t CListUser::ConnectBatchToList(uint32_t uListID, uint32_t* pNodes, int iLength)
    {
        ZASSERT(iLength > 0);
        ZASSERT(uListID != 0);

        for (int i = 0; i < iLength; ++i)
            ZASSERT(IsNodeInList(uListID, reinterpret_cast<void*>(pNodes[i])) == false);

        auto** pListHead = reinterpret_cast<uint32_t**>(&m_pStaticBuf[uListID]);
        auto* pDynamicEntry = *pListHead;

        if (pDynamicEntry)
        {
            while (DynamicBlockNext(pDynamicEntry))
                pDynamicEntry = DynamicBlockNext(pDynamicEntry);

            if (pDynamicEntry[2])
            {
                auto* pNextEntry = static_cast<uint32_t*>(Alloc());
                ZASSERT(pNextEntry != nullptr);
                SetDynamicBlockNext(pDynamicEntry, pNextEntry);
                pDynamicEntry = pNextEntry;
                ClearDynamicBlock(pDynamicEntry);
            }

            ZASSERT(DynamicBlockNext(pDynamicEntry) == nullptr);
        }
        else
        {
            pDynamicEntry = static_cast<uint32_t*>(Alloc());
            ZASSERT(pDynamicEntry != nullptr);
            *pListHead = pDynamicEntry;
            ClearDynamicBlock(pDynamicEntry);
        }

        uint32_t uWriteSlot = 2;
        if (!pDynamicEntry[1])
        {
            --uWriteSlot;
            if (!pDynamicEntry[0])
                --uWriteSlot;
        }

        ZASSERT(pDynamicEntry[2] == 0);

        int iNumToGo = iLength;
        while (iNumToGo)
        {
            auto* pCurrentEntry = pDynamicEntry;
            for (uint32_t i = uWriteSlot; i < 3; ++i)
            {
                pCurrentEntry[i] = pNodes[--iNumToGo];
                ZASSERT(pNodes[iNumToGo] != 0);
                ZASSERT((pNodes[iNumToGo] & 7u) == 0);

                if (!iNumToGo)
                    break;
            }

            if (iNumToGo)
            {
                auto* pNextEntry = static_cast<uint32_t*>(Alloc());
                SetDynamicBlockNext(pDynamicEntry, pNextEntry);
                pDynamicEntry = pNextEntry;
                ZASSERT(pDynamicEntry != nullptr);
                ClearDynamicBlock(pDynamicEntry);
                uWriteSlot = 0;
            }
        }

        return iLength;
    }
    
    uint32_t CListUser::QuickStaticDelete(uint32_t uListID, uint32_t* pListPos, void* pNode, bool bIsFromTo)
    {
        uint32_t uResult = 0;

        if (bIsFromTo)
        {
            const uintptr_t uFrom = ListValue(*pListPos);
            const uintptr_t uTo = ListValue(pListPos[1]);
            const uintptr_t uNode = reinterpret_cast<uintptr_t>(pNode);

            if (uNode - uFrom < uTo - uNode)
            {
                uintptr_t uConnectFrom = ListValue(*pListPos);
                intptr_t iConnectTo = static_cast<intptr_t>(uNode - sizeof(ZBaseGeom));

                if (uTo - uNode - sizeof(ZBaseGeom) < sizeof(ZBaseGeom) * 2)
                {
                    *pListPos &= ~LIST_RANGE;
                    uResult = 2;
                }

                ZASSERT(uTo - uNode != sizeof(ZBaseGeom));
                *pListPos = PackListValue(uNode + sizeof(ZBaseGeom), ListAttribs(*pListPos));

                for (uintptr_t i = uConnectFrom; static_cast<uintptr_t>(iConnectTo) >= i; i += sizeof(ZBaseGeom))
                    ConnectToList(uListID, reinterpret_cast<void*>(i));
            }
            else
            {
                uintptr_t uConnectFrom = uNode + sizeof(ZBaseGeom);
                intptr_t iConnectTo = static_cast<intptr_t>(ListValue(pListPos[1]));

                if (uNode - uFrom - sizeof(ZBaseGeom) < sizeof(ZBaseGeom) * 2)
                {
                    *pListPos &= ~LIST_RANGE;
                    uResult = 2;
                }

                if (uNode - uFrom == sizeof(ZBaseGeom))
                    iConnectTo = 0;
                else
                    pListPos[1] = PackListValue(uNode - sizeof(ZBaseGeom), ListAttribs(pListPos[1]));

                for (uintptr_t i = uConnectFrom; static_cast<uintptr_t>(iConnectTo) >= i; i += sizeof(ZBaseGeom))
                    ConnectToList(uListID, reinterpret_cast<void*>(i));
            }
        }
        else if (IsListEnd(*pListPos))
        {
            if (&m_pStaticBuf[uListID + 1] >= pListPos)
                *pListPos = 1;
            else
                pListPos[-1] |= LIST_END;
        }
        else
        {
            ClearHoleInStaticList(pListPos);
            return 4;
        }

        return uResult;
    }
    
    uint32_t CListUser::DisconnectFromStaticList(uint32_t uListID, void* pNodes)
    {
        return TrackDownInStaticList(uListID, pNodes, true) != 1;
    }
    
    uint32_t CListUser::DisconnectFromDynamicList(uint32_t uListID, void* pNodes)
    {
        return TrackDownInDynamicList(uListID, pNodes, true) != 1;
    }
    
    uint32_t CListUser::PriUnfoldList(uint32_t* pGetMembers, uint32_t uListID)
    {
        ZASSERT(uListID != 0);

        uint32_t uTotalCount = 0;
        uint32_t uWriteIndex = 0;

        uint32_t* pStaticEntry = &m_pStaticBuf[uListID];
        auto* pDynamicBlock = reinterpret_cast<uint32_t*>(*pStaticEntry);

        uint32_t uEntry;
        do
        {
            uEntry = *++pStaticEntry;
            ++uTotalCount;

            uint32_t uMember = ListValue(uEntry);
            pGetMembers[uWriteIndex] = uMember;

            if (IsListRange(uEntry))
            {
                const uint32_t uNextMember = ListValue(pStaticEntry[1]);
                for (uint32_t uRangeMember = uMember + sizeof(ZBaseGeom); uRangeMember < uNextMember; uRangeMember += sizeof(ZBaseGeom))
                {
                    pGetMembers[uTotalCount] = uRangeMember;
                    ++uTotalCount;
                }
            }

            uWriteIndex = uTotalCount;
        }
        while (!IsListEnd(uEntry));

        uint32_t uStaticCount = pGetMembers[0] == 0 ? 0 : uTotalCount;
        const uint32_t uDynamicStart = uStaticCount;

        if (pDynamicBlock)
        {
            uint32_t* pOut = &pGetMembers[uStaticCount];
            do
            {
                uStaticCount += 3;
                pOut[0] = pDynamicBlock[0];
                pOut[1] = pDynamicBlock[1];
                pOut[2] = pDynamicBlock[2];
                pDynamicBlock = DynamicBlockNext(pDynamicBlock);
                pOut += 3;
            }
            while (pDynamicBlock);

            if (pOut[-1] == 0)
            {
                --uStaticCount;
                if (pOut[-2] == 0)
                    --uStaticCount;
            }
        }

        return ((uStaticCount - uDynamicStart) << 16) | uDynamicStart;
    }
    
    bool CListUser::ConnectToList(uint32_t uListID, void* pNodes)
    {
        auto uRes = ConnectBatchToList(uListID, reinterpret_cast<uint32_t*>(&pNodes), 1);
        ZASSERT(uRes == 1 || uRes == 0);
        return uRes == 1;
    }
    
    bool CListUser::DisconnectFromList(uint32_t uListID, void* pNodes)
    {
        ZASSERT(uListID != 0);
        
        return DisconnectFromDynamicList(uListID, pNodes) || DisconnectFromStaticList(uListID, pNodes);
    }
    
    bool CListUser::IsNodeInStaticList(uint32_t uListID, void* pNodes)
    {
        return TrackDownInStaticList(uListID, pNodes, false) == 0;
    }
    
    bool CListUser::IsNodeInDynamicList(uint32_t uListID, void* pNodes)
    {
        return TrackDownInDynamicList(uListID, pNodes, false) == 0;
    }
}
