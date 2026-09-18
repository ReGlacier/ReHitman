#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/XMLInterface/Elements/ZNavigator.h>


namespace Glacier
{
    class ZCloseWindow : public ZNavigator
    {
    public:
        // vtbl
        virtual void readParams(const char** ppParams, ZMenuElements* pElems) override;
        virtual void Click(eZWUserEvents, int, ZXMLGUISystem*) override;

        // methods
        ZCloseWindow();
        ~ZCloseWindow();

        // members
        bool m_bFade;     // +0x98
        bool m_bCloseAll; // +0x99
        bool m_bCancel;   // +0x9a
    };
    RE_VERIFY_SIZE(ZCloseWindow, 0x9c); // Verified PC alloc (0x98 on PS2)
    RE_VERIFY_OFFSET(ZCloseWindow, m_bFade, 0x98);
    RE_VERIFY_OFFSET(ZCloseWindow, m_bCloseAll, 0x99);
    RE_VERIFY_OFFSET(ZCloseWindow, m_bCancel, 0x9a);
}
