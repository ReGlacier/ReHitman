#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/XMLInterface/Elements/ZNavigator.h>


namespace Glacier
{
    class ZOpenSubwindow : public ZNavigator
    {
    public:
        // vtbl
        virtual void readParams(const char** ppParams, ZMenuElements* pElems) override;
        virtual void Update(bool) override;
        virtual void Click(eZWUserEvents, int, ZXMLGUISystem*) override;
        virtual const char* GetWindow() override;

        // methods
        ZOpenSubwindow();
        ~ZOpenSubwindow();

        // members
        char m_szWindow[64]; // +0x98
        bool m_bResetStack;  // +0xd8
    };
    RE_VERIFY_SIZE(ZOpenSubwindow, 0xdc); // Verified PC alloc (0xd8 on PS2)
    RE_VERIFY_OFFSET(ZOpenSubwindow, m_szWindow, 0x98);
    RE_VERIFY_OFFSET(ZOpenSubwindow, m_bResetStack, 0xd8);
}
