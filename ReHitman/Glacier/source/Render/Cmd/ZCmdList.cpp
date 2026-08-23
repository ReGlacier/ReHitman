#include <Glacier/Render/Cmd/ZCmdList.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    void* ZCmdList::ZCmd::AddData(uint32_t lDataSize)
    {
        ZASSERT(lDataSize > 0);
        ZASSERT(!(lDataSize & 0x3) && (lDataSize + m_pCmdList->m_pCurrent <= m_pCmdList->m_pBufferEnd));

        auto* pRet = m_pCmdList->m_pCurrent;
        m_pCmdList->m_pCurrent += lDataSize;
        m_lNrObjects += lDataSize >> 2;
        return pRet;
    }

    void* ZCmdList::ZCmd::AddData(void* pData, uint32_t lDataSize)
    {
        ZASSERT(lDataSize > 0);
        ZASSERT(!(lDataSize & 0x3) && (lDataSize + m_pCmdList->m_pCurrent <= m_pCmdList->m_pBufferEnd));

        void* pResult = memcpy(m_pCmdList->m_pCurrent, pData, lDataSize);
        m_pCmdList->m_pCurrent += lDataSize;
        m_lNrObjects += lDataSize >> 2;
        return pResult;
    }

    ZCmdList::ZCmd* ZCmdList::ZCmd::Next() const
    {
        return const_cast<ZCmdList::ZCmd*>(reinterpret_cast<const ZCmdList::ZCmd*>(reinterpret_cast<const char*>(this) + sizeof(std::intptr_t) * m_lNrObjects + sizeof(ZCmdList::ZCmd)));
    }

    ZCmdList::ZCmd* ZCmdList::Current() const
    {
        return reinterpret_cast<ZCmd*>(m_pCurrent);
    }

    void ZCmdList::NextCommand()
    {
        auto* pNext = reinterpret_cast<ZCmdList::ZCmd*>(m_pCurrent) + 1;
        m_lNrCmds++;
        m_pCurrent = reinterpret_cast<char*>(pNext);
    }
}
