#include <BloodMoney/Game/Items/ZHM3ItemTool.h>

#include <BloodMoney/Game/Items/ZHM3Item.h>
#include <BloodMoney/Game/Items/ZHM3ItemContainer.h>
#include <BloodMoney/Game/Items/ZHM3ItemTemplate.h>
#include <BloodMoney/Game/Items/ZHM3ItemTemplateWeapon.h>
#include <BloodMoney/Game/Items/ZHM3ItemWeapon.h>

#include <Glacier/Items/ZItem.h>
#include <Glacier/Items/ZItemTemplate.h>
#include <Glacier/ZSTL/StringUtils.h>
#include <Glacier/ZUniAssert.h>

namespace Hitman
{
    namespace
    {
        struct SHM3ItemTypeName
        {
            EHM3ItemType type;
            const char* pszName;
        };

        // PC 0x8031E0 (`g_aHM3ItemTypeNameList`): ordered by item type; maps each EHM3ItemType to
        // its script/editor name. The table is also read by HM3 routines outside this class
        // (e.g. ZHM3ItemWeapon::GetBloodyVersionOfWeapon, sub_649C60) that are not reversed yet.
        const SHM3ItemTypeName g_aHM3ItemTypeNameList[] =
        {
            { EHM3ItemType::eHM3NoType, "NoType" },
            { EHM3ItemType::eHM3Lockpick, "Item_LockPick" },
            { EHM3ItemType::eHM3CustomHardballer, "Custom_Pistol" },
            { EHM3ItemType::eHM3CustomHardballerPickup, "Custom_Pistol_PICKUP" },
            { EHM3ItemType::eHM3CustomSniper, "Custom_SniperRifle" },
            { EHM3ItemType::eHM3CustomSMG, "Custom_SubMachineGun" },
            { EHM3ItemType::eHM3CustomMG, "Custom_AssaultRifle" },
            { EHM3ItemType::eHM3CustomSG, "Custom_ShotGun" },
            { EHM3ItemType::eHM3RemoteBomb, "Equip_Bomb_01" },
            { EHM3ItemType::eHM3RemoteControl, "Equip_BombRemote_01" },
            { EHM3ItemType::eHM3Gun_Browning_01, "Gun_Browning_01" },
            { EHM3ItemType::eHM3Gun_DesertEagle_01, "Gun_DesertEagle_01" },
            { EHM3ItemType::eHM3Gun_HKusp_01, "Gun_HKusp_01" },
            { EHM3ItemType::eHM3Gun_HKusp_Silenced_01, "Gun_HKusp_Silenced_01" },
            { EHM3ItemType::eHM3Gun_Mauser_01, "Gun_Mauser_01" },
            { EHM3ItemType::eHM3Gun_PropMauser_01, "Gun_PropMauser_01" },
            { EHM3ItemType::eHM3Gun_NailGun_01, "Gun_NailGun_01" },
            { EHM3ItemType::eHM3Gun_SixShooter_01, "Gun_SixShooter_01" },
            { EHM3ItemType::eHM3Gun_StubNosed_01, "Gun_SnubNosed_01" },
            { EHM3ItemType::eHM3Gun_Taurus_01, "Gun_Taurus_01" },
            { EHM3ItemType::eHM3Gun_Albino_01, "Gun_Albino_01" },
            { EHM3ItemType::eHM3Rifle_Airrifle_01_unused, "Rifle_Airrifle_01_unused" },
            { EHM3ItemType::eHM3Rifle_Airrifle_Tranquilizer_01, "Rifle_Airrifle_Tranquilizer_01" },
            { EHM3ItemType::eHM3Rifle_Enfield_01, "Rifle_Enfield_01" },
            { EHM3ItemType::eHM3Rifle_FN2000_01, "Rifle_FN2000_01" },
            { EHM3ItemType::eHM3Rifle_M14_01, "Rifle_M14_01" },
            { EHM3ItemType::eHM3Rifle_Remington_01, "Rifle_Remington_01" },
            { EHM3ItemType::eHM3Rifle_SG552_01, "Rifle_SG552_01" },
            { EHM3ItemType::eHM3Sniper, "Sniper" },
            { EHM3ItemType::eHM3SniperRifle_Browning_01, "SniperRifle_Browning_01" },
            { EHM3ItemType::eHM3SniperRifle_Dragunov_01, "SniperRifle_Dragunov_01" },
            { EHM3ItemType::eHM3SniperRifle_SakoTRG_01, "SniperRifle_SakoTRG_01" },
            { EHM3ItemType::eHM3SMG_FamaeSAF_01, "SMG_FamaeSAF_01" },
            { EHM3ItemType::eHM3SMG_MP5_01, "SMG_MP5_01" },
            { EHM3ItemType::eHM3SMG_MP7_01, "SMG_MP7_01" },
            { EHM3ItemType::eHM3SMG_RugerMP9_01, "SMG_RugerMP9_01" },
            { EHM3ItemType::eHM3SMG_SteyrTMP_01, "SMG_SteyrTMP_01" },
            { EHM3ItemType::eHM3CC_BaseballBat_01, "CC_BaseballBat_01" },
            { EHM3ItemType::eHM3CC_FiberWire_01, "CC_FiberWire_01" },
            { EHM3ItemType::eHM3CC_FireExtinguisher_01, "CC_FireExtinguisher_01" },
            { EHM3ItemType::eHM3CC_Knife_Bowie_01, "CC_Knife_Bowie_01" },
            { EHM3ItemType::eHM3CC_Knife_Butterfly_01, "CC_Knife_Butterfly_01" },
            { EHM3ItemType::eHM3CC_Knife_Kitchen_01, "CC_Knife_Kitchen_01" },
            { EHM3ItemType::eHM3CC_Knife_Throwing_01, "CC_Knife_Throwing_01" },
            { EHM3ItemType::eHM3CC_NailFile_01, "CC_NailFile_01" },
            { EHM3ItemType::eHM3CC_Pens_01, "CC_Pens_01" },
            { EHM3ItemType::eHM3CC_PickAxe_01, "CC_PickAxe_01" },
            { EHM3ItemType::eHM3CC_StunGun_01, "CC_StunGun_01" },
            { EHM3ItemType::eHM3CC_Sword_Cane_01, "CC_Sword_Cane_01" },
            { EHM3ItemType::eHM3CC_Sword_Cane_03, "CC_Sword_Cane_03" },
            { EHM3ItemType::eHM3CC_Sword_Marine_01, "CC_Sword_Marine_01" },
            { EHM3ItemType::eHM3CC_Syringe_Anastetic_01, "CC_Syringe_Anastetic_01" },
            { EHM3ItemType::eHM3CC_Syringe_HeartAttack_01, "CC_Syringe_HeartAttack_01" },
            { EHM3ItemType::eHM3CC_Syringe_Laxative_01, "CC_Syringe_Laxative_01" },
            { EHM3ItemType::eHM3CC_Syringe_Poison_01, "CC_Syringe_Poison_01" },
            { EHM3ItemType::eHM3CC_HedgeCutter_01, "CC_HedgeCutter_01" },
            { EHM3ItemType::eHM3CC_PitchFork_01, "CC_Pitchfork_01" },
            { EHM3ItemType::eHM3CloseCombatSword, "CloseCombatSword" },
            { EHM3ItemType::eHM3CloseCombatChloroform, "CloseCombatChloroform" },
            { EHM3ItemType::eHM3CloseCombatGolfclub, "CloseCombatGolfclub" },
            { EHM3ItemType::eHM3CloseCombatKnife, "CloseCombatKitchenKnife" },
            { EHM3ItemType::eHM3Suitcase, "Suitcase" },
            { EHM3ItemType::eHM3Key, "KeyLocker" },
            { EHM3ItemType::eHM3Flowers, "FlowerBuket_01" },
            { EHM3ItemType::eHM3GroceryCrate, "CrateGrocery" },
            { EHM3ItemType::eHM3PartyInvitation, "PartyInvitation_01" },
            { EHM3ItemType::eHM3WineGlass, "WineGlass_01" },
            { EHM3ItemType::eHM3Sausages, "Sausages_01" },
            { EHM3ItemType::eHM3Ammo_45ACP_01, "Ammo_45ACP_01" },
            { EHM3ItemType::eHM3AmmoBlank, "Ammo_Blank" },
            { EHM3ItemType::eHM3Ammo_DartTranquilizer_01, "Ammo_Dart_Tranquilizer_01" },
            { EHM3ItemType::eHM3Ammo_Magnum_01, "Ammo_Magnum_01" },
            { EHM3ItemType::eHM3AmmoNailGun, "Ammo_NailGun_01" },
            { EHM3ItemType::eHM3Ammo_Pistol_01, "Ammo_Pistol_01" },
            { EHM3ItemType::eHM3Ammo_Prop_Mauser_01, "Ammo_Prop_Mauser_01" },
            { EHM3ItemType::eHM3Ammo_Rifle_01, "Ammo_Rifle_01" },
            { EHM3ItemType::eHM3Ammo_ShotGun_01, "Ammo_ShotGun_01" },
            { EHM3ItemType::eHM3Ammo_SMG_01, "Ammo_SMG_01" },
            { EHM3ItemType::eHM3Ammo_Sniper_01, "Ammo_Sniper_01" },
            { EHM3ItemType::eHM3AmmoCustomAssaultRifle, "Ammo_Custom_AssaultRifle_01" },
            { EHM3ItemType::eHM3AmmoCustomPistol, "Ammo_Custom_Pistol_01" },
            { EHM3ItemType::eHM3AmmoCustomShotGun, "Ammo_Custom_ShotGun_01" },
            { EHM3ItemType::eHM3AmmoCustomSniper, "Ammo_Custom_Sniper_01" },
            { EHM3ItemType::eHM3AmmoCustomSubMachineGun, "Ammo_Custom_SubMachineGun_01" },
            { EHM3ItemType::eHM3CameraTVCrew, "Scope_CameraTV_01" },
            { EHM3ItemType::eHM3Poison, "Item_Poison_01" },
            { EHM3ItemType::eHM3M01Cello, "Item_Cello_01" },
            { EHM3ItemType::eHM3M01CelloBow, "Item_Cello_Bow_01" },
            { EHM3ItemType::eHM3M03KeycardLightControlRoom, "Item_KeyCard_LightControl_01" },
            { EHM3ItemType::eHM3M03ToolBoxCarpenter, "Container_ToolBoxCarpenter_01" },
            { EHM3ItemType::eHM3M03WardropeTicket, "Item_WardrobeTicket_01" },
            { EHM3ItemType::eHM3M03NoteExecutioner, "Item_NoteExecutioner_01" },
            { EHM3ItemType::eHM3M07SuitcaseNegotiator, "Container_SuitcaseNegotiator_01" },
            { EHM3ItemType::eHM3M07ToolboxMechanic, "Container_ToolBoxMechanic_01" },
            { EHM3ItemType::eHM3M07BrothelPass, "Item_BrothelPass_01" },
            { EHM3ItemType::eHM3M11FoodGlassdrink, "Food_GlassDrink_01" },
            { EHM3ItemType::eHM3M11FlightcaseRifle, "Container_RifleFlightcase_01" },
            { EHM3ItemType::eHM3M11PartyMask, "Item_MaskParty_01" },
            { EHM3ItemType::eHM3M11KeyStorage, "Item_KeyStorage_01" },
            { EHM3ItemType::eHM3M11KitchenCrate, "Container_CrateKitchen_01" },
            { EHM3ItemType::eHM3M11KeyCardElevator, "Item_KeyCard_Elevator_01" },
            { EHM3ItemType::eHM3M11Cake, "Al07_Food" },
            { EHM3ItemType::eHM3M11KeyOffice, "Item_KeyM11Office" },
            { EHM3ItemType::eHM3ContainerBeerCase, "Container_BeerCase_01" },
            { EHM3ItemType::eHM3ContainerBibleHollow, "Container_BibleHollow_01" },
            { EHM3ItemType::eHM3ContainerBriefcase, "Container_Briefcase_01" },
            { EHM3ItemType::eHM3ContainerBriefcaseAfrikaner, "Container_Briefcase_Afrikaner_01" },
            { EHM3ItemType::eHM3ContainerBriefcaseDiamonds, "Container_Briefcase_Diamonds_01" },
            { EHM3ItemType::eHM3ContainerBriefcaseSheik, "Container_Briefcase_Sheik_01" },
            { EHM3ItemType::eHM3ContainerBucketChicken, "Container_BucketChicken_01" },
            { EHM3ItemType::eHM3ContainerCrateCatering, "Container_CrateCatering_01" },
            { EHM3ItemType::eHM3ContainerCrateDonuts, "Container_CrateDonuts_01" },
            { EHM3ItemType::eHM3ContainerGarbageCan, "Container_GarbageCan_01" },
            { EHM3ItemType::eHM3ContainerGiftBox, "Container_GiftBox_01" },
            { EHM3ItemType::eHM3ContainerRifle, "Container_Rifle_01" },
            { EHM3ItemType::eHM3FoodAphrodiciac, "Food_Aphrodiciac_01" },
            { EHM3ItemType::eHM3FoodBottleBaklava, "Food_BottleBaklava_01" },
            { EHM3ItemType::eHM3FoodBottleChampagne, "Food_BottleChampagne_01" },
            { EHM3ItemType::eHM3FoodBottleCleanser, "Food_BottleCleanser_01" },
            { EHM3ItemType::eHM3FoodBottleLiqour, "Food_BottleLiquor_01" },
            { EHM3ItemType::eHM3FoodBottleMartini, "Food_BottleMartini_01" },
            { EHM3ItemType::eHM3FoodBottlePort, "Food_BottlePort_01" },
            { EHM3ItemType::eHM3FoodBottleVodka, "Food_BottleVodka_01" },
            { EHM3ItemType::eHM3FoodBottleWhiskey, "Food_BottleWhiskey_01" },
            { EHM3ItemType::eHM3ContainerCake, "Container_Cake_01" },
            { EHM3ItemType::eHM3FoodChickenMorsel, "Food_ChickenMorsel_01" },
            { EHM3ItemType::eHM3FoodCupCoffee, "Food_CupCoffee_01" },
            { EHM3ItemType::eHM3FoodFlaskMetal, "Food_FlaskMetal_01" },
            { EHM3ItemType::eHM3FoodGlassLiquor, "Food_GlassLiquor_01" },
            { EHM3ItemType::eHM3FoodGlassMartini, "Food_GlassMartini_01" },
            { EHM3ItemType::eHM3FoodGlassWine, "Food_GlassWine_01" },
            { EHM3ItemType::eHM3FoodSantaPort, "Food_SantaPort_01" },
            { EHM3ItemType::eHM3ItemAdmissionPaper, "Item_AdmissionPaper_01" },
            { EHM3ItemType::eHM3ItemBalloon, "Item_Balloon_01" },
            { EHM3ItemType::eHM3ItemBoxClosed, "Item_BoxClosed_01" },
            { EHM3ItemType::eHM3ItemCartLaundry, "Item_CartLaundry_01" },
            { EHM3ItemType::eHM3ItemCoffeeMachine, "Item_CoffeeMachine_01" },
            { EHM3ItemType::eHM3ItemDiamonds, "Item_Diamonds_01" },
            { EHM3ItemType::eHM3ItemDNA, "Item_DNA_01" },
            { EHM3ItemType::eHM3ItemHatDevil, "Item_HatDevil_01" },
            { EHM3ItemType::eHM3ItemHatEngel, "Item_HatEngel_01" },
            { EHM3ItemType::eHM3ItemHatUnicorn, "Item_HatUnicorn_01" },
            { EHM3ItemType::eHM3ItemHeartVIP, "Item_HeartVIP_01" },
            { EHM3ItemType::eHM3ItemHelmetPilot, "Item_HelmetPilot_01" },
            { EHM3ItemType::eHM3ItemKeyBoat, "Item_KeyBoat_01" },
            { EHM3ItemType::eHM3ItemKeyCard, "Item_KeyCard_01" },
            { EHM3ItemType::eHM3ItemKeyCard_Cabin_01, "Item_KeyCard_Cabin_01" },
            { EHM3ItemType::eHM3ItemKeyCard_Cabin_02, "Item_KeyCard_Cabin_02" },
            { EHM3ItemType::eHM3ItemKeyCard_Cabin_03, "Item_KeyCard_Cabin_03" },
            { EHM3ItemType::eHM3ItemKeyCard_Cabin_Master_01, "Item_KeyCard_Cabin_Master_01" },
            { EHM3ItemType::eHM3ItemKeyCard_Universal, "Item_KeyCard_Universal_01" },
            { EHM3ItemType::eHM3ItemKeyCard_WH, "Item_KeyCard_WH_01" },
            { EHM3ItemType::eHM3ItemKeyCard_WH_Senior, "Item_KeyCard_WH_Senior_01" },
            { EHM3ItemType::eHM3ItemKeyClinicCells, "Item_KeyClinicCells_01" },
            { EHM3ItemType::eHM3ItemKeyClinicRoom, "Item_KeyClinicRoom_01" },
            { EHM3ItemType::eHM3ItemKeyCrew, "Item_KeyCrew_01" },
            { EHM3ItemType::eHM3ItemKeyHelicopter, "Item_KeyHelicopter_01" },
            { EHM3ItemType::eHM3ItemKeyMansion, "Item_KeyMansion_01" },
            { EHM3ItemType::eHM3ItemKeyPoolHouse, "Item_KeyPoolHouse_01" },
            { EHM3ItemType::eHM3ItemKeySedanLuxury, "Item_KeySedanLuxury_01" },
            { EHM3ItemType::eHM3ItemMoney, "Item_Money_01" },
            { EHM3ItemType::eHM3ItemNecklace, "Item_Necklace_01" },
            { EHM3ItemType::eHM3ItemNoteRifle, "Item_NoteRifle_01" },
            { EHM3ItemType::eHM3ItemParcelPictures, "Item_ParcelPictures_01" },
            { EHM3ItemType::eHM3ItemPartyInvitation, "Item_PartyInvitation_01" },
            { EHM3ItemType::eHM3ItemPassLounge, "Item_PassLounge_01" },
            { EHM3ItemType::eHM3ItemPassVIP, "Item_PassVIP_01" },
            { EHM3ItemType::eHM3ItemPhoneNumberSheik, "Item_PhoneNumberSheik_01" },
            { EHM3ItemType::eHM3ItemPhotoHitman, "Item_PhotoHitman_01" },
            { EHM3ItemType::eHM3ItemPhotosGovSon, "Item_PhotosGovSon_01" },
            { EHM3ItemType::eHM3ItemPoison, "Item_Poison_01" },
            { EHM3ItemType::eHM3ItemRadio, "Item_Radio_01" },
            { EHM3ItemType::eHM3ItemSack, "Item_Sack_01" },
            { EHM3ItemType::eHM3FoodSausage, "Food_Sausage_01" },
            { EHM3ItemType::eHM3ItemVestKevlar, "Item_VestKevlar_01" },
            { EHM3ItemType::eHM3ItemWalkieTalkie, "Item_WalkieTalkie_01" },
            { EHM3ItemType::eHM3ScopeCamera, "Scope_Camera_01" },
            { EHM3ItemType::eHM3ScopeCameraTV, "Scope_CameraTV_01" },
            { EHM3ItemType::eHM3EquipGogglesNV01, "Equip_Goggles_NV_01" },
            { EHM3ItemType::eHM3FoodBurger01, "Food_Burger_01" },
            { EHM3ItemType::eHM3FoodBurger02, "Food_Burger_02" },
            { EHM3ItemType::eHM3FoodBottleBeer01, "Food_BottleBeer_01" },
            { EHM3ItemType::eHM3ItemPyroFlaskExplodes01, "Item_PyroFlaskExplodes_01" },
            { EHM3ItemType::eHM3ItemPyroFlask01, "Item_PyroFlask_01" },
            { EHM3ItemType::eHM3ItemGunMauser01, "Item_GunMauser_01" },
            { EHM3ItemType::eHM3ItemKeyStorage01, "Item_KeyStorage_01" },
            { EHM3ItemType::eHM3GunSnubNosed01, "Gun_SnubNosed_01" },
            { EHM3ItemType::eHM3CCScythe01, "CC_Scythe_01" },
            { EHM3ItemType::eHM3CCKnifeStiletto01, "CC_Knife_Stiletto_01" },
            { EHM3ItemType::eHM3ItemPen, "Item_Pen_01" },
            { EHM3ItemType::eHM3ItemPaper, "Item_Paper_01" },
            { EHM3ItemType::eHM3ItemDishCloth, "Item_Dishcloth_01" },
            { EHM3ItemType::eHM3ItemDumbbell, "Item_Dumbbell_01" },
            { EHM3ItemType::eHMItemChipsMoney, "Item_ChipsMoney_01" },
            { EHM3ItemType::eHM3FoodBottleBeer, "Food_BottleBeer_01" },
            { EHM3ItemType::eHM3ContainerSuitcaseSniper01, "Container_SuitcaseSniper_01" },
            { EHM3ItemType::eHM3PhotoTarget1, "Item_PhotoTarget1_01" },
            { EHM3ItemType::eHM3PhotoTarget2, "Item_PhotoTarget2_01" },
            { EHM3ItemType::eHM3PhotoTarget3, "Item_PhotoTarget3_01" },
            { EHM3ItemType::eHM3CCSyringeHeartAttack, "CC_Syringe_HeartAttack_01" },
            { EHM3ItemType::eHM3CCKnifeMeatCleaver01, "CC_Knife_MeatCleaver_01" },
            { EHM3ItemType::eHM3CCHammer, "CC_Hammer_01" },
            { EHM3ItemType::eHM3CCShovel, "CC_Shovel_01" },
            { EHM3ItemType::eHM3FoodGlassWhisky, "Food_GlassWhisky_01" },
            { EHM3ItemType::eHM3ItemMobilePhone, "Item_MobilePhone_01" },
            { EHM3ItemType::eHM3ItemWaitressTray, "Item_WaitressTray_01" },
            { EHM3ItemType::eHM3ItemFishingPole, "Item_FishingPole_01" },
            { EHM3ItemType::eHM3ItemFishingRod, "Item_FishingRod_01" },
            { EHM3ItemType::eHM3ItemKeyCardUniversal02, "Item_KeyCard_Universal_02_01" },
            { EHM3ItemType::eHM3ItemSecurityDevice, "Item_SecurityDevice_01" },
            { EHM3ItemType::eHM3ItemBookSmallLeft, "Item_BookSmallLeft_01" },
            { EHM3ItemType::eHM3ItemScopeSLRCamera, "Item_Scope_SLRCamera_01" },
            { EHM3ItemType::eHM3ItemScopeSLRCameraFlash, "Item_Scope_SLRCameraFlash_01" },
            { EHM3ItemType::eHM3ItemVHSCassette, "Item_VHSCassette_01" },
            { EHM3ItemType::eHM3ItemGasolineCan, "Item_GasolineCan_01" },
            { EHM3ItemType::eHM3ItemTray, "Item_Tray_01" },
            { EHM3ItemType::eHM3ItemTrayBurger, "Container_TrayBurger_01" },
            { EHM3ItemType::eHM3ItemPlate, "Item_Plate_01" },
            { EHM3ItemType::eHM3ItemTeddyBear, "Item_TeddyBear_01" },
            { EHM3ItemType::eHM3ItemPoolCleanerNet, "Item_PoolCleanerNet_01" },
            { EHM3ItemType::eHM3ItemBalloonDeflated, "Item_BalloonDeflated_01" },
            { EHM3ItemType::eHM3ItemBalloonSmall, "Item_BalloonSmall_01" },
            { EHM3ItemType::eHM3ItemBalloonMedium, "Item_BalloonMedium_01" },
            { EHM3ItemType::eHM3ItemBalloonLarge, "Item_BalloonLarge_01" },
            { EHM3ItemType::eHM3ItemKeyCardHitman, "Item_KeyCard_Hitman_01" },
            { EHM3ItemType::eHM3ItemmedicineTray, "Item_medicineTray_01" },
            { EHM3ItemType::eHM3ItemVoiceRecorder, "Item_VoiceRecorder_01" },
            { EHM3ItemType::eHM3ItemKeyCardAfrikaaner, "Item_KeyCard_Afrikaner_01" },
            { EHM3ItemType::eHM3ItemMicrophone, "Item_Microphone_01" },
            { EHM3ItemType::eHM3ItemPaintbrush, "Item_Paintbrush_01" },
            { EHM3ItemType::eHM3ItemBook, "Item_Book_01" },
            { EHM3ItemType::eHM3ItemGiftPackage, "Item_GiftPackage_01" },
            { EHM3ItemType::eHM3Itemtrash, "Item_trash_01" },
            { EHM3ItemType::eHM3ItemFork, "Item_Fork_01" },
            { EHM3ItemType::eHM3ItemKnife, "Item_Knife_01" },
            { EHM3ItemType::eHM3CCScrewdriver01, "CC_Screwdriver_01" },
            { EHM3ItemType::eHM3ItemKeyCard_Rehab, "Item_KeyCard_Rehab_01" },
            { EHM3ItemType::eHM3FoodBottleEmpty, "Food_BottleEmpty_01" },
            { EHM3ItemType::eHM3FoodBottleCoke, "Food_BottleCoke_01" },
            { EHM3ItemType::eHM3ItemWhitehousePaper, "Item_WhitehousePaper_01" },
            { EHM3ItemType::eHM3ItemCroupierRake, "Item_CroupierRake_01" },
            { EHM3ItemType::eHM3ItemChipsMoneyA, "Item_ChipsMoneyA_01" },
            { EHM3ItemType::eHM3ItemShoes, "Item_Shoes_01" },
            { EHM3ItemType::eHM3ItemSponge, "Item_Sponge_01" },
            { EHM3ItemType::eHM3ItemThreeCards, "Item_ThreeCards_01" },
            { EHM3ItemType::eHM3ItemCardFan, "Item_CardFan_01" },
            { EHM3ItemType::eHM3Itemcard, "Item_card_01" },
            { EHM3ItemType::eHM3ItemKeyCard_RatClub, "Item_KeyCard_RatClub_01" },
            { EHM3ItemType::eHM3ItemCigarette, "Item_Cigarette_01" },
            { EHM3ItemType::eHM3ItemKeyCardScientist, "Item_KeyCard_Scientist_01" },
            { EHM3ItemType::eHM3ItemKeyCardSamantha, "Item_KeyCard_Samantha_01" },
            { EHM3ItemType::eHM3ItemKeyCardUniversal_03, "Item_KeyCard_Universal_03" },
            { EHM3ItemType::eHM3ItemKeyCardUniversal_02, "Item_KeyCard_Universal_02" },
            { EHM3ItemType::eHM3ItemKeyCardChemist_01, "Item_KeyCard_Chemist_01" },
            { EHM3ItemType::eHM3ItemNoteRoomNumber707, "Item_NoteRoomNumber707_01" },
            { EHM3ItemType::eHM3ItemBottleFireLighter, "Item_BottleFireLighter_01" },
            { EHM3ItemType::eHM3ItemBottleEther, "Item_BottleEther_01" },
            { EHM3ItemType::eHM3ItemScopeVideoCamera, "Item_Scope_Video_Camera_01" },
            { EHM3ItemType::eHM3ItemQuarterBackPhoto, "Item_QuarterBackPhoto_01" },
            { EHM3ItemType::eHM3Itempillow, "Item_pillow_01" },
            { EHM3ItemType::eHM3ItemCigarUnLit, "Item_CigarUnLit_01" },
            { EHM3ItemType::eHM3ItemCigar, "Item_Cigar_01" },
            { EHM3ItemType::eHM3ItemSpoon, "Item_Spoon_01" },
            { EHM3ItemType::eHM3ItemThong, "Item_Thong_01" },
            { EHM3ItemType::eHM3ContainerCrateBakingSoda, "Container_CrateBakingSoda_01" },
            { EHM3ItemType::eHM3PainKillers, "Equip_PainKillers_01" },
            { EHM3ItemType::eHM3Binoculars, "Equip_Binoculars_01" },
            { EHM3ItemType::eHM3Adrenaline, "Equip_Adrenaline_01" },
            { EHM3ItemType::eHM3FoodGlassEmpty, "Food_GlassEmpty_01" },
            { EHM3ItemType::eHM3FoodChickenLeg, "Food_ChickenLeg_01" },
            { EHM3ItemType::eHM3ItemCigaretteLit, "Item_CigaretteLit_01" },
            { EHM3ItemType::eHM3ItemBookAnimated, "Item_BookAnimated_01" },
            { EHM3ItemType::eHM3ItemBubblePack, "Item_BubblePack_01" },
            { EHM3ItemType::eHM3Pc09H_WeddingCake, "Pc09H_WeddingCake" },
            { EHM3ItemType::eHM3EquipSuitcaseFoilPadded, "Equip_Suitcase_FoilPadded_01" },
            { EHM3ItemType::eHM3EquipLockPick03, "Equip_LockPick_03" },
            { EHM3ItemType::eHM3EquipLockPick02, "Equip_LockPick_02" },
            { EHM3ItemType::eHM3EquipLockPick01, "Equip_LockPick_01" },
            { EHM3ItemType::eHM3EquipKevlarVest01, "Equip_KevlarVest_01" },
            { EHM3ItemType::eHM3EquipFlakVest02, "Equip_FlakVest_02" },
            { EHM3ItemType::eHM3EquipFlakVest01, "Equip_FlakVest_01" },
            { EHM3ItemType::eHM3EquipBinocularsNV01, "Equip_Binoculars_NV_01" },
            { EHM3ItemType::eHM3EquipBinoculars02, "Equip_Binoculars_02" },
            { EHM3ItemType::eHM3ItemCelloBow01, "Item_CelloBow_01" },
            { EHM3ItemType::eHM3ContainerCakeHack01, "Container_CakeHack_01" },
            { EHM3ItemType::eHM3CCSwordCane02, "CC_Sword_Cane_02" },
            { EHM3ItemType::eHM3FoodChickenLeg01, "Food_ChickenLeg_01" },
            { EHM3ItemType::eHM3FoodGlassTim01, "Food_GlassTim_01" },
            { EHM3ItemType::eHM3ItemBookAnimated01, "Item_BookAnimated_01" },
            { EHM3ItemType::eHM3ItemBubblePack01, "Item_BubblePack_01" },
            { EHM3ItemType::eHM3Pc09HWeddingCake, "Pc09H_WeddingCake" },
            { EHM3ItemType::eHM3ItemWineBarrel01, "Item_WineBarrel_01" },
            { EHM3ItemType::eHM3ContainerSuitcaseWoman, "Container_SuitcaseWoman_01" },
            { EHM3ItemType::eHM3ItemKeyCardUniversal_04, "Item_KeyCard_Universal_04" },
            { EHM3ItemType::eHM3ItemItemTVRemote, "Item_TVRemote_01" },
            { EHM3ItemType::eHM3ItemItemDonut, "Item_Donut_01" },
            { EHM3ItemType::eHM3ItemRifleEnfield, "Item_Rifle_Enfield_01" },
            { EHM3ItemType::eHM3Food_SteakCooked, "Food_SteakCooked_01" },
            { EHM3ItemType::eHM3ItemSteakRaw, "Item_SteakRaw_01" },
            { EHM3ItemType::eHM3ItemSteakJoint, "Item_SteakJoint_01" },
            { EHM3ItemType::eHM3ItemCoin, "Item_Coin_01" },
            { EHM3ItemType::eHM3ItemElephant, "Item_Elephant" },
            { EHM3ItemType::eHM3ItemPhotoBoy, "Item_PhotoBoy_01" },
            { EHM3ItemType::eHM3ItemKeyCardRoom704, "Item_KeyCard_Room_704" },
            { EHM3ItemType::eHM3ItemKeyCardRoom705, "Item_KeyCard_Room_705" },
            { EHM3ItemType::eHM3ItemKeyCardRoom706, "Item_KeyCard_Room_706" },
            { EHM3ItemType::eHM3ItemKeyCardRoom708, "Item_KeyCard_Room_708" },
            { EHM3ItemType::eHM3ItemKeyCardRoom801, "Item_KeyCard_Room_801" },
            { EHM3ItemType::eHM3ItemKeyCardRoom807, "Item_KeyCard_Room_807" },
            { EHM3ItemType::eHM3ContainerWaitressTray, "Container_WaitressTray_01" },
            { EHM3ItemType::eHM3ItemAirgunDarts, "Item_Dart_Tranquilizer_01" },
            { EHM3ItemType::eHM3ItemNailFile, "Item_NailFile_01" },
            { EHM3ItemType::eHM3CC_Syringe_Antidote_01, "CC_Syringe_Antidote_01" },
            { EHM3ItemType::eHM3CC_BaseballBat_Used_01, "CC_BaseballBat_Used_01" },
            { EHM3ItemType::eHM3CC_FireExtinguisher_Used_01, "CC_FireExtinguisher_Used_01" },
            { EHM3ItemType::eHM3CC_Shovel_Used_01, "CC_Shovel_Used_01" },
            { EHM3ItemType::eHM3CC_Hammer_Used_01, "CC_Hammer_Used_01" },
            { EHM3ItemType::eHM3CC_HedgeCutter_Used_01, "CC_HedgeCutter_Used_01" },
            { EHM3ItemType::eHM3CC_PickAxe_Used_01, "CC_PickAxe_Used_01" },
            { EHM3ItemType::eHM3CC_Screwdriver_Used_01, "CC_Screwdriver_Used_01" },
            { EHM3ItemType::eHM3Ammo_Magnum_02, "Ammo_Magnum_02" },
        };
    }

