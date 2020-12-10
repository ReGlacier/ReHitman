#pragma once

#include <BloodMoney/Game/ZHM3Actor.h>
#include <BloodMoney/Game/LevelControls/ZHM3LevelControl.h>

#include <Glacier/ZGameData.h>
#include <Glacier/ZSTL/REFTAB.h>

#define DECLARE_UNKNOWN_TYPE_PTR(name) using name = std::uintptr_t;

namespace Hitman::BloodMoney
{
    DECLARE_UNKNOWN_TYPE_PTR(N00001B3F);
    DECLARE_UNKNOWN_TYPE_PTR(N000033C1);
    DECLARE_UNKNOWN_TYPE_PTR(ZHM3GameStats);
    DECLARE_UNKNOWN_TYPE_PTR(N0000163D);
    DECLARE_UNKNOWN_TYPE_PTR(CGlobalCom);
    DECLARE_UNKNOWN_TYPE_PTR(ZHM3WeaponUpgradeControl);
    DECLARE_UNKNOWN_TYPE_PTR(ZHM3CameraEventCameraClass);
    DECLARE_UNKNOWN_TYPE_PTR(ZClothTracker);

    class ZHM3DialogControl;
    class ZHM3MenuElements;
    class ZHM3Hitman3;
    class ZOSD;
    class ZGui;
    class CIngameMap;
    class ZHM3BriefingControl;
    class ZHM3CameraClass;
    class ZBoidSystem;

    class ZHM3GameData final : public Glacier::ZGameData
    {
    public:
        ZBoidSystem* m_BoidSystem; //0x0004
        int32_t m_ActorsInPoolCount; //0x0008
        ZHM3Actor* m_ActorsPool[512]; //0x000C
        N000033C1* m_UNKNOWN_LIST; //0x080C
        char pad_0x0810[0x208]; //0x0810
        Glacier::REFTAB* m_REFTAB00; //0x0A18
        ZHM3MenuElements* m_MenuElements; //0x0A1C
        ZHM3Hitman3* m_Hitman3; //0x0A20 (also in A40 located similar instance, it doesn't matter what I will use, but few instructions trying to talk with +A40)
        char pad_0x0A24[0xC]; //0x0A24
        ZHM3GameStats* m_GameStats; //0x0A30
        char pad_0x0A34[0xC]; //0x0A34
        N0000163D* N0000357E; //0x0A40 unknown thing
        char pad_0x0A44[0x4]; //0x0A44
        ZHM3LevelControl* m_LevelControl; //0x0A48
        ZOSD* m_OSD; //0x0A4C
        ZGui* m_Gui; //0x0A50
        CIngameMap* m_IngameMap; //0x0A54
        char pad_0x0A58[0x4]; //0x0A58
        CGlobalCom* m_GlobCom; //0x0A5C
        char m_ProfileName[16]; //0xFC5888
        char pad_0x0A70[0x64]; //0x0A70
        int32_t m_PlayerMoney; //0x0AD4
        char pad_0AD8[24096]; //0x0AD8
        ZHM3BriefingControl* m_BriefingControl; //0x68F8
        char pad_68FC[116]; //0x68FC
        ZHM3WeaponUpgradeControl* m_WeaponUpgradeControl; //0x6970
        ZHM3CameraEventCameraClass* m_CameraEventCameraClass; //0x6974
        ZHM3CameraClass* m_CameraClass; //0x6978
        char pad_697C[24]; //0x697C
        int m_FightControllerComponentID; //0x6994
        int m_field6998;	//0x6998
        ZClothTracker* m_ClothTracker; //0x699C
        char pad_69A0[8]; //0x69A0
        ZHM3DialogControl* m_DialogControl; //0x69A8
    };
}

#undef DECLARE_UNKNOWN_TYPE_PTR