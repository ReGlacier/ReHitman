#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/XMLInterface/Elements/ZCloseWindow.h>


namespace Glacier
{
    class ZCloseAndDeleteVoiceMessage : public ZCloseWindow
    {
    public:
        // vtbl
        virtual void Click(eZWUserEvents, int, ZXMLGUISystem*) override;

        // methods
        ZCloseAndDeleteVoiceMessage();
        ~ZCloseAndDeleteVoiceMessage();
    };
    RE_VERIFY_SIZE(ZCloseAndDeleteVoiceMessage, 0x9c); // Verified PC alloc
}
