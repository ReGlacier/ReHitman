#include <Glacier/ZSTL/ZCyclicBuffer.h>
#include <Glacier/ZUniMemory.h>

#include <cstring>


namespace Glacier
{
    ZCyclicBuffer::ZCyclicBuffer(int iBufferSize)
    {
        m_iBufferSize = iBufferSize;
        m_pBuffer = static_cast<char*>(ZUniMemory::Allocate(iBufferSize));

        if (!m_pBuffer)
            m_iBufferSize = 0;

        m_iFirst = 0;
        m_iCount = 0;
        m_iStart = 0;
        m_iEnd = 0;
    }

    ZCyclicBuffer::~ZCyclicBuffer()
    {
        if (m_iBufferSize && m_pBuffer)
            ZUniMemory::Free(m_pBuffer);
    }

    void* ZCyclicBuffer::Alloc(int iSize)
    {
        int iAlignedSize = (iSize & 3) ? (iSize - (iSize & 3) + 4) : iSize;

        if (m_iBufferSize < iAlignedSize + 8)
            return nullptr;

        while (SpaceFree() < iAlignedSize)
            FreeSpace();

        if (m_iBufferSize < m_iEnd + iAlignedSize + 8)
            m_iEnd = 0;

        *reinterpret_cast<int*>(m_pBuffer + m_iEnd) = iAlignedSize;

        void* pRecord = m_pBuffer + m_iEnd + 4;

        m_iEnd += iAlignedSize + 4;
        *reinterpret_cast<int*>(m_pBuffer + m_iEnd) = -1;

        ++m_iCount;
        return pRecord;
    }

    void* ZCyclicBuffer::Get(int iIndex)
    {
        if (iIndex < 0)
            iIndex += m_iFirst + m_iCount;

        if (iIndex < m_iFirst || iIndex >= m_iFirst + m_iCount)
            return nullptr;

        int iOffset = m_iStart;

        for (int i = iIndex - m_iFirst; i; --i)
        {
            int iSize = *reinterpret_cast<int*>(m_pBuffer + iOffset);

            if (iSize == -1)
            {
                iOffset = 0;
                iSize = *reinterpret_cast<int*>(m_pBuffer);
            }

            iOffset += iSize + 4;
        }

        return m_pBuffer + iOffset + 4;
    }

    int ZCyclicBuffer::Last()
    {
        return m_iFirst + m_iCount - 1;
    }

    int ZCyclicBuffer::First()
    {
        return m_iFirst;
    }

    int ZCyclicBuffer::Count()
    {
        return m_iCount;
    }

    void ZCyclicBuffer::FreeSpace()
    {
        if (!m_iCount)
            return;

        --m_iCount;
        ++m_iFirst;

        if (m_iCount)
        {
            m_iStart += *reinterpret_cast<int*>(m_pBuffer + m_iStart) + 4;

            if (*reinterpret_cast<int*>(m_pBuffer + m_iStart) == -1)
                m_iStart = 0;
        }
        else
        {
            m_iStart = 0;
            m_iEnd = 0;
        }
    }

    int ZCyclicBuffer::SpaceFree()
    {
        if (m_iEnd < m_iStart)
            return m_iStart - m_iEnd - 8;

        int iFree = m_iBufferSize - m_iEnd;

        if (iFree < m_iStart)
            iFree = m_iStart;

        return iFree - 8;
    }
}
