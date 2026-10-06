#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/XMLInterface/Elements/ZNavigator.h>


namespace Glacier
{
    class ZQuitGame : public ZNavigator
    {
    public:
        // vtbl
        virtual void Click(eZWUserEvents, int, ZXMLGUISystem*) override;

        // methods
        ZQuitGame();
        ~ZQuitGame();
    };
    RE_VERIFY_SIZE(ZQuitGame, 0x98); // Verified PC alloc (0x94 on PS2)
}
