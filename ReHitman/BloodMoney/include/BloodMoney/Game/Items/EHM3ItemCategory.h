#pragma once

namespace Hitman
{
    // Reversed from the PC type DB (`EHM3ItemCategory`, 9 values). The generic enumerator names
    // match the original engine enum.
    enum class EHM3ItemCategory : int {
        GUN = 0,
        SMG = 1,
        NONCONCEALABLE = 2,
        KNIFE = 3,
        BOMB = 4,
        SYRINGE = 5,
        FIBER_WIRE = 6,
        NUM_OF_CATEGORIES = 7,
        NONE_SPECIFIED = 8,
    };
}
