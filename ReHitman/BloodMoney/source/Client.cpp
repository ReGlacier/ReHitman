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
        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZGEOM_RefToPtr        = 0x004E5BE0;

        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZHumanBoid_SetTarget  = 0x00585670;

        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_ParentGroup = 0x00432640;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_SetName     = 0x00431570;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_SetPrim     = 0x00431DB0;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_GetMatPos   = 0x00431430;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_Next        = 0x004317C0;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_SetNext     = 0x004317F0;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_GetPrev     = 0x00431E20;
        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_SetPrev     = 0x00431E50;

        Glacier::G1ConfigurationService::G1API_FunctionAddress_ZAction_GetActionArray = 0x0052B670;
#pragma endregion
#pragma region BloodMoney Configuration Table
        BloodMoney::BMConfigurationService::BMAPI_FunctionAddress_ZPathFollower_GetClosestWaypoint      = 0x00654450;
        BloodMoney::BMConfigurationService::BMAPI_FunctionAddress_ZPathFollower_GetRndUsePoint          = 0x00654580;
        BloodMoney::BMConfigurationService::BMAPI_FunctionAddress_ZPathFollower_SetExternalWaypointList = 0x006543F0;
        BloodMoney::BMConfigurationService::BMAPI_FunctionAddress_ZPathFollower_SetWaypointIndex        = 0x00654270;
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
        //m_patches->RegisterPatch<ZGEOMManagementPatch>();
        m_patches->RegisterPatch<EnableCheatsPatch>();
        m_patches->RegisterPatch<ZCutSequencePlayerPatch>();
    }
}