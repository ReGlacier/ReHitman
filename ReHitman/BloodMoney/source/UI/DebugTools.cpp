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

        if (ImGui::Button("Aim to buddy and shoot him"))
        {
            for (int i = 1; i < gameData->m_ActorsInPoolCount; i++)
            {
                ZHM3Actor* currentActor = gameData->m_ActorsPool[i];
                ZHM3Actor* previousActor = gameData->m_ActorsPool[i - 1];

                int weaponOfCurrentActor = currentActor->GetWeapon();
                if (weaponOfCurrentActor)
                {
                    Glacier::Vector3 previousActorPos;
                    previousActor->GetActorWorldPosition(&previousActorPos);

                    currentActor->SetAimTarget(0, &previousActorPos, 0.4f, nullptr, 0, 0, -1, 100);
                    currentActor->FireWeapon(weaponOfCurrentActor, true, 5.f, true);
                }

                int weaponOfPreviousActor = previousActor->GetWeapon();
                if (weaponOfPreviousActor)
                {
                    Glacier::Vector3 currentActorPos;
                    currentActor->GetActorWorldPosition(&currentActorPos);

                    previousActor->SetAimTarget(0, &currentActorPos, 0.4f, 0, 0, 0, -1, 4);
                    previousActor->FireWeapon(weaponOfPreviousActor, true, 5.f, true);
                }
            }
        }

        ImGui::End();
    }
}