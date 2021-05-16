#include <BloodMoney/Client.h>

#include <G1ConfigurationService.h>
#include <BloodMoney/BMConfigurationService.h>

#include <HF/HackingFramework.hpp>
#include <spdlog/spdlog.h>

/// Delegates
#include <BloodMoney/Delegates/DX9Delegate.h>
#include <BloodMoney/Delegates/ImGuiInputDelegate.h>

/// Patches
#include <BloodMoney/Patches/All/ZHitman3Patches.h>
#include <BloodMoney/Patches/All/ZDirect3D9DevicePatches.h>
#include <BloodMoney/Patches/All/InputDevicesPatches.h>
#include <BloodMoney/Patches/All/ZGEOMManagementPatch.h>
#include <BloodMoney/Patches/All/EnableCheatsPatch.h>
#include <BloodMoney/Patches/All/ZCutSequencePlayerPatch.h>
#include <BloodMoney/Patches/All/FreeFileSystemPatch.h>
#include <BloodMoney/Patches/Mods/SkinChanger.h>


namespace Hitman::BloodMoney
{
    static constexpr std::string_view kDirectX9DllName = "d3d9.dll";
    static constexpr std::string_view kProcessName = "HitmanBloodMoney.exe";

    bool Client::OnAttach()
    {
        spdlog::info("----------[ WELCOME TO RE:HITMAN PROJECT ]----------");

        if (!LocateModules()) return false;

        // Register game configuration
        if (!RegisterGameConfigurationForGlacier())
        {
            spdlog::error("Failed to register game configuration!");
            return false;
        }

        m_patches = std::make_shared<CommonPatches>(m_selfProcess, m_selfModule, m_d3d9Module);
        RegisterPatches();
        if (!m_patches->Setup())
        {
            spdlog::error("Failed to setup patches!");
            return false;
        }

        return true;
    }

    void Client::OnDestroy()
    {
        m_patches->Release();
        ReleaseModules();
    }

    bool Client::RegisterGameConfigurationForGlacier()
    {
#pragma region Glacier Engine Configuration Table
        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZSysMem_Alloc         = 0x00446720;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZSysMem_Free          = 0x004466D0;

        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZGROUP_CreateGeom     = 0x004EA060;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZGROUP_IsRoot         = 0x004EA2F0;

        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZGEOM_GetMatPos       = 0x004E5E40;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZGEOM_GetRootTM       = 0x004E6990;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZGEOM_RefToPtr        = 0x004E5BE0;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZGEOM_GetRef          = 0x004E5BC0;

        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZHumanBoid_SetTarget  = 0x00585670;

        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_DoInit      = 0x004316C0;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_ParentGroup = 0x00432640;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_SetName     = 0x00431570;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_SetPrim     = 0x00431DB0;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_GetMatPos   = 0x00431430;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_Next        = 0x004317C0;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_SetNext     = 0x004317F0;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_GetPrev     = 0x00431E20;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_SetPrev     = 0x00431E50;

        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZAction_GetActionArray = 0x0052B670;

        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZItemContainer_FreePos               = 0x005103F0;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZItemContainer_OccupyPos             = 0x00510370;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZItemContainer_IsContainerFull       = 0x005105C0;

        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZLnkActionQueue_DispatchNextAction   = 0x00653CB0;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZLnkActionQueue_RemoveAction         = 0x00653D10;

        Glacier::G1ConfigurationService::G1API_FunctionAddress_CRigidBody_Enable                    = 0x005740A0;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_CRigidBody_Disable                   = 0x005741A0;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_CRigidBody_DisableRemove             = 0x00574250;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_CRigidBody_SetPos                    = 0x00574320;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_CRigidBody_SetVelocity               = 0x00574340;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_CRigidBody_SetupTransform            = 0x00574D30;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_CRigidBody_HandleHit                 = 0x00574D70;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_CRigidBody_HandleExplodeBomb         = 0x00574DE0;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_CRigidBody_PlaySound                 = 0x00574E90;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_CRigidBody_CheckCollision4a          = 0x00575010;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_CRigidBody_CheckCollision4b          = 0x005754A0;

        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZHash_int_SMatPos_Put                = 0x00665E20;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZHash_int_SMatPos_Clear              = 0x00663FB0;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZHash_int_SMatPos_Find               = 0x00664060;

        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZEngineDataBase_GetSceneCom          = 0x0045AED0;

        Glacier::G1ConfigurationService::G1API_FunctionAddress_CInventory_AddItem                   = 0x005C5400;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_CInventory_RemoveItem                = 0x005C5780;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZSNDOBJ_AttachToDefaultRoom          = 0x004FA240;

        Glacier::G1ConfigurationService::G1API_FunctionAddress_GetGQC                               = 0x00461A20;

        Glacier::G1ConfigurationService::G1ConfigurationService::G1API_FunctionAddress_ZScriptC_FindScript = 0x00549980;
        Glacier::G1ConfigurationService::G1ConfigurationService::G1API_FunctionAddress_ZAction_AddAction = 0x005D3C10; //another func 0x00549980
        Glacier::G1ConfigurationService::G1ConfigurationService::G1API_FunctionAddress_ZAction_Show = 0x0052BC30;
        Glacier::G1ConfigurationService::G1ConfigurationService::G1API_FunctionAddress_ZAction_Hide = 0x0052B070;

        Glacier::G1ConfigurationService::G1API_InstanceAddress_ZEngineGeomControl                   = 0x007F5AA8;
        Glacier::G1ConfigurationService::G1API_InstanceAddress_CConfiguration_bSubtitles            = 0x008ACA9C;
#pragma endregion
#pragma region BloodMoney Configuration Table
        BloodMoney::BMConfigurationService::BMAPI_FunctionAddress_ZPathFollower_GetClosestWaypoint      = 0x00654450;
        BloodMoney::BMConfigurationService::BMAPI_FunctionAddress_ZPathFollower_GetRndUsePoint          = 0x00654580;
        BloodMoney::BMConfigurationService::BMAPI_FunctionAddress_ZPathFollower_SetExternalWaypointList = 0x006543F0;
        BloodMoney::BMConfigurationService::BMAPI_FunctionAddress_ZPathFollower_SetWaypointIndex        = 0x00654270;
        BloodMoney::BMConfigurationService::BMAPI_FunctionAddress_ZHM3ItemBomb_Explode                  = 0x0064F810;
        BloodMoney::BMConfigurationService::BMAPI_FunctionAddress_CMetalDetector_DoDetectWeapon         = 0x005D5D60;
        BloodMoney::BMConfigurationService::BMAPI_FunctionAddress_CMetalDetector_DoAlarm                = 0x005D5B50;
        BloodMoney::BMConfigurationService::BMAPI_FunctionAddress_ZHM3Actor_PreparePath                 = 0x00505570;
        BloodMoney::BMConfigurationService::BMAPI_FunctionAddress_ZHM3Actor_ActivateBloodSpurt          = 0x0063D210;
        BloodMoney::BMConfigurationService::BMAPI_FunctionAddress_ZTie_HideTie                          = 0x00580470;
        BloodMoney::BMConfigurationService::BMAPI_FunctionAddress_ZTie_HideTieInMirror                  = 0x00580480;
        BloodMoney::BMConfigurationService::BMAPI_FunctionAddress_ZXMLGUISystem_GetTopWindow            = 0x005665E0;
        BloodMoney::BMConfigurationService::BMAPI_FunctionAddress_CTelePortList_TeleportToPointAtIndex  = 0x0052A970;
        BloodMoney::BMConfigurationService::BMAPI_FunctionAddress_ZHM3DialogControl_StartDialog         = 0x00645550;
        BloodMoney::BMConfigurationService::BMAPI_FunctionAddress_ZHM3DialogControl_SetDialogSkipable   = 0x00645060;
        BloodMoney::BMConfigurationService::BMAPI_FunctionAddress_CElevatorHouse_GetElevatorDoorStatus  = 0x0059F1E0;
        BloodMoney::BMConfigurationService::BMAPI_FunctionAddress_ZOSD_AddInfo                          = 0x00660EE0;
        BloodMoney::BMConfigurationService::BMAPI_FunctionAddress_ZOSD_AddWarning                       = 0x00660F20;
        BloodMoney::BMConfigurationService::BMAPI_FunctionAddress_ZOSD_AddHint                          = 0x00660F40;
        BloodMoney::BMConfigurationService::BMAPI_GlobalVariableAddress_ZVCR_m_msgStealTape             = 0x009B3C24;
#pragma endregion
        return true;
    }