    // PC 0x64B470
    ZHM3ItemTemplate* ZHM3ItemTool::GetHM3ItemTemplate(Glacier::ZItem* pItem)
    {
        if (pItem == nullptr || (pItem->GetObjectId() & ZHM3Item::m_Mask) != ZHM3Item::m_Id)
            return nullptr;

        Glacier::ZItemTemplate* pTemplate = pItem->GetItemTemplate();
        if (pTemplate != nullptr &&
            (pTemplate->GetObjectId() & ZHM3ItemTemplate::m_Mask) == ZHM3ItemTemplate::m_Id)
        {
            return static_cast<ZHM3ItemTemplate*>(pTemplate);
        }

        return nullptr;
    }

    // PC 0x64B4D0 (tail-calls the shared body at PC 0x518D00)
    ZHM3ItemTemplateWeapon* ZHM3ItemTool::GetHM3ItemTemplateWeapon(Glacier::ZItem* pItem)
    {
        if (pItem == nullptr ||
            (pItem->GetObjectId() & ZHM3ItemWeapon::m_Mask) != ZHM3ItemWeapon::m_Id)
            return nullptr;

        Glacier::ZItemTemplate* pTemplate = pItem->GetItemTemplate();
        ZASSERT(pTemplate != nullptr);
        ZASSERT((pTemplate->GetObjectId() & ZHM3ItemTemplateWeapon::m_Mask) == ZHM3ItemTemplateWeapon::m_Id);

        return static_cast<ZHM3ItemTemplateWeapon*>(pTemplate);
    }

