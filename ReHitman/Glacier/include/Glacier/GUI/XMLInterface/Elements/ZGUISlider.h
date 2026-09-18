#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/XMLInterface/Elements/ZOption.h>
#include <Glacier/ZSTL/zstring.h>


namespace Glacier
{
    // fwds
    class ZSlider;

    class ZGUISlider : public ZOption
    {
    public:
        // vtbl
        virtual void readParams(const char** ppParams, ZMenuElements* pElems) override;
        virtual void SetValue(int iValue) override;
        virtual ZGUIElementLink Setup(float*, ZResourceManager*, ZWINGROUP*) override;
        virtual void ReleaseResources(ZResourceManager*) override;
        virtual bool SetFocus(bool) override;
        virtual void Invalidate() override;

        // methods
        ZGUISlider();
        ~ZGUISlider();

        // members
        ZSlider* m_pSlider;          // +0x90
        zstring m_sText;             // +0x94
        EAlignment m_eTextAlignment; // +0xa0
        float m_fSliderSize;         // +0xa4 - PC default 100.0f
        float m_fSliderOffset;       // +0xa8
        bool m_bDisplayAsFloat;      // +0xac - PC only
        float m_fFloatMultiplier;    // +0xb0 - PC only
        bool m_bSyncOption;          // +0xb4 - PC only
    };
    RE_VERIFY_SIZE(ZGUISlider, 0xb8); // Verified PC alloc (PS2 is 0xac)
    RE_VERIFY_OFFSET(ZGUISlider, m_pSlider, 0x90);
    RE_VERIFY_OFFSET(ZGUISlider, m_sText, 0x94);
    RE_VERIFY_OFFSET(ZGUISlider, m_eTextAlignment, 0xa0);
    RE_VERIFY_OFFSET(ZGUISlider, m_fSliderSize, 0xa4);
    RE_VERIFY_OFFSET(ZGUISlider, m_fSliderOffset, 0xa8);
    RE_VERIFY_OFFSET(ZGUISlider, m_bDisplayAsFloat, 0xac);
    RE_VERIFY_OFFSET(ZGUISlider, m_fFloatMultiplier, 0xb0);
    RE_VERIFY_OFFSET(ZGUISlider, m_bSyncOption, 0xb4);
}
