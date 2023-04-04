#pragma once

namespace Glacier {
    enum class EWeaponOperation : int {
        WO_MANUAL = 1,
        WO_SEMIAUTO = 2,
        WO_FULLAUTO = 4,
        WO_FULLAUTO3 = 8
    };
}