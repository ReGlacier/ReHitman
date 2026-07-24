#include <Glacier/Filesystem/ZBufferReader.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZBufferReader::ZBufferReader() = default;
    ZBufferReader::~ZBufferReader() = default;

    const void* ZBufferReader::GetData(uint32_t lOffset)
    {
        ZASSERT(m_pBuffer);

        if (m_pBuffer)
        {
            ZASSERT(lOffset <= m_lBufferEnd);
            if (lOffset <= m_lBufferEnd)
            {
                return &m_pBuffer[lOffset];
            }
        }

        return nullptr;
    }

    uint32_t ZBufferReader::GetBufferSize()
    {
        return m_lBufferSize;
    }

    void ZBufferReader::GetBufferData(Callback_t pCallback, void* pData)
    {
        ZASSERT(pCallback);
        pCallback(m_pBuffer, m_lBufferSize, pData);
    }

    void ZBufferReader::InstallBuffer(void* pBuffer, uint32_t lBufferSize, uint32_t lBufferSizeAllocated)
    {
        FreeResources();
        FreeBuffer();

        m_lBufferSize = lBufferSize;
        m_lBufferSizeAllocated = lBufferSizeAllocated;
        m_lBufferEnd = lBufferSize;
        m_pBuffer = static_cast<char*>(pBuffer);
    }

    void ZBufferReader::FreeBuffer()
    {
        if (m_pBuffer)
        {
            ZUniMemory::Free(m_pBuffer);
        }

        m_pBuffer = nullptr;
        m_lBufferSize = 0;
        m_lBufferSizeAllocated = 0;
        m_lBufferEnd = 0;
    }

    void ZBufferReader::AllocateResources()
    {
        // Do nothing here
    }

    void ZBufferReader::FreeResources()
    {
        // Do nothing here
    }
}
