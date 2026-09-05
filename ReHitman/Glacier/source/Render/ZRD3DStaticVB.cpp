#include <Glacier/Render/ZRD3DStaticVB.h>
#include <Glacier/Render/ZDirect3DDevice.h>


namespace Glacier
{
    ZRD3DStaticVB::ZRD3DStaticVB(ZDirect3DDevice* pDevice, uint32_t lLength, uint32_t lUsage, D3DPOOL ePool)
    {
        m_pLockedData = nullptr;
        m_lLength = lLength;

        pDevice->CreateVertexBuffer(lLength, lUsage, 0, ePool, &m_pVertexBuffer, nullptr);
    }

    ZRD3DStaticVB::~ZRD3DStaticVB()
    {
        if (m_pLockedData)
        {
            m_pLockedData = nullptr;
            m_pVertexBuffer->Unlock();
        }

        m_pVertexBuffer->Release();
    }

    IDirect3DVertexBuffer9* ZRD3DStaticVB::Interface()
    {
        auto* pResult = m_pVertexBuffer;
        if (m_pLockedData)
        {
            m_pLockedData = nullptr;
            pResult->Unlock();
            return m_pVertexBuffer;
        }

        return pResult;
    }

    void* ZRD3DStaticVB::LockedData()
    {
        if (!m_pLockedData)
        {
            m_pVertexBuffer->Lock(0, 0, &m_pLockedData, 0);
        }

        return m_pLockedData;
    }
}