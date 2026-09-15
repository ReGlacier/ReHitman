#pragma once

namespace Hitman::BloodMoney {
    enum class ESilencerType : int {
        eNotSilent = 0,
        eSilentLowVelocityAmmo = 1,
        eSilentUpTo20 = 2,
        eSilentOver20 = 3,
        eSilentWithNoEvent = 4,
    };
}