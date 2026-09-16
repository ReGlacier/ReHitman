#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/XMLInterface/System/ZGUIBase.h>


namespace Glacier
{
    class ZButtonGraphicPartType : public ZGUIBase
    {
    public:
        // vtbl
        virtual void readParams(const char** ppParams, ZMenuElements* pElems) override;

        // methods
        ZButtonGraphicPartType();

        // members
        uint32_t m_iType; // +0x4C
    };
    RE_VERIFY_SIZE(ZButtonGraphicPartType, 0x50);
    RE_VERIFY_OFFSET(ZButtonGraphicPartType, m_iType, 0x4C);
}
