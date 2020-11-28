#include <BloodMoney/UI/Widgets/ActorsListWidget.h>

#include <BloodMoney/Game/ZHM3GameData.h>
#include <BloodMoney/Game/ZHM3Actor.h>
#include <BloodMoney/Game/Globals.h>

#include <BloodMoney/UI/ImGuiInspector.h>
#include <BloodMoney/UI/GlacierInspectors.h>

#include <Glacier/ZSysInterfaceWintel.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/ZEntityLocator.h>
#include <Glacier/ZGeomBuffer.h>
#include <Glacier/ZHumanBoid.h>
#include <Glacier/Glacier.h>
#include <Glacier/ZGROUP.h>
#include <Glacier/ZGEOM.h>

#include <spdlog/spdlog.h>
#include <imgui.h>

namespace Hitman::BloodMoney
{
    static constexpr const char* kWindowTitle = "Actors List";

    void DrawErrorDialog(const char* errorMsg)
    {
        ImGui::Begin(kWindowTitle);
        ImGui::TextColored(ImVec4 { 1.f, 1.f, 0.f, 1.f }, errorMsg);
        ImGui::End();
    }

    void DrawActorGeneralInfo(Hitman::BloodMoney::ZHM3Actor* actor)
    {
        ImGui::BeginTabItem("General");
        {
            ImGui::Text("Name: "); ImGui::SameLine(0.f, 4.f); ImGui::TextColored(ImVec4{ 0.f, 1.f, 0.f, 1.f }, "%s", actor->ActorInformation->location->entityName);
            ImGui::Inspector<Glacier::ZVector3>::Draw("Actor position", &actor->ActorInformation->location->position);
            ImGui::Inspector<Glacier::ZHumanBoid>::Draw("Actor boid", actor->m_boid);
        }
        ImGui::EndTabItem();
    }

    void DrawActorInfo(Hitman::BloodMoney::ZHM3Actor* actor, int actorIndex)
    {
        ImGui::Text("Actor #%.2d (at 0x%.8X)", actorIndex, reinterpret_cast<uint32_t>(actor));
        ImGui::Separator();

        if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
        {
            DrawActorGeneralInfo(actor);
        }
        ImGui::EndTabBar();
    }

    void DrawActorsListDialog(Glacier::ZEngineDataBase* engineDataBase, Hitman::BloodMoney::ZHM3GameData* gameData)
    {
        ImGui::SetNextWindowSize(ImVec2(650, 440), ImGuiCond_FirstUseEver);

        ImGui::Begin("Actors List", &ActorsListWidget::g_bIsOpened);
        {
            ImGui::Separator();

            // Left side
            static int selectedIndex = 0;
            ImGui::BeginChild("$leftside", ImVec2(300, 0), true);
            for (int actorIndex = 0; actorIndex < gameData->m_ActorsInPoolCount; actorIndex++)
            {
                auto entityName = fmt::format("#{:2d} {}",
                                              (actorIndex + 1),
                                              gameData->m_ActorsPool[actorIndex]->ActorInformation->location->entityName);
                if (ImGui::Selectable(entityName.data(), selectedIndex == actorIndex))
                {
                    selectedIndex = actorIndex;
                }
            }
            ImGui::EndChild();
            ImGui::SameLine();

            //Right side
            Hitman::BloodMoney::ZHM3Actor* currentActor = gameData->m_ActorsPool[selectedIndex];

            ImGui::BeginGroup();
            ImGui::BeginChild("$itemview", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

            DrawActorInfo(currentActor, selectedIndex);

            ImGui::EndChild();
            ImGui::EndGroup();
        }
        ImGui::End();
    }

    void ActorsListWidget::draw()
    {
        DebugWidget::draw();

        if (!g_bIsOpened) return;

        auto sysInterface = Glacier::getInterface<Glacier::ZSysInterfaceWintel>(Globals::kSysInterfaceAddr);
        if (!sysInterface) { return; }

        auto engineDb = sysInterface->m_engineDataBase;
        if (!engineDb) { return; }

        auto gameData = Glacier::getInterface<Hitman::BloodMoney::ZHM3GameData>(Globals::kGameDataAddr);

        if (!gameData)
        {
            DrawErrorDialog("NO GAME DATA");
            return;
        }

        if (!gameData->m_ActorsInPoolCount)
        {
            DrawErrorDialog("NO ACTORS ON THIS SCENE");
            return;
        }

        DrawActorsListDialog(engineDb, gameData);
    }

    bool ActorsListWidget::g_bIsOpened = false;
}