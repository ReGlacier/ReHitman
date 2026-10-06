#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Component/ZComponentSingleton.h>
#include <Glacier/Component/ZRuntimeComponentBase.h>


namespace Glacier
{
    class ZScriptCComponent : public ZComponentSingleton<ZScriptCComponent, ZRuntimeComponentBase>
    {
    public:
        // vtbl
        ~ZScriptCComponent() override;
        
        // methods
        ZScriptCComponent();

        // members
    };
    RE_VERIFY_SIZE(ZScriptCComponent, 0x10);
}