#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/XMLInterface/Elements/ZOption.h>
#include <Glacier/ZSTL/zstring.h>


namespace Glacier
{
    // fwds
    class ZBUTTON;

    class ZToggle : public ZOption
    {
    public:
        // vtbl
        virtual void readParams(const char** ppParams, ZMenuElements* pElems) override;
        virtual void Click(eZWUserEvents, int, ZXMLGUISystem*) override;
        virtual ZGUIElementLink Setup(float*, ZResourceManager*, ZWINGROUP*) override;
        virtual void ReleaseResources(ZResourceManager*) override;
        virtual bool SetFocus(bool) override;
        virtual void Invalidate() override;
        virtual void OptionChanged();
        virtual void SetActive(bool bActive);

        // methods
        ZToggle();
        ~ZToggle();

        // members
        ZBUTTON* m_pZButton; // +0x90
        zstring m_sText;     // +0x94
        bool m_bActive;      // +0xa0
    };
    RE_VERIFY_SIZE(ZToggle, 0xa4);
    RE_VERIFY_OFFSET(ZToggle, m_pZButton, 0x90);
    RE_VERIFY_OFFSET(ZToggle, m_sText, 0x94);
    RE_VERIFY_OFFSET(ZToggle, m_bActive, 0xa0);
}
