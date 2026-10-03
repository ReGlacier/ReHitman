#include <Glacier/Render/ZRD3DDynamicIB32.h>
#include <Glacier/Render/ZDirect3DDevice.h>


namespace Glacier
{
    ZRD3DDynamicIB32::ZRD3DDynamicIB32(ZDirect3DDevice* pDevice, uint32_t lLength, uint32_t lUsage, D3DPOOL ePool)
    {
        m_pLockedData = nullptr;
        m_lCurrentOffset = 0;
        m_bForceReset = false;
        m_lLength = lLength;

        pDevice->CreateIndexBuffer(lLength, lUsage, D3DFMT_INDEX32, ePool, &m_pIndexBuffer, nullptr);
    }

    ZRD3DDynamicIB32::~ZRD3DDynamicIB32()
    {
        m_pIndexBuffer->Release();
    }

    IDirect3DIndexBuffer9* ZRD3DDynamicIB32::Interface()
    {
        return m_pIndexBuffer;
    }

    void* ZRD3DDynamicIB32::Lock(uint32_t lNumIndices, uint32_t& rOutStartIndex)
    {
        const uint32_t lDataSize = 4 * lNumIndices;

        uint32_t lLockFlags;
        if (m_bForceReset || m_lCurrentOffset + lDataSize > m_lLength)
        {
            m_bForceReset = false;
            m_lCurrentOffset = 0;
            lLockFlags = D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK;
        }
        else
        {
            lLockFlags = D3DLOCK_NOOVERWRITE | D3DLOCK_NOSYSLOCK;
        }

        m_pIndexBuffer->Lock(m_lCurrentOffset, lDataSize, &m_pLockedData, lLockFlags);
        rOutStartIndex = m_lCurrentOffset >> 2;
        m_lCurrentOffset += lDataSize;
        return m_pLockedData;
    }

    void ZRD3DDynamicIB32::Unlock()
    {
        m_pIndexBuffer->Unlock();
        m_pLockedData = nullptr;
    }
}
