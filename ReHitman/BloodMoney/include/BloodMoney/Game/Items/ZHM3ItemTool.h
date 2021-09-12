#pragma once

#include "EHM3ItemType.h"
#include <string_view>

namespace Hitman::BloodMoney {
    struct ZHM3ItemTool {
        static std::string_view GetHM3ItemName(EHM3ItemType eItemType);
    };
}