    // PC 0x64D190
    EHM3ItemType ZHM3ItemTool::GetHM3Type(Glacier::ZItem* pItem)
    {
        ZHM3ItemTemplate* pTemplate = GetHM3ItemTemplate(pItem);
        if (pTemplate != nullptr)
            return pTemplate->GetHM3ItemType();

        if (pItem == nullptr)
            return EHM3ItemType::eHM3NoType;

        if ((pItem->GetObjectId() & ZHM3ItemWeapon::m_Mask) == ZHM3ItemWeapon::m_Id)
        {
            ZHM3ItemTemplateWeapon* pWeaponTemplate = GetHM3ItemTemplateWeapon(pItem);
            if (pWeaponTemplate != nullptr)
                return pWeaponTemplate->GetHM3ItemType();
        }

        if ((pItem->GetObjectId() & ZHM3ItemContainer::m_Mask) == ZHM3ItemContainer::m_Id)
            return static_cast<ZHM3ItemContainer*>(pItem)->GetHM3ItemType();

        return EHM3ItemType::eHM3NoType;
    }

    // PC 0x649400
    EHM3ItemType ZHM3ItemTool::GetHM3Type(const char* pszName)
    {
        for (int i = 0; i < static_cast<int>(EHM3ItemType::eHM3NumItems); ++i)
        {
            if (Glacier::strcasecmp(pszName, g_aHM3ItemTypeNameList[i].pszName) == 0)
                return g_aHM3ItemTypeNameList[i].type;
        }

        return EHM3ItemType::eHM3NoType;
    }

