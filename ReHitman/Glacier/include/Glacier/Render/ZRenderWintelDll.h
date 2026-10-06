#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/ZRenderBaseDll.h>


namespace Glacier
{
    class ZRenderWintelDll : public ZRenderBaseDll
    {
    public:
        // vtbl
        void CrashFree() override;

        // methods
        ZRenderWintelDll();
    };

    RE_VERIFY_SIZE(ZRenderWintelDll, 0xBC);
}
