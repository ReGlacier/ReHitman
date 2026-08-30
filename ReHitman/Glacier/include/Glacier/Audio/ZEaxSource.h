#pragma once

#include <Glacier/Audio/Eax3.h>
#include <Glacier/Audio/ZEaxBase.h>

namespace Glacier
{
    class ZEaxSource : public ZEaxBase
    {
    public:
        // vtbl
        ~ZEaxSource() override;
        void Update() override;
        void GetDefaultValues() override;

        // methods
        ZEaxSource();

        // members
        EaxBufferProperties m_Properties;
    };

    RE_VERIFY_OFFSET(ZEaxSource, m_Properties, 0x08);
    RE_VERIFY_SIZE(ZEaxSource, 0x50);
}
