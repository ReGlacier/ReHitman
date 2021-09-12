#include <MonoBindings.h>
#include <spdlog/spdlog.h>
#include <mono/jit/jit.h>

#include <Glacier/ZSysInterfaceWintel.h>
#include <Glacier/ZEngineDataBase.h>
#include <BloodMoney/Game/ZHM3GameData.h>
#include <BloodMoney/Game/Globals.h>

namespace API {
    static void DeveloperConsole_Info(MonoString* psMessage) {
        if (psMessage) {
            spdlog::info("(Managed) {}", mono_string_to_utf8(psMessage));
        }
    }

    static void DeveloperConsole_Warning(MonoString* psMessage) {
        if (psMessage) {
            spdlog::warn("(Managed) {}", mono_string_to_utf8(psMessage));
        }
    }

    static void DeveloperConsole_Error(MonoString* psMessage) {
        if (psMessage) {
            spdlog::error("(Managed) {}", mono_string_to_utf8(psMessage));
        }
    }

    static float Game_GetTimeScale() {
        auto pSysInterface = Glacier::getInterface<Glacier::ZSysInterfaceWintel>(Hitman::BloodMoney::Globals::kSysInterfaceAddr);
        if (!pSysInterface) {
            return .0f;
        }
        //TODO: Refactor this place
        return *reinterpret_cast<float*>(reinterpret_cast<int>(pSysInterface) + 0xB24);
    }

    static void Game_SetTimeScale(float value) {
        auto pSysInterface = Glacier::getInterface<Glacier::ZSysInterfaceWintel>(Hitman::BloodMoney::Globals::kSysInterfaceAddr);
        if (!pSysInterface) {
            return;
        }

        //TODO: Refactor this place
        *reinterpret_cast<float*>(reinterpret_cast<int>(pSysInterface) + 0xB24) = value;
    }

    static MonoString* Game_GetSceneName() {
        auto pSysInterface = Glacier::getInterface<Glacier::ZSysInterfaceWintel>(Hitman::BloodMoney::Globals::kSysInterfaceAddr);
        if (!pSysInterface) {
            return nullptr;
        }

        auto pEngineDb = pSysInterface->m_engineDataBase;
        if (!pEngineDb) {
            return nullptr;
        }

        if (!pEngineDb->GetSceneName()) {
            return nullptr;
        }

        return mono_string_new(mono_domain_get(), pEngineDb->GetSceneName());
    }

    static MonoString* Game_GetProfileName() {
        auto pGameData = Glacier::getInterface<Hitman::BloodMoney::ZHM3GameData>(Hitman::BloodMoney::Globals::kGameDataAddr);
        if (!pGameData) {
            return nullptr;
        }

        return mono_string_new(mono_domain_get(), &pGameData->m_ProfileName[0]);
    }

    static int32_t Game_GetProfileMoney() {
        auto pGameData = Glacier::getInterface<Hitman::BloodMoney::ZHM3GameData>(Hitman::BloodMoney::Globals::kGameDataAddr);
        if (!pGameData) {
            return 0;
        }

        return pGameData->m_PlayerMoney;
    }

    static void Game_SetProfileMoney(int32_t money) {
        auto pGameData = Glacier::getInterface<Hitman::BloodMoney::ZHM3GameData>(Hitman::BloodMoney::Globals::kGameDataAddr);
        if (pGameData) {
            pGameData->m_PlayerMoney = money;
        }
    }
}

namespace Hitman::BloodMoney::Scripting {
    void MonoBindings::DeclareFunctions() {
        // DeveloperConsole
        mono_add_internal_call("ReHitman.ScriptingRuntime.Native.DeveloperConsole::Info", API::DeveloperConsole_Info);
        mono_add_internal_call("ReHitman.ScriptingRuntime.Native.DeveloperConsole::Warning", API::DeveloperConsole_Warning);
        mono_add_internal_call("ReHitman.ScriptingRuntime.Native.DeveloperConsole::Error", API::DeveloperConsole_Error);

        // Game
        mono_add_internal_call("ReHitman.ScriptingRuntime.Native.Game::GetTimeScale", API::Game_GetTimeScale);
        mono_add_internal_call("ReHitman.ScriptingRuntime.Native.Game::SetTimeScale", API::Game_SetTimeScale);
        mono_add_internal_call("ReHitman.ScriptingRuntime.Native.Game::GetSceneName", API::Game_GetSceneName);
        mono_add_internal_call("ReHitman.ScriptingRuntime.Native.Game::GetProfileName", API::Game_GetProfileName);
        mono_add_internal_call("ReHitman.ScriptingRuntime.Native.Game::GetProfileMoney", API::Game_GetProfileMoney);
        mono_add_internal_call("ReHitman.ScriptingRuntime.Native.Game::SetProfileMoney", API::Game_SetProfileMoney);
    }
}