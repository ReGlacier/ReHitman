#pragma once

#include <cstdint>

namespace Hitman::BloodMoney::Globals
{
    static constexpr std::intptr_t kSysInterfaceAddr = 0x00820820;
    static constexpr std::intptr_t kGameDataAddr     = 0x0082083C;
    static constexpr std::uintptr_t kFileSystemAddr  = 0x0082081C;
}