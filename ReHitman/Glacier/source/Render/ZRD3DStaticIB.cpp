#include <Glacier/Render/ZRD3DStaticIB.h>
#include <Glacier/Render/ZDirect3DDevice.h>


namespace Glacier
{
    // ZRD3DStaticIB
    ZRD3DStaticIB::ZRD3DStaticIB(ZDirect3DDevice* pDevice, uint32_t lLength, uint32_t lUsage, D3DPOOL ePool)
    {
        m_pLockedData = nullptr;
        m_lLength = lLength;

        pDevice->CreateIndexBuffer(lLength, lUsage, D3DFMT_INDEX16, ePool, &m_pIndexBuffer, nullptr);
    }

    ZRD3DStaticIB::~ZRD3DStaticIB()
    {
        if (m_pLockedData)
        {
            m_pLockedData = nullptr;
            m_pIndexBuffer->Unlock();
        }

        m_pIndexBuffer->Release();
    }

    IDirect3DIndexBuffer9* ZRD3DStaticIB::Interface()
    {
        if (m_pLockedData)
        {
            m_pLockedData = nullptr;
            m_pIndexBuffer->Unlock();
        }

        return m_pIndexBuffer;
    }

    void* ZRD3DStaticIB::LockedData()
    {
        if (!m_pLockedData)
        {
            m_pIndexBuffer->Lock(0, 0, &m_pLockedData, 0);
        }

        return m_pLockedData;
    }
}
