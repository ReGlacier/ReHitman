#include <BloodMoney/UI/DebugTools.h>
#include <BloodMoney/Game/Globals.h>
#include <BloodMoney/Game/ZHM3GameData.h>
#include <BloodMoney/Game/CIngameMap.h>

#include <Glacier/ZEngineDataBase.h>
#include <Glacier/ZSysInterfaceWintel.h>

#include <spdlog/spdlog.h>
#include <imgui.h>

namespace Hitman::BloodMoney
{
    void DebugTools::Draw()
    {
        auto gameData = Glacier::getInterface<ZHM3GameData>(Globals::kGameDataAddr);
        auto sysInterface = Glacier::getInterface<Glacier::ZSysInterfaceWintel>(Globals::kSysInterfaceAddr);
        auto engineDataBase = sysInterface->m_engineDataBase;

        ImGui::Begin("Demo");

        if (gameData && ImGui::Button("Dump props list"))
        {
            for (int i = 0; i < gameData->m_ActorsInPoolCount; i++)
            {
                spdlog::info("Actor[{:03}] props at {:08X}", i, gameData->m_ActorsPool[i]->GetActorProperties());
            }
        }

        ImGui::End();
    }
}