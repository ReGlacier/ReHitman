#pragma once

namespace Hitman::BloodMoney
{
    enum class ZLnkActionType
    {
        /// Hitman actions
        HitmanActionChangeClothesFromBody		= 0x10000002u,
        HitmanActionPushActor					= 0x10000003u,
        HitmanActionThrowItem					= 0x10000004u,
        HitmanActionPickLock					= 0x10000005u,
        RESERVED_0								= 0x10000007u,						/// ALWAYS NULLPTR
        RESERVED_1								= 0x1000000Cu,						/// ALWAYS NULLPTR
        RESERVED_2								= 0x1000000Eu,						/// ALWAYS NULLPTR
        HitmanActionDropDeadBody				= 0x10000009u,
        HitmanActionHumanShield					= 0x1000000Au,
        HitmanActionHideWeapon					= 0x1000000Du,
        HitmanActionUseKeyCard					= 0x1000000Fu,
        HitmanActionChargeHitKnife				= 0x10000010u,
        HitmanActionChargeHitHedgeCutter		= 0x10000011u,
        HitmanActionHumanShieldRelease			= 0x10000012u,
        HitmanActionChargeHitPickAxe			= 0x10000015u,
        HitmanActionChargeHitFireExtinguisher	= 0x10000016u,
        HitmanActionChargeHitBaseballBat		= 0x10000017u,
        HitmanActionChargeHitPitchfork			= 0x10000018u,
        HitmanActionPackUnpackSuitcase			= 0x1000001Au,
        HitmanActionBalconyJump					= 0x1000001Bu,
        ZHitmanActionPlaceBomb					= 0x1000001Cu,
        HitmanSedateCIAAgent					= 0x1000001Du,
        HitmanPutDownSuitcase					= 0x10000022u,
        HitmanReviveCIAAgent					= 0x10000023u,
        HitmanActionUseSwitch					= 0x10000024u,
        HitmanDropWeight						= 0x10000025u,
        HitmanActionPlaceItemToPos				= 0x10000026u,
        HitmanBreakUtilBox						= 0x10000027u,
        HitmanActionChargeHitHammer				= 0x10000029u,
        HitmanUnlockDoor						= 0x1000002Au,
        HitmanActionChargeHitStungun			= 0x1000002Bu,
        HitmanActionPourFluidOverDevice			= 0x1000002Eu,
        HitmanActionChargeHitScrewdriver		= 0x1000002Fu,
        HitmanActionCloseCombatStunTarget		= 0x10000030u,
        HitmanActionCloseCombatDisarmTarget		= 0x10000031u,
        HitmanStartStrangleInElev				= 0x10000032u,
        HitmanActionStartDragBody				= 0x10000033u,
        HitmanUsePhone							= 0x10000034u,
        HitmanOpenGas							= 0x10000035u,
        HitmanUseComputer						= 0x10000036u,
        HitmanActionDestroyDualWeapons			= 0x10000037u,
        HitmanPickupSuitcase					= 0x10000038u,
        HitmanPhoning							= 0x10000039u,	/// M04 ONLY, CAN CRASH IN OTHER LEVEL
        HitmanPickupBox							= 0x1000003Au,
        HitmanPutDownBox						= 0x1000003Bu,
        HitmanBoxFromElevator					= 0x1000003Cu,
        HitmanBoxToElevator						= 0x1000003Du,
        HitmanActionDieIngame					= 0x1000003Eu,
        HitmanActionStealTape					= 0x1000003Fu,
        HitmanActionChargeHitShovel				= 0x10000040u,
        HitmanPickupBurgers						= 0x10000041u,
        HitmanPutDownBurgers					= 0x10000042u,
        HitmanActionKnockOnDoor					= 0x10000043u,
        HitmanActionPackUnpackRifle				= 0x10000044u,
        HitmanActionChargeHitCaneSword			= 0x10000045u,
        HitmanActionPoisonBottle				= 0x10000046u,
        /// Other actions
        HitmanActionPickupItem_0				= 0x3u,	//Equal code, not reversed, need to explore
        HitmanActionPickupItem_1				= 0x4u,
        HitmanActionPutItem_0					= 0x5u,
        HitmanActionPutItem_1					= 0x6u,
        ZHitmanActionGetItem_0					= 0x7u,
        ZHitmanActionGetItem_1					= 0x8u,
        HitmanActionDropItem_0					= 0x9u,
        HitmanActionDropItem_1					= 0xAu,
        HitmanActionSwapItems					= 0xBu,
        HitmanActionReloadItem_0				= 0xCu,
        HitmanActionReloadItem_1				= 0xDu,
        HitmanActionChamberItem_0				= 0xEu,
        HitmanActionChamberItem_1				= 0xFu,
    };
}