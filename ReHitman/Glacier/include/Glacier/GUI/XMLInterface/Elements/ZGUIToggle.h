#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/XMLInterface/Elements/ZToggle.h>
#include <Glacier/GUI/XMLInterface/Elements/ZGraphic.h>
#include <Glacier/ZSTL/zvector.h>


namespace Glacier
{
    // fwds
    class ZResourceManager;
    class ZColorSet;

    class ZGUIToggle : public ZToggle
    {
    public:
        // vtbl
        virtual void addElement(const char* pName, ZGUIBase* pEntry) override;
        virtual ZGUIElementLink Setup(float*, ZResourceManager*, ZWINGROUP*) override;
        virtual void ReleaseResources(ZResourceManager*) override;
        virtual void OptionChanged() override;

        // methods
        ZGUIToggle();
        ~ZGUIToggle();

        // members
        zvector<ZGraphic*> m_vecGraphic;      // +0xa4
        ZResourceManager* m_pResourceManager; // +0xb0
        ZWINGROUP* m_pParent;                 // +0xb4

    protected:
        void SetupGraphic();
        void ChangeColorSet(ZColorSet* pColorSet);
    };
    RE_VERIFY_SIZE(ZGUIToggle, 0xb8);
    RE_VERIFY_OFFSET(ZGUIToggle, m_vecGraphic, 0xa4);
    RE_VERIFY_OFFSET(ZGUIToggle, m_pResourceManager, 0xb0);
    RE_VERIFY_OFFSET(ZGUIToggle, m_pParent, 0xb4);
}
