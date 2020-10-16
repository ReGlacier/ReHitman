#include <BloodMoney/UI/DebugTools.h>
#include <BloodMoney/Game/Globals.h>
#include <BloodMoney/Game/ZHM3GameData.h>
#include <BloodMoney/Game/CIngameMap.h>

#include <BloodMoney/Editors/LevelEditorGeomsPool.h>

#include <Glacier/ZEngineDataBase.h>
#include <Glacier/ZSysInterfaceWintel.h>
#include <Glacier/ZGEOM.h>

#include <spdlog/spdlog.h>
#include <imgui.h>

namespace Hitman::BloodMoney
{
    void DebugTools::Draw()
    {
        auto gameData = Glacier::getInterface<ZHM3GameData>(Globals::kGameDataAddr);
        auto sysInterface = Glacier::getInterface<Glacier::ZSysInterfaceWintel>(Globals::kSysInterfaceAddr);
        auto engineDataBase = sysInterface->m_engineDataBase;

        ImGui::Begin("ZGEOMS list");
        ImGui::Text("Total geoms: %.4d", LevelEditorGeomsPool::GetInstance().GetCount());

        ImGui::SetNextItemOpen(true);
        if (ImGui::TreeNode("Scene"))
        {
            // Draw everything
            auto DrawGeomInfo = [](Glacier::ZGEOM* geom) {
                ImGui::InputFloat3("Position", &geom->m_baseGeom->position.x);
                ImGui::Text("Parent: %p", &geom->m_baseGeom->parent);
                ImGui::Text("Data: %p", &geom->m_baseGeom->m_data);

                if (ImGui::Button("Hide")) geom->Hide(true); ImGui::SameLine(0.f, 10.f);
                if (ImGui::Button("Show")) geom->Hide(false); ImGui::SameLine(0.f, 10.f);
                if (ImGui::Button("Draw Bound Box")) geom->DispBound(true);
            };

            LevelEditorGeomsPool::GetInstance().ForEach([DrawGeomInfo](Glacier::ZGEOM* geom) {
                if (ImGui::TreeNode((const void*)geom, "%s (object ID %d)", geom->GetOldClassInfo()->ComplexTypeName, geom->GetObjectId()))
                {
                    DrawGeomInfo(geom);
                    ImGui::TreePop();
                }
            });

            ImGui::TreePop();
        }

        ImGui::End();
    }
}