    bool Client::LocateModules()
    {
        auto PrintModInfo = [](const HF::Win32::Module::Ptr& mod)
        {
            if (!mod) {
                spdlog::error("BAD MODULE");
                return;
            }

            spdlog::info("Module: {} base address at {:08x} of size {:08x}", mod->getName(), mod->getBaseAddress(), mod->getSize());
        };

        m_selfProcess = std::make_shared<HF::Win32::Process>(kProcessName);
        spdlog::info("Trying to locate modules...");

        if (!m_selfProcess->isValid())
        {
            spdlog::error("Failed to locate {} process", kProcessName);
            return false;
        }

        m_selfModule = m_selfProcess->getSelfModule();
        if (!m_selfModule)
        {
            spdlog::error("Failed to locate self module!");
            return false;
        }

        m_d3d9Module = m_selfProcess->getModule(kDirectX9DllName);
        if (!m_d3d9Module)
        {
            spdlog::error("Failed to locate {} module!", kDirectX9DllName);
            return false;
        }

        PrintModInfo(m_selfModule);
        PrintModInfo(m_d3d9Module);

        spdlog::info("All required modules located!");

        return true;
    }

    void Client::ReleaseModules()
    {
        m_selfModule = nullptr;
        m_d3d9Module = nullptr;
        m_selfProcess = nullptr;
    }

    void Client::RegisterPatches()
    {
        m_patches->RegisterPatch<ZDirect3D9DevicePatches>(std::make_unique<DX9Delegate>());
        m_patches->RegisterPatch<InputDevicesPatches>(std::make_unique<ImGuiInputDelegate>());
        m_patches->RegisterPatch<ZHitman3Patches>();
        m_patches->RegisterPatch<EnableCheatsPatch>();
        m_patches->RegisterPatch<ZCutSequencePlayerPatch>();
        m_patches->RegisterPatch<SkinChanger>();
        m_patches->RegisterPatch<FreeFileSystemPatch>();
    }
}