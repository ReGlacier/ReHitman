#include <Glacier/Audio/ZEaxBase.h>

#include <cstdint>

namespace Glacier
{
    namespace
    {
        const GUID EAXListenerProperties =
            {0xA8FA6882, 0xB476, 0x11D3, {0xBD, 0xB9, 0x00, 0xC0, 0xF0, 0x2D, 0xDF, 0x87}};
        const GUID EAXBufferProperties =
            {0xA8FA6881, 0xB476, 0x11D3, {0xBD, 0xB9, 0x00, 0xC0, 0xF0, 0x2D, 0xDF, 0x87}};
    }

    ZEaxBase::ZEaxBase()
        : m_pPropertySet(nullptr)
    {
    }

    ZEaxBase::~ZEaxBase()
    {
        if (m_pPropertySet)
            m_pPropertySet->Release();
    }

    bool ZEaxBase::Init(IDirectSoundBuffer* _buffer, bool _setDefaults)
    {
        if (FAILED(_buffer->QueryInterface(IID_IKsPropertySet,
                reinterpret_cast<void**>(&m_pPropertySet))))
        {
            return false;
        }
        if (_setDefaults)
            GetDefaultValues();
        return true;
    }

    void ZEaxBase::Update()
    {
    }

    bool ZEaxBase::QueryEAXSupport()
    {
        if (!m_pPropertySet)
            return false;

        ULONG support = 0;
        constexpr ULONG required = KSPROPERTY_SUPPORT_GET | KSPROPERTY_SUPPORT_SET;
        if (FAILED(m_pPropertySet->QuerySupport(EAXListenerProperties, 1, &support)) ||
            (support & required) != required)
        {
            return false;
        }
        if (FAILED(m_pPropertySet->QuerySupport(EAXBufferProperties, 1, &support)) ||
            (support & required) != required)
        {
            return false;
        }

        int32_t room = -10000;
        return SUCCEEDED(m_pPropertySet->Set(EAXListenerProperties, 5, nullptr, 0,
            &room, sizeof(room)));
    }

    const GUID& GetEAXListenerPropertiesGuid()
    {
        return EAXListenerProperties;
    }

    const GUID& GetEAXBufferPropertiesGuid()
    {
        return EAXBufferProperties;
    }
}
