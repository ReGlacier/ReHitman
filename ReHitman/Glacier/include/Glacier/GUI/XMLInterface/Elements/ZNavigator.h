#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/XMLInterface/Elements/IGUIElement.h>
#include <Glacier/ZSTL/zstring.h>


namespace Glacier
{
    // fwds
    class ZBUTTON;

    class ZNavigator : public IGUIElement
    {
    public:
        // vtbl
        virtual void readParams(const char** ppParams, ZMenuElements* pElems) override;
        virtual void Click(eZWUserEvents, int, ZXMLGUISystem*) override;
        virtual ZGUIElementLink Setup(float*, ZResourceManager*, ZWINGROUP*) override;
        virtual void ReleaseResources(ZResourceManager*) override;
        virtual bool SetFocus(bool) override;
        virtual void SetText();
        virtual void ForceSetText(const char* pText);

        // methods
        ZNavigator();
        ~ZNavigator();

        // members
        ZBUTTON* m_pButton;               // +0x68
        int32_t m_iActivateButton;        // +0x6c
        zstring m_sText;                  // +0x70
        zstring m_sMoveFocus;             // +0x7c
        int32_t m_iMoveButton;            // +0x88
        bool m_bReceiveAllInput;          // +0x8c
        bool m_bLink;                     // +0x8d
        bool m_bDisable;                  // +0x8e
        bool m_bTakeFocus;                // +0x8f
        bool m_bMouseColiOnHiddenObjects; // +0x90
        uint32_t m_iType;                 // +0x94

    protected:
        const char* GetText();
        void SetType(uint32_t iType);
    };
    RE_VERIFY_SIZE(ZNavigator, 0x98);
    RE_VERIFY_OFFSET(ZNavigator, m_pButton, 0x68);
    RE_VERIFY_OFFSET(ZNavigator, m_iActivateButton, 0x6c);
    RE_VERIFY_OFFSET(ZNavigator, m_sText, 0x70);
    RE_VERIFY_OFFSET(ZNavigator, m_sMoveFocus, 0x7c);
    RE_VERIFY_OFFSET(ZNavigator, m_iMoveButton, 0x88);
    RE_VERIFY_OFFSET(ZNavigator, m_bReceiveAllInput, 0x8c);
    RE_VERIFY_OFFSET(ZNavigator, m_bLink, 0x8d);
    RE_VERIFY_OFFSET(ZNavigator, m_bDisable, 0x8e);
    RE_VERIFY_OFFSET(ZNavigator, m_bTakeFocus, 0x8f);
    RE_VERIFY_OFFSET(ZNavigator, m_bMouseColiOnHiddenObjects, 0x90);
    RE_VERIFY_OFFSET(ZNavigator, m_iType, 0x94);
}
