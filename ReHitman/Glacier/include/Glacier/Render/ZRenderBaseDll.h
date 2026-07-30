#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZDllBase.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    // TODO: Finish me
    struct ZRenderBaseDll : public ZDllBase
    {};

    STATIC_GLOBAL_CLASS_INSTANCE(ZRenderBaseDll*, g_pRenderDll);
}