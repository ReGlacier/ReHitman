#pragma once

#include <Glacier/GUI/XMLInterface/Elements/ZGUISlider.h>


namespace Glacier
{
    class ZGUISliderText : public ZGUISlider
    {
    public:
        // vtbl
        virtual void readParams(const char** ppParams, ZMenuElements* pElems) override;
        virtual void addElement(const char* pName, ZGUIBase* pEntry) override;
        virtual void SetValue(int iValue) override;

        // methods
        ZGUISliderText();
        ~ZGUISliderText();

        // members
        zstring m_sTextValues[8];  // +0xb8
        int32_t m_iNumOfTextAdded; // +0x118
        bool m_bUseOptionText;     // +0x11c
    };
    RE_VERIFY_SIZE(ZGUISliderText, 0x120); // Verified PC alloc 288 (PS2 is 0x110)
    RE_VERIFY_OFFSET(ZGUISliderText, m_sTextValues, 0xb8);
    RE_VERIFY_OFFSET(ZGUISliderText, m_iNumOfTextAdded, 0x118);
    RE_VERIFY_OFFSET(ZGUISliderText, m_bUseOptionText, 0x11c);
}