    // PC 0x6493D0
    const char* ZHM3ItemTool::GetHM3ItemName(EHM3ItemType eItemType)
    {
        ZASSERT(static_cast<int>(eItemType) < static_cast<int>(EHM3ItemType::eHM3NumItems));

        for (int i = 0; i < static_cast<int>(EHM3ItemType::eHM3NumItems); ++i)
        {
            if (g_aHM3ItemTypeNameList[i].type == eItemType)
                return g_aHM3ItemTypeNameList[i].pszName;
        }

        return nullptr;
    }

    // PC 0x649440
    int ZHM3ItemTool::GetHM3OSDSpriteID(EHM3ItemType eItemType)
    {
        const int lType = static_cast<int>(eItemType);

        if (lType > 40)
        {
            if (lType > 186)
            {
                switch (lType)
                {
                case 188: // eHM3CCKnifeStiletto01
                case 202: // eHM3CCShovel
                case 313: // eHM3CC_BaseballBat_Used_01
                case 314: // eHM3CC_FireExtinguisher_Used_01
                case 315: // eHM3CC_Shovel_Used_01
                case 317: // eHM3CC_HedgeCutter_Used_01
                case 318: // eHM3CC_PickAxe_Used_01
                    return 9;
                default:
                    return 10;
                }
            }

            if (lType == 186) // eHM3GunSnubNosed01
                return 2;

            switch (lType)
            {
            case 42: // eHM3CC_Knife_Kitchen_01
            case 44: // eHM3CC_NailFile_01
            case 45: // eHM3CC_Pens_01
            case 46: // eHM3CC_PickAxe_01
            case 47: // eHM3CC_StunGun_01
            case 48: // eHM3CC_Sword_Cane_01
            case 50: // eHM3CC_Sword_Marine_01
            case 51: // eHM3CC_Syringe_Anastetic_01
            case 53: // eHM3CC_Syringe_Laxative_01
            case 54: // eHM3CC_Syringe_Poison_01
            case 55: // eHM3CC_HedgeCutter_01
            case 56: // eHM3CC_PitchFork_01
            case 70: // eHM3Ammo_DartTranquilizer_01
                return 9;
            default:
                return 10;
            }
        }

        if (lType >= 37)
            return 9;

        int lSprite = 10;
        switch (lType)
        {
        case 2:  // eHM3CustomHardballer
        case 3:  // eHM3CustomHardballerPickup
        case 10: // eHM3Gun_Browning_01
        case 11: // eHM3Gun_DesertEagle_01
        case 12: // eHM3Gun_HKusp_01
        case 13: // eHM3Gun_HKusp_Silenced_01
        case 14: // eHM3Gun_Mauser_01
        case 15: // eHM3Gun_PropMauser_01
        case 17: // eHM3Gun_SixShooter_01
        case 18: // eHM3Gun_StubNosed_01
        case 19: // eHM3Gun_Taurus_01
        case 20: // eHM3Gun_Albino_01
            return 2;
        case 4:  // eHM3CustomSniper
        case 29: // eHM3SniperRifle_Browning_01
        case 30: // eHM3SniperRifle_Dragunov_01
        case 31: // eHM3SniperRifle_SakoTRG_01
            lSprite = 3;
            break;
        case 5:  // eHM3CustomSMG
        case 32: // eHM3SMG_FamaeSAF_01
        case 33: // eHM3SMG_MP5_01
        case 34: // eHM3SMG_MP7_01
        case 35: // eHM3SMG_RugerMP9_01
        case 36: // eHM3SMG_SteyrTMP_01
            lSprite = 1;
            break;
        case 6:  // eHM3CustomMG
        case 23: // eHM3Rifle_Enfield_01
        case 24: // eHM3Rifle_FN2000_01
        case 25: // eHM3Rifle_M14_01
        case 27: // eHM3Rifle_SG552_01
            lSprite = 0;
            break;
        case 7:  // eHM3CustomSG
        case 26: // eHM3Rifle_Remington_01
            lSprite = 4;
            break;
        case 16: // eHM3Gun_NailGun_01
            return 9;
        default:
            lSprite = 10;
            break;
        }

        return lSprite;
    }

