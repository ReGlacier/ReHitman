#include <Glacier/ZSTL/ZMath.h>

#include <Glacier/System/ZSysInterface.h>


namespace Glacier
{
    // engine/zstdlib/zmath.cpp. PC: Engine__Randomrange (0x59DA80).
    // The PC build uses the exclusive upper bound form `FRand() * (iMax - iMin) + iMin`.
    int RandomRange(int iMin, int iMax)
    {
        return static_cast<int>(
            g_pSysInterface->FRand(const_cast<char*>(__FILE__), __LINE__) * static_cast<double>(iMax - iMin)
            + static_cast<double>(iMin));
    }
}
