#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/XMLInterface/Elements/ZCloseWindow.h>


namespace Glacier
{
    class ZCloseAndDeleteTextMessage : public ZCloseWindow
    {
    public:
        // vtbl
        virtual void Click(eZWUserEvents, int, ZXMLGUISystem*) override;

        // methods
        ZCloseAndDeleteTextMessage();
        ~ZCloseAndDeleteTextMessage();
    };
    RE_VERIFY_SIZE(ZCloseAndDeleteTextMessage, 0x9c); // Verified PC alloc
}
