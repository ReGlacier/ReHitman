#pragma once

#include <Glacier/ZUniMemory.h>
#include <Windows.h>



namespace Glacier
{
    STATIC_GLOBAL_CLASS_INSTANCE(HANDLE, g_hCmdFrameProcessed);
    STATIC_GLOBAL_CLASS_INSTANCE(HANDLE, g_hSoundEngineReady);
    STATIC_GLOBAL_CLASS_INSTANCE(HANDLE, g_hFrameReady);
}
