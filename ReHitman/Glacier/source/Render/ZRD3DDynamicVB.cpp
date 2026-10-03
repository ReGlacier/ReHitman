#include <Glacier/Render/ZRD3DDynamicVB.h>
#include <Glacier/Render/ZDirect3DDevice.h>


namespace Glacier
{
    ZRD3DDynamicVB::ZRD3DDynamicVB(ZDirect3DDevice* pDevice, uint32_t lLength, uint32_t lUsage, D3DPOOL ePool)
    {
        m_pLockedData = nullptr;
        m_lCurrentOffset = 0;
        m_bForceReset = false;
        m_lLength = lLength;

        pDevice->CreateVertexBuffer(lLength, lUsage, 0, ePool, &m_pVertexBuffer, nullptr);
    }

    ZRD3DDynamicVB::~ZRD3DDynamicVB()
    {
        m_pVertexBuffer->Release();
    }

    IDirect3DVertexBuffer9* ZRD3DDynamicVB::Interface()
    {
        return m_pVertexBuffer;
    }

    void* ZRD3DDynamicVB::Lock(uint32_t lNumVertices, uint32_t lVertexSize, uint32_t& rOutStartVertex)
    {
        const uint32_t lDataSize = lNumVertices * lVertexSize;
        // Round the write position up to the next multiple of lVertexSize (never backwards).
        uint32_t lOffset = lVertexSize + (m_lCurrentOffset / lVertexSize) * lVertexSize;
        m_lCurrentOffset = lOffset;

        uint32_t lLockFlags;
        if (m_bForceReset || lOffset + lDataSize > m_lLength)
        {
            m_bForceReset = false;
            m_lCurrentOffset = 0;
            lOffset = 0;
            lLockFlags = D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK;
        }
        else
        {
            lLockFlags = D3DLOCK_NOOVERWRITE | D3DLOCK_NOSYSLOCK;
        }

        m_pVertexBuffer->Lock(lOffset, lDataSize, &m_pLockedData, lLockFlags);
        rOutStartVertex = m_lCurrentOffset / lVertexSize;
        m_lCurrentOffset += lDataSize;
        return m_pLockedData;
    }

    void ZRD3DDynamicVB::Unlock()
    {
        m_pVertexBuffer->Unlock();
        m_pLockedData = nullptr;
    }
}