    // PC 0x6495C0
    bool ZHM3ItemTool::SpriteSizeLarge(EHM3ItemType eItemType)
    {
        switch (static_cast<int>(eItemType))
        {
        case 0xB:  // eHM3Gun_DesertEagle_01
        case 0xE:  // eHM3Gun_Mauser_01
        case 0x10: // eHM3Gun_NailGun_01
        case 0x20: // eHM3SMG_FamaeSAF_01
        case 0x24: // eHM3SMG_SteyrTMP_01
            return true;
        default:
            return false;
        }
    }

    // PC 0x649630
    EHM3ItemCategory ZHM3ItemTool::GetItemCategory(EHM3ItemType eItemType)
    {
        const int lType = static_cast<int>(eItemType);

        if (lType <= 31)
        {
            if (lType < 22)
            {
                EHM3ItemCategory eCategory = EHM3ItemCategory::NONE_SPECIFIED;
                switch (lType)
                {
                case 2:  // eHM3CustomHardballer
                case 3:  // eHM3CustomHardballerPickup
                case 10: // eHM3Gun_Browning_01
                case 11: // eHM3Gun_DesertEagle_01
                case 12: // eHM3Gun_HKusp_01
                case 13: // eHM3Gun_HKusp_Silenced_01
                case 14: // eHM3Gun_Mauser_01
                case 15: // eHM3Gun_PropMauser_01
                case 16: // eHM3Gun_NailGun_01
                case 17: // eHM3Gun_SixShooter_01
                case 18: // eHM3Gun_StubNosed_01
                case 19: // eHM3Gun_Taurus_01
                case 20: // eHM3Gun_Albino_01
                    eCategory = EHM3ItemCategory::GUN;
                    break;
                case 4:  // eHM3CustomSniper
                case 6:  // eHM3CustomMG
                case 7:  // eHM3CustomSG
                    return EHM3ItemCategory::NONCONCEALABLE;
                case 5:  // eHM3CustomSMG
                    eCategory = EHM3ItemCategory::SMG;
                    break;
                case 8:  // eHM3RemoteBomb
                    eCategory = EHM3ItemCategory::BOMB;
                    break;
                default:
                    eCategory = EHM3ItemCategory::NONE_SPECIFIED;
                    break;
                }

                return eCategory;
            }

            return EHM3ItemCategory::NONCONCEALABLE;
        }

        if (lType > 57)
        {
            if (lType > 200)
            {
                switch (lType)
                {
                case 202: // eHM3CCShovel
                case 313: // eHM3CC_BaseballBat_Used_01
                case 314: // eHM3CC_FireExtinguisher_Used_01
                case 315: // eHM3CC_Shovel_Used_01
                case 317: // eHM3CC_HedgeCutter_Used_01
                case 318: // eHM3CC_PickAxe_Used_01
                    return EHM3ItemCategory::NONCONCEALABLE;
                case 234: // eHM3ItemKnife
                case 235: // eHM3CCScrewdriver01
                case 319: // eHM3CC_Screwdriver_Used_01
                    return EHM3ItemCategory::KNIFE;
                default:
                    return EHM3ItemCategory::NONE_SPECIFIED;
                }
            }

            if (lType == 200) // eHM3CCKnifeMeatCleaver01
                return EHM3ItemCategory::KNIFE;

            switch (lType)
            {
            case 58:  // eHM3CloseCombatChloroform
            case 199: // eHM3CCSyringeHeartAttack
                return EHM3ItemCategory::SYRINGE;
            case 59:  // eHM3CloseCombatGolfclub
                return EHM3ItemCategory::NONCONCEALABLE;
            case 60:  // eHM3CloseCombatKnife
            case 188: // eHM3CCKnifeStiletto01
                return EHM3ItemCategory::KNIFE;
            default:
                return EHM3ItemCategory::NONE_SPECIFIED;
            }
        }

        if (lType == 57) // eHM3CloseCombatSword
            return EHM3ItemCategory::NONCONCEALABLE;

        EHM3ItemCategory eCategory = EHM3ItemCategory::NONE_SPECIFIED;
        switch (lType)
        {
        case 32: // eHM3SMG_FamaeSAF_01
        case 33: // eHM3SMG_MP5_01
        case 34: // eHM3SMG_MP7_01
        case 35: // eHM3SMG_RugerMP9_01
        case 36: // eHM3SMG_SteyrTMP_01
            eCategory = EHM3ItemCategory::SMG;
            break;
        case 37: // eHM3CC_BaseballBat_01
        case 39: // eHM3CC_FireExtinguisher_01
        case 46: // eHM3CC_PickAxe_01
        case 48: // eHM3CC_Sword_Cane_01
        case 50: // eHM3CC_Sword_Marine_01
        case 55: // eHM3CC_HedgeCutter_01
            return EHM3ItemCategory::NONCONCEALABLE;
        case 38: // eHM3CC_FiberWire_01
            eCategory = EHM3ItemCategory::FIBER_WIRE;
            break;
        case 40: // eHM3CC_Knife_Bowie_01
        case 41: // eHM3CC_Knife_Butterfly_01
        case 42: // eHM3CC_Knife_Kitchen_01
        case 43: // eHM3CC_Knife_Throwing_01
            return EHM3ItemCategory::KNIFE;
        case 47: // eHM3CC_StunGun_01
            eCategory = EHM3ItemCategory::GUN;
            break;
        case 51: // eHM3CC_Syringe_Anastetic_01
        case 53: // eHM3CC_Syringe_Laxative_01
        case 54: // eHM3CC_Syringe_Poison_01
            eCategory = EHM3ItemCategory::SYRINGE;
            break;
        default:
            return EHM3ItemCategory::NONE_SPECIFIED;
        }

        return eCategory;
    }

    // PC 0x649A30
    int ZHM3ItemTool::MaxNumOfItem(EHM3ItemCategory eCategory)
    {
        switch (eCategory)
        {
        case EHM3ItemCategory::GUN:
        case EHM3ItemCategory::SMG:
        case EHM3ItemCategory::NONCONCEALABLE:
        case EHM3ItemCategory::KNIFE:
        case EHM3ItemCategory::BOMB:
        case EHM3ItemCategory::FIBER_WIRE:
            return 1;
        case EHM3ItemCategory::SYRINGE:
            return 2;
        default:
            return 0;
        }
    }
}
