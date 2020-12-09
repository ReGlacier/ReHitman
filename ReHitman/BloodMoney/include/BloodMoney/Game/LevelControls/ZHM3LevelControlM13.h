#pragma once

#include <Glacier/GlacierFWD.h>
#include <BloodMoney/Game/ZPodiumController.h>

namespace Hitman::BloodMoney
{
    using Glacier::ISerializerStream;
    using Glacier::IInputSerializerStream;
    using Glacier::IOutputSerializerStream;

    // Type forwardings for class ZHM3LevelControlM13
    enum class ESecurityZone;
    class ZItem;
    class ZEventBase;
    class ZItemWeapon;
    class ZHM3ItemWeapon;
    class ZHM3Item;
    class ZHM3ClothBundle;
    class ZBaseGeom;
    class ZCheckPoBuffer;
    class ZHM3HmAs;
    class ZItemContainer;
    class ZAction;
    class ZHM3Actor;
    class ZGEOM;
    class ZROUTCLASSINFO;
    class ZHM3ItemBomb;
    enum class EHM3ItemType;

    class ZCheckPointBuffer{};

    // Class definition ZHM3LevelControlM13
    class ZHM3LevelControlM13 {
    public:
        virtual void Release_ZHM3LevelControlM13(bool); //#0000 at 00478374 org ZHM3LevelControlM13::~ZHM3LevelControlM13()
        virtual void PreSave(ISerializerStream &); //#0001 at 0043E228 org ZSerializable::PreSave(ISerializerStream &)
        virtual void PostSave(ISerializerStream &); //#0002 at 003FF584 org ZHM3LevelControlM13::PostSave(ISerializerStream &)
        virtual void PreLoad(ISerializerStream &); //#0003 at 0043E238 org ZSerializable::PreLoad(ISerializerStream &)
        virtual void PostLoad(ISerializerStream &); //#0004 at 003FF514 org ZHM3LevelControlM13::PostLoad(ISerializerStream &)
        virtual void PostProcess(unsigned int,unsigned int); //#0005 at 0043E248 org ZSerializable::PostProcess(unsigned int,unsigned int)
        virtual void LoadSave(ISerializerStream &,bool); //#0006 at 003DC7F0 org ZHM3LevelControl::LoadSave(ISerializerStream &,bool)
        virtual void LoadObject(IInputSerializerStream &); //#0007 at 003DC848 org ZHM3LevelControl::LoadObject(IInputSerializerStream &)
        virtual void SaveObject(IOutputSerializerStream &); //#0008 at 003DC9BC org ZHM3LevelControl::SaveObject(IOutputSerializerStream &)
        virtual void ExchangeObject(ISerializerStream &); //#0009 at 0043CC1C org ZSerializable::ExchangeObject(ISerializerStream &)
        virtual void SetToDefault(); //#0010 at 0043C568 org ZSerializable::SetToDefault(void)
        virtual void GetTypeID(); //#0011 at 0043F270 org RTP::cBase::GetTypeID(void)
        virtual void GetProperties(); //#0012 at 00478414 org ZHM3LevelControlM13::GetProperties(void)const
        virtual void GetEventPriority(); //#0013 at 00475DF0 org ZHM3LevelControl::GetEventPriority(void)const
        virtual void Init(); //#0014 at 003FE4AC org ZHM3LevelControlM13::Init(void)
        virtual void Init2(); //#0015 at 003FE59C org ZHM3LevelControlM13::Init2(void)
        virtual void PostInit(); //#0016 at 0043FA28 org ZEventBase::PostInit(void)
        virtual void PostInit2(); //#0017 at 003FE700 org ZHM3LevelControlM13::PostInit2(void)
        virtual void CopyData(ZEventBase *); //#0018 at 003DC258 org ZHM3LevelControl::CopyData(ZEventBase *)
        virtual void EventName(); //#0019 at 0043FAB0 org ZBaseConRout::EventName(void)
        virtual void ExpandBounds(float *,float *,float *,ZBaseGeom *); //#0020 at 0043FA48 org ZEventBase::ExpandBounds(float *,float *,float *,ZBaseGeom *)
        virtual void PreSaveGame(); //#0021 at 00174824 org ZEventBase::PreSaveGame(void)
        virtual void RegisterInstance(unsigned int); //#0022 at 0043FA50 org ZEventBase::RegisterInstance(unsigned int)
        virtual void CheckPointSave(ZCheckPointBuffer &); //#0023 at 0043FA58 org ZEventBase::CheckPointSave(ZCheckPointBuffer &)
        virtual void CheckPointLoad(ZCheckPointBuffer &); //#0024 at 0043FA60 org ZEventBase::CheckPointLoad(ZCheckPointBuffer &)
        virtual void Reset(); //#0025 at 00174C0C org ZEventBase::Reset(void)
        virtual void TimeUpdate(); //#0026 at 0043FA68 org ZEventBase::TimeUpdate(void)
        virtual void FrameUpdate(); //#0027 at 003FEE80 org ZHM3LevelControlM13::FrameUpdate(void)
        virtual void Command(unsigned short,void *); //#0028 at 003FEC28 org ZHM3LevelControlM13::Command(unsigned short,void *)
        virtual void DoEvent(int,int,void *); //#0029 at 0043FAD0 org ZBaseConRout::DoEvent(int,int,void *)
        virtual void End(); //#0030 at 003FEAEC org ZHM3LevelControlM13::End(void)
        virtual void EditorCommand(unsigned short,void *); //#0031 at 0043FA90 org ZEventBase::EditorCommand(unsigned short,void *)
        virtual void Remove(); //#0032 at 00174BF0 org ZEventBase::Remove(void)
        virtual void SchedUpdate(); //#0033 at 0043FA98 org ZEventBase::SchedUpdate(void)
        virtual void InitBaseConRout(ZROUTCLASSINFO *); //#0034 at 0043FAA0 org ZBaseConRout::InitBaseConRout(ZROUTCLASSINFO *)
        virtual void UnknownCommand(unsigned short,void *); //#0035 at 0043FAC8 org ZBaseConRout::UnknownCommand(unsigned short,void *)
        virtual void MusicInit(); //#0036 at 003FEA30 org ZHM3LevelControlM13::MusicInit(void)
        virtual void CharacterKilled(unsigned int); //#0037 at 003FEB34 org ZHM3LevelControlM13::CharacterKilled(unsigned int)
        virtual void CharacterHarmed(unsigned int); //#0038 at 003DCDDC org ZHM3LevelControl::CharacterHarmed(unsigned int)
        virtual void HitmanHideWeapon(); //#0039 at 003DE708 org ZHM3LevelControl::HitmanHideWeapon(void)
        virtual void HitmanDragBody(ZGEOM *); //#0040 at 00475DF8 org ZHM3LevelControl::HitmanDragBody(ZGEOM *)
        virtual void HitmanDropDeadBody(unsigned int,int); //#0041 at 003FEC14 org ZHM3LevelControlM13::HitmanDropDeadBody(unsigned int,int)
        virtual void HitmanPickupItem(ZItem *); //#0042 at 003FEC04 org ZHM3LevelControlM13::HitmanPickupItem(ZItem *)
        virtual void HitmanDropItem(ZItem *); //#0043 at 003FEC0C org ZHM3LevelControlM13::HitmanDropItem(ZItem *)
        virtual void HitmanUseItem(ZItem *); //#0044 at 003DD0EC org ZHM3LevelControl::HitmanUseItem(ZItem *)
        virtual void HitmanThrowItem(ZItem *); //#0045 at 003DCFE0 org ZHM3LevelControl::HitmanThrowItem(ZItem *)
        virtual void HitmanGetCurrentUBAnim(unsigned int *); //#0046 at 003DD0F4 org ZHM3LevelControl::HitmanGetCurrentUBAnim(unsigned int *)
        virtual void HitmanDied(); //#0047 at 003DD104 org ZHM3LevelControl::HitmanDied(void)
        virtual void HitmanNearDeath(); //#0048 at 003DD0FC org ZHM3LevelControl::HitmanNearDeath(void)
        virtual void OnHitmanMayAttackCloseCombat(ZHM3ItemWeapon *,ZHM3Actor *); //#0049 at 00475E08 org ZHM3LevelControl::OnHitmanMayAttackCloseCombat(ZHM3ItemWeapon *,ZHM3Actor *)
        virtual void OnHitmanMayStrangulateTarget(ZHM3Actor *); //#0050 at 00475E10 org ZHM3LevelControl::OnHitmanMayStrangulateTarget(ZHM3Actor *)
        virtual void OnHitmanMayDisarmTarget(ZHM3Actor *); //#0051 at 00475E18 org ZHM3LevelControl::OnHitmanMayDisarmTarget(ZHM3Actor *)
        virtual void OnHitmanTakeHumanShield(ZHM3Actor *); //#0052 at 00475E20 org ZHM3LevelControl::OnHitmanTakeHumanShield(ZHM3Actor *)
        virtual void OnHitmanUnpackSniper(); //#0053 at 00475E28 org ZHM3LevelControl::OnHitmanUnpackSniper(void)
        virtual void OnHitmanCloseInventory(); //#0054 at 00475E30 org ZHM3LevelControl::OnHitmanCloseInventory(void)
        virtual void OnHitmanChangeClothes(ZHM3HmAs *,ZGEOM *); //#0055 at 003DD1A0 org ZHM3LevelControl::OnHitmanChangeClothes(ZHM3HmAs *,ZGEOM *)
        virtual void OnDropHitmanClothBundle(ZHM3ClothBundle *); //#0056 at 00475E38 org ZHM3LevelControl::OnDropHitmanClothBundle(ZHM3ClothBundle *)
        virtual void BombExploded(ZHM3ItemBomb *); //#0057 at 00475E40 org ZHM3LevelControl::BombExploded(ZHM3ItemBomb *)
        virtual void OnBombPlaced(ZHM3ItemBomb *); //#0058 at 00475E48 org ZHM3LevelControl::OnBombPlaced(ZHM3ItemBomb *)
        virtual void BombAddedToHitman(ZItem *); //#0059 at 00475E50 org ZHM3LevelControl::BombAddedToHitman(ZItem *)
        virtual void MissionFailed(char const*); //#0060 at 003DCEE4 org ZHM3LevelControl::MissionFailed(char const*)
        virtual void CalculateExtraStats(); //#0061 at 003DB434 org ZHM3LevelControl::CalculateExtraStats(void)
        virtual void MissionCompleted(); //#0062 at 003DB340 org ZHM3LevelControl::MissionCompleted(void)
        virtual void MissionCompleted2(); //#0063 at 003FEB00 org ZHM3LevelControlM13::MissionCompleted2(void)
        virtual void TerminateScene(); //#0064 at 003DCFC8 org ZHM3LevelControl::TerminateScene(void)
        virtual void CompleteObjective(int,bool); //#0065 at 003DCFE8 org ZHM3LevelControl::CompleteObjective(int,bool)
        virtual void UnCompleteObjective(int); //#0066 at 003DD00C org ZHM3LevelControl::UnCompleteObjective(int)
        virtual void HideObjective(int); //#0067 at 003DD02C org ZHM3LevelControl::HideObjective(int)
        virtual void UnHideObjective(int); //#0068 at 003DD04C org ZHM3LevelControl::UnHideObjective(int)
        virtual void IsHitmanAllowedHere(ESecurityZone,ZHM3HmAs *,ZGEOM *,ZItem *,ZItem *); //#0069 at 00475E58 org ZHM3LevelControl::IsHitmanAllowedHere(ESecurityZone,ZHM3HmAs *,ZGEOM *,ZItem *,ZItem *)
        virtual void IsIgnoredWeapon(ZItemWeapon *); //#0070 at 00475E60 org ZHM3LevelControl::IsIgnoredWeapon(ZItemWeapon *)
        virtual void IsWeaponFoundAtHitman(bool &); //#0071 at 00475E68 org ZHM3LevelControl::IsWeaponFoundAtHitman(bool &)
        virtual void TransferHitmanWeaponsToActor(unsigned int,bool &); //#0072 at 00475E74 org ZHM3LevelControl::TransferHitmanWeaponsToActor(unsigned int,bool &)
        virtual void CanHMSyringeTarget(ZHM3Actor *,ZHM3ItemWeapon *); //#0073 at 00475E7C org ZHM3LevelControl::CanHMSyringeTarget(ZHM3Actor *,ZHM3ItemWeapon *)const
        virtual void CanHMSyringeItem(ZHM3Item *,ZHM3ItemWeapon *); //#0074 at 00475E84 org ZHM3LevelControl::CanHMSyringeItem(ZHM3Item *,ZHM3ItemWeapon *)const
        virtual void CanHitmanFiberWireTarget(ZHM3Actor *); //#0075 at 00475E8C org ZHM3LevelControl::CanHitmanFiberWireTarget(ZHM3Actor *)const
        virtual void CanHMPutItemIntoElevator(ZGEOM *); //#0076 at 00475E94 org ZHM3LevelControl::CanHMPutItemIntoElevator(ZGEOM *)const
        virtual void OnObjectExplode(ZGEOM *); //#0077 at 00475E9C org ZHM3LevelControl::OnObjectExplode(ZGEOM *)
        virtual void CanHMOperateElevatorButton(ZAction *,bool &); //#0078 at 00475EA4 org ZHM3LevelControl::CanHMOperateElevatorButton(ZAction *,bool &)
        virtual void OnActorDropItemsInHand(ZHM3Actor *); //#0079 at 00475EB0 org ZHM3LevelControl::OnActorDropItemsInHand(ZHM3Actor *)
        virtual void DisguiseUsed(unsigned int); //#0080 at 003DDBD4 org ZHM3LevelControl::DisguiseUsed(unsigned int)
        virtual void CanHMPlaceItemToContainer(ZItemContainer *,bool &); //#0081 at 00475EB8 org ZHM3LevelControl::CanHMPlaceItemToContainer(ZItemContainer *,bool &)
        virtual void CanHMRetrieveItemFromContainer(ZItemContainer *,bool &); //#0082 at 00475EC4 org ZHM3LevelControl::CanHMRetrieveItemFromContainer(ZItemContainer *,bool &)
        virtual void PlaceItemToContainer(ZItemContainer *,ZItem *); //#0083 at 00475ED0 org ZHM3LevelControl::PlaceItemToContainer(ZItemContainer *,ZItem *)
        virtual void RetrieveItemFromContainer(ZItemContainer *,ZItem *); //#0084 at 00475ED8 org ZHM3LevelControl::RetrieveItemFromContainer(ZItemContainer *,ZItem *)
        virtual void OnHitmanPlaceItemToContainer(ZItemContainer *,ZItem *); //#0085 at 00475EE0 org ZHM3LevelControl::OnHitmanPlaceItemToContainer(ZItemContainer *,ZItem *)
        virtual void OnHitmanRetrieveItemFromContainer(ZItemContainer *,ZItem *); //#0086 at 00475EE8 org ZHM3LevelControl::OnHitmanRetrieveItemFromContainer(ZItemContainer *,ZItem *)
        virtual void CanHitmanOperateGenericAction(ZAction *,bool &); //#0087 at 003FEC1C org ZHM3LevelControlM13::CanHitmanOperateGenericAction(ZAction *,bool &)
        virtual void OnHitmanBreakUtilBox(ZGEOM *); //#0088 at 00475EFC org ZHM3LevelControl::OnHitmanBreakUtilBox(ZGEOM *)
        virtual void OnUtilBoxRepaired(ZGEOM *); //#0089 at 00475F04 org ZHM3LevelControl::OnUtilBoxRepaired(ZGEOM *)
        virtual void OnPickLockOpenDoor(ZGEOM *); //#0090 at 00475F0C org ZHM3LevelControl::OnPickLockOpenDoor(ZGEOM *)
        virtual void OnOpenedDoorWithKeycard(ZGEOM *); //#0091 at 00475F14 org ZHM3LevelControl::OnOpenedDoorWithKeycard(ZGEOM *)
        virtual void OnHitmanReloadWeapon(ZItem *); //#0092 at 00475F1C org ZHM3LevelControl::OnHitmanReloadWeapon(ZItem *)
        virtual void OnHitmanPickupWeapon(ZItem *); //#0093 at 00475F24 org ZHM3LevelControl::OnHitmanPickupWeapon(ZItem *)
        virtual void OnHitmanPickupItem(ZItem *); //#0094 at 00475F2C org ZHM3LevelControl::OnHitmanPickupItem(ZItem *)
        virtual void OnHitmanDisarmNPC(ZHM3Actor *); //#0095 at 00475F34 org ZHM3LevelControl::OnHitmanDisarmNPC(ZHM3Actor *)
        virtual void OnHitmanPutWeaponRightHandItem(ZItem *); //#0096 at 00475F3C org ZHM3LevelControl::OnHitmanPutWeaponRightHandItem(ZItem *)
        virtual void OnHitmanPlacedBomb(); //#0097 at 00475F44 org ZHM3LevelControl::OnHitmanPlacedBomb(void)
        virtual void OnHitmanUseSyringeOnItem(ZHM3Item *); //#0098 at 00475F4C org ZHM3LevelControl::OnHitmanUseSyringeOnItem(ZHM3Item *)
        virtual void OnHitmanUseSyringe(EHM3ItemType); //#0099 at 003DE0E8 org ZHM3LevelControl::OnHitmanUseSyringe(EHM3ItemType)
        virtual void OnHitmanHideInCloset(ZGEOM *); //#0100 at 00475F54 org ZHM3LevelControl::OnHitmanHideInCloset(ZGEOM *)


