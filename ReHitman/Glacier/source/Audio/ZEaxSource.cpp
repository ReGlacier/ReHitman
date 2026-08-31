#include <Glacier/Audio/ZEaxSource.h>

namespace Glacier
{
    const GUID& GetEAXBufferPropertiesGuid();

    ZEaxSource::ZEaxSource()
        : m_Properties{}
    {
        m_Properties.m_fOcclusionLFRatio = 0.25f;
        m_Properties.m_fOcclusionRoomRatio = 0.5f;
    }

    ZEaxSource::~ZEaxSource() = default;

    void ZEaxSource::Update()
    {
        if (!m_pPropertySet)
            return;
        m_Properties.m_lFlags = 7;
        m_pPropertySet->Set(GetEAXBufferPropertiesGuid(), 0x80000001, nullptr, 0,
            &m_Properties, sizeof(m_Properties));
    }

    void ZEaxSource::GetDefaultValues()
    {
        if (!m_pPropertySet)
            return;

        m_Properties = {};
        m_Properties.m_fOcclusionLFRatio = 0.25f;
        m_Properties.m_fOcclusionRoomRatio = 1.5f;
        m_Properties.m_fOcclusionDirectRatio = 1.0f;
        m_Properties.m_fExclusionLFRatio = 1.0f;
        m_Properties.m_fAirAbsorptionFactor = 1.0f;
        m_Properties.m_lFlags = 7;
        Update();
    }
}
