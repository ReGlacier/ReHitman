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
        static std::intptr_t BMAPI_GlobalVariableAddress_ZVCR_m_msgStealTape;
    };
}