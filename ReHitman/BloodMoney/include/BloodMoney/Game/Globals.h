#pragma once

#include <cstdint>

namespace Hitman::BloodMoney::Globals
{
    static constexpr std::uintptr_t kSysInterfaceAddr  = 0x00820820;
    static constexpr std::uintptr_t kGameDataAddr      = 0x0082083C;
    static constexpr std::uintptr_t kFileSystemAddr    = 0x0082081C;
    static constexpr std::uintptr_t kGeomBufferAddr    = 0x008BA0C0;
    static constexpr std::uintptr_t kActionManagerAddr = 0x008ACAC4;
}