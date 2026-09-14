#include <Glacier/Render/ZShadowListUser.h>


namespace Glacier
{
    void ZShadowListUser::Init(uint8_t* pxData)
    {
        m_pxData = pxData;
        m_pxWork = pxData + 4;
    }

    int ZShadowListUser::GetNumSets() const
    {
        return *reinterpret_cast<int*>(m_pxData);
    }

    SetHeader* ZShadowListUser::GetSetHeader()
    {
        auto* ptr = m_pxWork;
        m_pxWork = m_pxWork + sizeof(SetHeader);
        return reinterpret_cast<SetHeader*>(ptr);
    }

    CasterHeader* ZShadowListUser::GetCasterHeader()
    {
        auto* ptr = m_pxWork;
        m_pxWork = m_pxWork + sizeof(CasterHeader);
        return reinterpret_cast<CasterHeader*>(ptr);
    }

    const ZBaseGeom** ZShadowListUser::GetReceiverBase(int nReceivers)
    {
        const ZBaseGeom** ptr = reinterpret_cast<const ZBaseGeom**>(m_pxWork);
        m_pxWork = reinterpret_cast<uint8_t*>(&ptr[nReceivers]);
        return ptr;
    }
}
