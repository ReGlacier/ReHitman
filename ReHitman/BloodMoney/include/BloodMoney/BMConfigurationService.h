#pragma once

#include <cstdint>

namespace Hitman::BloodMoney
{
    struct BMConfigurationService
    {
        // Consts
        static constexpr std::intptr_t kNotConfiguredOption = 0x0;

        // Config
        static std::intptr_t BMAPI_FunctionAddress_ZPathFollower_GetClosestWaypoint;
        static std::intptr_t BMAPI_FunctionAddress_ZPathFollower_GetRndUsePoint;
        static std::intptr_t BMAPI_FunctionAddress_ZPathFollower_SetExternalWaypointList;
        static std::intptr_t BMAPI_FunctionAddress_ZPathFollower_SetWaypointIndex;
        static std::intptr_t BMAPI_FunctionAddress_ZHM3ItemBomb_Explode;
        static std::intptr_t BMAPI_FunctionAddress_CMetalDetector_DoDetectWeapon;
        static std::intptr_t BMAPI_FunctionAddress_CMetalDetector_DoAlarm;
        static std::intptr_t BMAPI_FunctionAddress_ZHM3Actor_PreparePath;
        static std::intptr_t BMAPI_FunctionAddress_ZHM3Actor_ActivateBloodSpurt;
        static std::intptr_t BMAPI_FunctionAddress_ZTie_HideTie;
        static std::intptr_t BMAPI_FunctionAddress_ZTie_HideTieInMirror;
        static std::intptr_t BMAPI_FunctionAddress_ZXMLGUISystem_GetTopWindow;
        static std::intptr_t BMAPI_FunctionAddress_CTelePortList_TeleportToPointAtIndex;
        static std::intptr_t BMAPI_FunctionAddress_ZHM3DialogControl_StartDialog;
        static std::intptr_t BMAPI_FunctionAddress_ZHM3DialogControl_SetDialogSkipable;
        static std::intptr_t BMAPI_FunctionAddress_CElevatorHouse_GetElevatorDoorStatus;
        static std::intptr_t BMAPI_FunctionAddress_ZOSD_AddInfo;
        static std::intptr_t BMAPI_FunctionAddress_ZOSD_AddWarning;
        static std::intptr_t BMAPI_FunctionAddress_ZOSD_AddHint;
        static std::intptr_t BMAPI_FunctionAddress_CKeycardReader_GetNearestDoor;
        static std::intptr_t BMAPI_FunctionAddress_CKeycardReader_ValidateKeyCard;
        static std::intptr_t BMAPI_FunctionAddress_CKeycardReader_InvalidateKeyCard;
        static std::intptr_t BMAPI_FunctionAddress_CKeycardReader_DetermineLnkObjLocation;
        static std::intptr_t BMAPI_FunctionAddress_CKeycardReader_GetKeyCard;
        static std::intptr_t BMAPI_FunctionAddress_CKeycardReader_GetItemTemplateFromName;
        static std::intptr_t BMAPI_FunctionAddress_CKeycardReader_IsKeyCardValid;
        static std::intptr_t BMAPI_FunctionAddress_ZLINEOBJ_FactoryConstructor;
        static std::intptr_t BMAPI_FunctionAddress_ZTTFONT_FactoryConstructor;
        static std::intptr_t BMAPI_FunctionAddress_ZKerningFont_FactoryConstructor;
        static std::intptr_t BMAPI_FunctionAddress_ZHM3WeaponUpgradeControl_InitWeaponReferences;
        static std::intptr_t BMAPI_FunctionAddress_ZHM3WeaponUpgradeControl_ApplyDefaultUpgrades;
        static std::intptr_t BMAPI_FunctionAddress_ZHM3WeaponUpgradeControl_GetWeaponType;
        static std::intptr_t BMAPI_FunctionAddress_ZHM3ItemWeaponCustom_ApplyUpgrades;
        static std::intptr_t BMAPI_FunctionAddress_ZHM3ItemWeaponCustom_UpdateWeaponPartDrawStatus;
        static std::intptr_t BMAPI_FunctionAddress_ZHM3ItemWeaponCustom_ClearUpgrades;
        static std::intptr_t BMAPI_FunctionAddress_ZHM3BriefingControl_CompleteObjective;
        static std::intptr_t BMAPI_FunctionAddress_ZHM3BriefingControl_PlaySpeech;
        static std::intptr_t BMAPI_FunctionAddress_ZHM3BriefingControl_StopSpeech;
        static std::intptr_t BMAPI_FunctionAddress_ZWINOBJ_GetTexture;
        static std::intptr_t BMAPI_FunctionAddress_ZGUIBase_GetSize;
        static std::intptr_t BMAPI_FunctionAddress_PF4_CreatePathFinder;
        static std::intptr_t BMAPI_FunctionAddress_ZHM3InventoryMenu_InitInventoryList;
        static std::intptr_t BMAPI_FunctionAddress_ZHM3InventoryMenu_CloseItemView;
        static std::intptr_t BMAPI_FunctionAddress_ZHM3InventoryMenu_CloseInventoryMenu;
        static std::intptr_t BMAPI_FunctionAddress_ZHM3InventoryMenu_UpdateItemView;
        static std::intptr_t BMAPI_FunctionAddress_ZHM3InventoryMenu_UpdateItemInfo;
        static std::intptr_t BMAPI_FunctionAddress_ZHM3InventoryMenu_SpinCircle;
        static std::intptr_t BMAPI_FunctionAddress_ZHM3InventoryMenu_TurnLeft;
        static std::intptr_t BMAPI_FunctionAddress_ZHM3InventoryMenu_TurnRight;
        static std::intptr_t BMAPI_FunctionAddress_IGUIElement_ChangeColorSet;
        static std::intptr_t BMAPI_FunctionAddress_IGUIElement_ChangeColor;
        static std::intptr_t BMAPI_FunctionAddress_IGUIElement_SetColor;
        static std::intptr_t BMAPI_FunctionAddress_IGUIElement_GetRightPosOfTextGroup;
        static std::intptr_t BMAPI_FunctionAddress_ZHM3MenuElements_GetGUIElement;
        static std::intptr_t BMAPI_FunctionAddress_ZGuardQuarterController_RegisterActor;
        static std::intptr_t BMAPI_GlobalVariableAddress_ZVCR_m_msgStealTape;
    };
}