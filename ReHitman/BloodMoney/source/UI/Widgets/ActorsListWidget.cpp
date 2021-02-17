#include <BloodMoney/UI/Widgets/ActorsListWidget.h>

#include <BloodMoney/Game/PF/ZPathFollower.h>
#include <BloodMoney/Game/ZHM3GameData.h>
#include <BloodMoney/Game/ZHM3Actor.h>
#include <BloodMoney/Game/Globals.h>

#include <BloodMoney/UI/ImGuiInspector.h>
#include <BloodMoney/UI/GlacierInspectors.h>
#include <BloodMoney/UI/BloodMoneyInspectors.h>

#include <Glacier/ZRenderWintelD3DDll.h>
#include <Glacier/ZSysInterfaceWintel.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/Geom/ZGeomBuffer.h>
#include <Glacier/IK/ZLNKOBJ.h>

#include <Glacier/Fysix/CRigidBody.h>
#include <imgui.h>

namespace ImGui
{
    void Inspector<Hitman::BloodMoney::ZHM3Actor>::Draw(const char* id, Hitman::BloodMoney::ZHM3Actor* actor) {
        {
            //ImGui::Inspector<Glacier::ZEntityLocator>::Draw("actor.entity", actor->ActorInformation->location);
            ImGui::Inspector<Glacier::ZEntityLocator>::Draw("actor.entity", reinterpret_cast<Glacier::ZGEOM*>(actor)->m_baseGeom);
            ImGui::Inspector<Glacier::ZHumanBoid>::Draw("Actor boid", actor->m_boid);
            ImGui::Text("Group Info: ");
            ImGui::Inspector<Glacier::ZGROUP>::Draw("ActorGroup", actor->ActorInformation->location->ParentGroup());
            ImGui::Inspector<Hitman::BloodMoney::ZPathFollower>::Draw("Actor.PathFollower", reinterpret_cast<Hitman::BloodMoney::ZPathFollower*>(actor->FindEvent(Hitman::BloodMoney::ZPathFollower::Name)));
            ImGui::Inspector<Glacier::CInventory>::Draw("Actor.Inventory", reinterpret_cast<Glacier::CInventory*>(actor->FindEvent(Glacier::CInventory::Name)));

            /// ==================
            {
                auto gameData = Glacier::getInterface<Hitman::BloodMoney::ZHM3GameData>(Hitman::BloodMoney::Globals::kGameDataAddr);
                if (gameData && ImGui::Button("Swap bodies"))
                {
                    auto actorGeom = reinterpret_cast<Glacier::ZGEOM*>(actor);
                    auto playerGeom = reinterpret_cast<Glacier::ZGEOM*>(gameData->m_Hitman3);

                    const int actorPrimId  = actorGeom->m_baseGeom->m_primitive;
                    const int playerPrimId = playerGeom->m_baseGeom->m_primitive;

                    reinterpret_cast<Glacier::ZLNKOBJ*>(actor)->CopyGeometryFrom(playerPrimId);
                    reinterpret_cast<Glacier::ZLNKOBJ*>(actor)->UpdateGeometry(true);

                    reinterpret_cast<Glacier::ZLNKOBJ*>(playerGeom)->CopyGeometryFrom(actorPrimId);
                    reinterpret_cast<Glacier::ZLNKOBJ*>(playerGeom)->UpdateGeometry(true);
                }
            }

            /// ==================
            {
                if (ImGui::Button("Clear PF4 path"))
                {
                    using PF4_Path_Clear_t = void(__thiscall*)(std::intptr_t*);
                    auto PF4_Path_Clear = (PF4_Path_Clear_t)0x004D8D00;

                    PF4_Path_Clear(&actor->m_field534);

                    actor->PreparePath();
                }
            }

            // ===================
            {
                auto gameData = Glacier::getInterface<Hitman::BloodMoney::ZHM3GameData>(Hitman::BloodMoney::Globals::kGameDataAddr);
                if (gameData && ImGui::Button("Start human shield with"))
                {
                    using StartActionHumanShield_t = void(__thiscall*)(std::intptr_t*, Hitman::BloodMoney::ZHM3Actor*);
                    auto StartActionHumanShield = (StartActionHumanShield_t)0x00600590;

                    StartActionHumanShield(reinterpret_cast<std::intptr_t*>(gameData->m_Hitman3), actor);
                }
            }
        }
    }
}

namespace Hitman::BloodMoney
{
    static constexpr const char* kWindowTitle = "Actors List";

    void DrawErrorDialog(const char* errorMsg)
    {
        ImGui::Begin(kWindowTitle, &ActorsListWidget::g_bIsOpened);
        ImGui::TextColored(ImVec4 { 1.f, 1.f, 0.f, 1.f }, "%s", errorMsg);
        ImGui::End();
    }

    void DrawActorInfo(Hitman::BloodMoney::ZHM3Actor* actor, int actorIndex)
    {
        ImGui::Text("Actor #%.2d (at 0x%.8X)", actorIndex, reinterpret_cast<uint32_t>(actor));
        ImGui::Separator();

        if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
        {
            ImGui::BeginTabItem("General");
            ImGui::Inspector<Hitman::BloodMoney::ZHM3Actor>::Draw("???", actor);
            ImGui::EndTabItem();
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