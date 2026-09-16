#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/XMLInterface/Elements/ZToggle.h>


namespace Glacier
{
    // fwds
    class ZResourceManager;
    class ZButtonGraphic;

    class ZCheckBox : public ZToggle
    {
    public:
        // vtbl
        virtual void readParams(const char** ppParams, ZMenuElements* pElems) override;
        virtual ZGUIElementLink Setup(float*, ZResourceManager*, ZWINGROUP*) override;
        virtual void OptionChanged() override;

        // methods
        ZCheckBox();
        ~ZCheckBox();

        // members
        ZResourceManager* m_pResourceManager; // +0xa4
        ZButtonGraphic* m_pBGChecked;         // +0xa8
        ZButtonGraphic* m_pBGUnchecked;       // +0xac
        bool m_bSetup;                        // +0xb0
    };
    RE_VERIFY_SIZE(ZCheckBox, 0xb4);
    RE_VERIFY_OFFSET(ZCheckBox, m_pResourceManager, 0xa4);
    RE_VERIFY_OFFSET(ZCheckBox, m_pBGChecked, 0xa8);
    RE_VERIFY_OFFSET(ZCheckBox, m_pBGUnchecked, 0xac);
    RE_VERIFY_OFFSET(ZCheckBox, m_bSetup, 0xb0);
}