    public:
        //Data
        int field_30;
        int field_34;
        int field_38;
        int field_3c;
        int field_40;
        int field_44;
        int field_48;
        int field_4c;
        int field_50;
        int field_54;
        int field_58;
        int field_5c;
        int field_60;
        int field_64;
        int field_68;
        int field_6c;
        int field_70;
        int field_74;
        int field_78;
        int field_7c;
        int field_80;
        int field_84;
        int field_88;
        int field_8c;
        int field_90;
        int field_94;
        int field_98;
        int field_9c;
        int field_a0;
        int field_a4;
        int field_a8;
        int field_ac;
        int field_b0;
        int field_b4;
        int field_b8;
        int field_bc;
        int field_c0;
        int field_c4;
        int field_c8;
        int field_cc;
        int field_d0;
        int field_d4;
        int field_d8;
        int field_dc;
        int field_e0;
        int field_e4;
        int field_e8;
        int field_ec;
        int field_f0;
        int field_f4;
        int field_f8;
        int field_fc;
        int field_100;
        int field_104;
        int field_108;
        int field_10c;
        int field_110;
        int field_114;
        int field_118;
        int field_11c;
        int field_120;
        int field_124;
        int field_128;
        int field_12c;
        int field_130;
        int field_134;
        int field_138;
        int field_13c;
        int field_140;
        int field_144;
        int field_148;
        int field_14c;
        int field_150;
        int field_154;
        int field_158;
        int field_15c;
        int field_160;
        int field_164;
        int field_168;
        int field_16c;
        int field_170;
        int field_174;
        int field_178;
        int field_17c;
        int field_180;
        int field_184;
        int field_188;
        int field_18c;
        int field_190;
        int field_194;
        int field_198;
        int field_19c;
        int field_1a0;
        int field_1a4;
        int field_1a8;
        int field_1ac;
        int field_1b0;
        int field_1b4;
        int field_1b8;
        int field_1bc;
        int field_1c0;
        int field_1c4;
        int field_1c8;
        int field_1cc;
        int field_1d0;
        int field_1d4;
        int field_1d8;
        int field_1dc;
        int field_1e0;
        int field_1e4;
        int field_1e8;
        int field_1ec;
        int field_1f0;
        int field_1f4;
        int field_1f8;
        int field_1fc;
        int field_200;
        int field_204;
        int field_208;
        int field_20c;
        int field_210;
        int field_214;
        int field_218;
        int field_21c;
        int field_220;
        int field_224;
        int field_228;
        int field_22c;
        int field_230;
        int field_234;
        int field_238;
        int field_23c;
        int field_240;
        int field_244;
        int field_248;
        int field_24c;
        int field_250;
        int field_254;
        int field_258;
        int field_25c;
        int field_260;
        int field_264;
        int field_268;
        int field_26c;
        int field_270;
        int field_274;
        int field_278;
        int field_27c;
        int field_280;
        int field_284;
        int field_288;
        int field_28c;
        int field_290;
        int field_294;
        int field_298;
        int field_29c;
        int field_2a0;
        int field_2a4;
        int field_2a8;
        int field_2ac;
        int field_2b0;
        int field_2b4;
        int field_2b8;
        int field_2bc;
        int field_2c0;
        int field_2c4;
        int field_2c8;
        int field_2cc;
        int field_2d0;
        int field_2d4;
        int field_2d8;
        int field_2dc;
        int field_2e0;
        int field_2e4;
        int field_2e8;
        int field_2ec;
        int field_2f0;
        int field_2f4;
        int field_2f8;
        int field_2fc;
        int field_300;
        int field_304;
        int field_308;
        int field_30c;
        int field_310;
        int field_314;
        int field_318;
        int field_31c;
        int field_320;
        int field_324;
        int field_328;
        int field_32c;
        int field_330;
        int field_334;
        int field_338;
        int field_33c;
        int field_340;
        int field_344;
        int field_348;
        int field_34c;
        int field_350;
        int field_354;
        int field_358;
        int field_35c;
        int field_360;
        int field_364;
        int field_368;
        int field_36c;
        int field_370;
        int field_374;
        int field_378;
        int field_37c;
        int field_380;
        int field_384;
        int field_388;
        int field_38c;
        int field_390;
        int field_394;
        int field_398;
        int field_39c;
        int field_3a0;
        int field_3a4;
        int field_3a8;
        int field_3ac;
        int field_3b0;
        int field_3b4;
        int field_3b8;
        int field_3bc;
        int field_3c0;
        int field_3c4;
        int field_3c8;
        int field_3cc;
        int field_3d0;
        int field_3d4;
        int field_3d8;
        int field_3dc;
        int field_3e0;
        int field_3e4;
        int field_3e8;
        int field_3ec;
        int field_3f0;
        int field_3f4;
        int field_3f8;
        int field_3fc;
        int field_400;
        int field_404;
        int field_408;
        int field_40c;
        int field_410;
        int field_414;
        int field_418;
        int field_41c;
        int field_420;
        int field_424;
        int field_428;
        int field_42c;
        int field_430;
        int field_434;
        int field_438;
        int field_43c;
        int field_440;
        int field_444;
        int field_448;
        int field_44c;
        int field_450;
        int field_454;
        int field_458;
        int field_45c;
        int field_460;
        int field_464;
        int field_468;
        int field_46c;
        int field_470;
        int field_474;
        int field_478;
        int field_47c;
        int field_480;
        int field_484;
        int field_488;
        int field_48c;
        int field_490;
        int field_494;
        int field_498;
        int field_49c;
        int field_4a0;
        int field_4a4;
        int field_4a8;
        int field_4ac;
        int field_4b0;
        int field_4b4;
        int field_4b8;
        int field_4bc;
        int field_4c0;
        int field_4c4;
        int field_4c8;
        int field_4cc;
        int field_4d0;
        int field_4d4;
        int field_4d8;
        int field_4dc;
        int field_4e0;
        int field_4e4;
        int field_4e8;
        int field_4ec;
        int field_4f0;
        int field_4f4;
        int field_4f8;
        int field_4fc;
        int field_500;
        int field_504;
        int field_508;
        int field_50c;
        int field_510;
        int field_514;
        int field_518;
        int field_51c;
        int field_520;
        int field_524;
        int field_528;
        int field_52c;
        int field_530;
        int field_534;
        int field_538;
        int field_53c;
        int field_540;
        int field_544;
        int field_548;
        int field_54c;
        int field_550;
        int field_554;
        int field_558;
        int field_55c;
        int field_560;
        int field_564;
        int field_568;
        int field_56c;
        int field_570;
        int field_574;
        int field_578;
        int field_57c;
        int field_580;
        int field_584;
        int field_588;
        int field_58c;
        int field_590;
        int field_594;
        int field_598;
        int field_59c;
        int field_5a0;
        int field_5a4;
        int field_5a8;
        int field_5ac;
        int field_5b0;
        int field_5b4;
        int field_5b8;
        int field_5bc;
        int field_5c0;
        int field_5c4;
        int field_5c8;
        int field_5cc;
        int field_5d0;
        int field_5d4;
        int field_5d8;
        int field_5dc;
        int field_5e0;
        int field_5e4;
        int field_5e8;
        int field_5ec;
        int field_5f0;
        int field_5f4;
        int field_5f8;
        int field_5fc;
        int field_600;
        int field_604;
        int field_608;
        int field_60c;
        int field_610;
        int field_614;
        int field_618;
        int field_61c;
        int field_620;
        int field_624;
        int field_628;
        int field_62c;
        int field_630;
        int field_634;
        ZPodiumController m_podiumControl;
    }; //End of ZHM3LevelControlM13 from 0057E6B8
}