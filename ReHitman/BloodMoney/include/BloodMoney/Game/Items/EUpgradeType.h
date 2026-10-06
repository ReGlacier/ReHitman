#pragma once

namespace Hitman
{
    /**
     * @brief Kind of a custom-weapon upgrade (PC enum `EUpgradeType`, registered through RTP as
     * "RTP::Type::Enum::ZHM3WeaponUpgrade::EUpgradeType"). The individual custom-weapon upgrade
     * geoms carry one of these values and the upgrade control/REFTAB lists are indexed by it.
     */
    enum EUpgradeType : int {
        UT_Dummy = 0, UT_AmmoACP = 1, UT_AmmoArmorPiercing = 2, UT_Ammo127mm = 3, UT_AmmoMagnum = 4,
        UT_AmmoFlechetteSlugs = 5, UT_AmmoGaugesSlugs = 6, UT_AmmoLowVelocity = 7, UT_Magazine = 8,
        UT_Silencer1 = 9, UT_Silencer2 = 10, UT_LaserSight = 11, UT_DualAction = 12, UT_DualActionAuto = 13,
        UT_DoubleCapMag = 14, UT_FullAuto = 15, UT_ReloadBoost = 16, UT_BeltFeeding = 17, UT_BiPod = 18,
        UT_ScopeType1 = 19, UT_ScopeType2 = 20, UT_ScopeType3 = 21, UT_NightVision = 22, UT_Lightweight = 23,
        UT_DefaultAmmo = 24, UT_DefaultNoScope = 25, UT_DefaultBarrel = 26, UT_DefaultMagazine = 27,
        UT_RailMount = 28, UT_CarbonBarrel = 29, UT_Buttstock = 30, UT_Suitcase = 31, UT_DefaultButtStock = 32,
        UT_DefaultGrip = 33, UT_DefaultHandguard = 34, UT_DefaultHandle = 35, UT_DefaultSight = 36,
        UT_BoltAction = 37, UT_RedDotSight = 38, UT_ShortBarrel = 39, UT_PistolGrip = 40, UT_HandGuard = 41,
        UT_RapidFire = 42, UT_LongSlide = 43, UT_ClipX2 = 44, UT_ClipX3 = 45, UT_ClipX4 = 46,
        UT_NumUpgradeTypes = 47
    };
}
