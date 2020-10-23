#include <BloodMoney/UI/DebugTools.h>
#include <BloodMoney/Game/Globals.h>
#include <BloodMoney/Game/ZHM3GameData.h>
#include <BloodMoney/Game/CIngameMap.h>

#include <Glacier/ResourceCollection.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/ZSysInterfaceWintel.h>

#include <Glacier/ZGEOM.h>

#include <spdlog/spdlog.h>
#include <imgui.h>

namespace Hitman::BloodMoney
{
    void DebugTools::Draw()
    {
        /// Useful interfaces
        auto gameData = Glacier::getInterface<ZHM3GameData>(Globals::kGameDataAddr);
        auto sysInterface = Glacier::getInterface<Glacier::ZSysInterfaceWintel>(Globals::kSysInterfaceAddr);
        Glacier::ZEngineDataBase* engineDataBase = sysInterface ? sysInterface->m_engineDataBase : nullptr;

        /// Test
        ImGui::Begin("Test");
        ImGui::End();
    }
}