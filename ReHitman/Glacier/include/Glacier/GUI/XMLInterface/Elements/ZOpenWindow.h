#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/XMLInterface/Elements/ZNavigator.h>
#include <Glacier/ZSTL/zstring.h>


namespace Glacier
{
    class ZOpenWindow : public ZNavigator
    {
    public:
        // vtbl
        virtual void readParams(const char** ppParams, ZMenuElements* pElems) override;
        virtual void Click(eZWUserEvents, int, ZXMLGUISystem*) override;

        // methods
        ZOpenWindow();
        ~ZOpenWindow();

        void setWindowTarget(const char* pszWindowTarget);
        zstring getWindowTarget();

        // members
        bool m_bPushOnStack;           // +0x98
        bool m_bResetStack;            // +0x99
        bool m_bRollBack;              // +0x9a
        zstring m_sWindow;             // +0x9c
        zstring m_sRollBackPushWindow; // +0xa8
        bool m_bKeepRollBackWindow;    // +0xb4

    protected:
        virtual void Execute();
    };
    RE_VERIFY_SIZE(ZOpenWindow, 0xb8); // Verified PC alloc (0xb4 on PS2)
    RE_VERIFY_OFFSET(ZOpenWindow, m_bPushOnStack, 0x98);
    RE_VERIFY_OFFSET(ZOpenWindow, m_bResetStack, 0x99);
    RE_VERIFY_OFFSET(ZOpenWindow, m_bRollBack, 0x9a);
    RE_VERIFY_OFFSET(ZOpenWindow, m_sWindow, 0x9c);
    RE_VERIFY_OFFSET(ZOpenWindow, m_sRollBackPushWindow, 0xa8);
    RE_VERIFY_OFFSET(ZOpenWindow, m_bKeepRollBackWindow, 0xb4);
}
