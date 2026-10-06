#include <Glacier/Audio/GlobalsWintel.h>


namespace Glacier
{
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(HANDLE, g_hCmdFrameProcessed, 0x00972590, nullptr);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(HANDLE, g_hSoundEngineReady, 0x009725A0, nullptr);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(HANDLE, g_hFrameReady, 0x00962298, nullptr);
}
