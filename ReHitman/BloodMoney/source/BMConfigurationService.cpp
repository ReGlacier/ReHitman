#include <BloodMoney/BMConfigurationService.h>

namespace Hitman::BloodMoney
{
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_ZPathFollower_GetClosestWaypoint = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_ZPathFollower_GetRndUsePoint = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_ZPathFollower_SetExternalWaypointList = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_ZPathFollower_SetWaypointIndex = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_ZHM3ItemBomb_Explode = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_CMetalDetector_DoDetectWeapon = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_CMetalDetector_DoAlarm = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_ZHM3Actor_PreparePath = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_ZHM3Actor_ActivateBloodSpurt = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_ZTie_HideTie = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_ZTie_HideTieInMirror = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_ZXMLGUISystem_GetTopWindow = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_CTelePortList_TeleportToPointAtIndex = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_ZHM3DialogControl_StartDialog = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_ZHM3DialogControl_SetDialogSkipable = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_CElevatorHouse_GetElevatorDoorStatus = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_ZOSD_AddInfo = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_ZOSD_AddWarning = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_ZOSD_AddHint = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_CKeycardReader_GetNearestDoor = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_CKeycardReader_ValidateKeyCard = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_CKeycardReader_InvalidateKeyCard = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_CKeycardReader_DetermineLnkObjLocation = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_CKeycardReader_GetKeyCard = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_CKeycardReader_GetItemTemplateFromName = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_CKeycardReader_IsKeyCardValid = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_ZLINEOBJ_FactoryConstructor = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_ZTTFONT_FactoryConstructor = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_ZKerningFont_FactoryConstructor = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_ZHM3WeaponUpgradeControl_InitWeaponReferences = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_ZHM3WeaponUpgradeControl_ApplyDefaultUpgrades = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_ZHM3WeaponUpgradeControl_GetWeaponType = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_ZHM3ItemWeaponCustom_ApplyUpgrades = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_ZHM3ItemWeaponCustom_UpdateWeaponPartDrawStatus = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_ZHM3ItemWeaponCustom_ClearUpgrades = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_ZHM3BriefingControl_CompleteObjective = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_ZHM3BriefingControl_PlaySpeech = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_FunctionAddress_ZHM3BriefingControl_StopSpeech = BMConfigurationService::kNotConfiguredOption;
    std::intptr_t BMConfigurationService::BMAPI_GlobalVariableAddress_ZVCR_m_msgStealTape = BMConfigurationService::kNotConfiguredOption;
}