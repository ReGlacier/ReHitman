#pragma once

namespace Hitman::BloodMoney
{
    enum ESecurityZone {
        eZoneUNDEFINED = 0,
        eZone1 = 1,
        eZone2 = 2,
        eZone2A = 4,
        eZone2B = 8,
        eZone3 = 16,
        eZone3A = 32,
        eZone3B = 64,
        eZoneMegaForbidden = 128,
    };
}