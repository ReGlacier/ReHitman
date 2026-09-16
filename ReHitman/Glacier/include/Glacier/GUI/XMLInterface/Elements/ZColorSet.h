#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/XMLInterface/System/ZGUIBase.h>


namespace Glacier
{
    class ZColorSet : public ZGUIBase
    {
    public:
        enum EColorIndex
        {
            NormalColor = 0,
            FocusColor = 1,
            DisableColor = 2
        };

        // vtbl
        virtual void readParams(const char** ppParams, ZMenuElements* pElems) override;

        // methods
        ZColorSet();
        uint32_t GetColor(EColorIndex colorIndex);

        //data (total size is 0x58, base size is 0x4C)
        uint32_t m_aiColors[3];
    };
    RE_VERIFY_SIZE(ZColorSet, 0x58); // Verified
}
