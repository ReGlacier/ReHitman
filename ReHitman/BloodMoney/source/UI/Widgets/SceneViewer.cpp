#include <BloodMoney/UI/Widgets/SceneViewer.h>

#include <spdlog/spdlog.h>
#include <imgui.h>

#include <BloodMoney/UI/GlacierInspectors.h>

#include <BloodMoney/Game/ZHM3GameData.h>

#include <BloodMoney/Game/Globals.h>

#include <Glacier/ZSysInterfaceWintel.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/Geom/ZEntityLocator.h>
#include <Glacier/Glacier.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/Items/ZItem.h>
#include <Glacier/Items/ZItemTemplate.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/Geom/ZTreeGroup.h>
#include <Glacier/Geom/ZSNDOBJ.h>
#include <Glacier/ZTypeTraits.h>

#include <BloodMoney/Game/ZHM3Actor.h>

#include <HF/HackingFramework.hpp>

namespace Hitman::BloodMoney
{
    static int g_nodeId = 0;

    static void PrepareGroup(Glacier::ZGROUP* currentGroup)
    {
        if (!currentGroup) return;

        Glacier::ZEntityLocator* currentEntity = currentGroup->m_pEntity0;

        while (currentEntity) {
            auto assignedTo = reinterpret_cast<Glacier::ZGEOM*>(currentEntity->m_assignedTo);
            if (!assignedTo) {
                currentEntity = currentEntity->Next();
                continue;
            }

            ImGui::PushID(g_nodeId);

            auto classInfo = assignedTo->GetOldClassInfo();
            std::string_view type = "N/A";
            std::string_view parent = "N/A";

            if (classInfo) { type = classInfo->ComplexTypeName; }

            if (Glacier::ZCast<Glacier::ZGROUP>::IsBasedOn(assignedTo)) {
                auto childGroup = reinterpret_cast<Glacier::ZGROUP*>(assignedTo);
                if (!childGroup) {
                    ImGui::TextColored(ImVec4{1.f, 0.f, 0.f, 1.f}, "Bad group pointer!");
                } else {
                    if (ImGui::TreeNode(fmt::format("Group \"{}\" (type: {} of {})", currentEntity->entityName, type, parent).c_str())) {
                        ImGui::Inspector<Glacier::ZEntityLocator>::Draw("NONE", assignedTo->m_baseGeom);
                        ImGui::Separator();
                        PrepareGroup(childGroup);
                        ImGui::TreePop();
                    }
                }
            }
            else if (Glacier::ZCast<Hitman::BloodMoney::ZHM3Actor>::IsBasedOn(assignedTo)) {
                if (ImGui::TreeNode(fmt::format("Actor \"{}\" (type: {} of {})", currentEntity->entityName, type, parent).c_str())) {
                    ImGui::Inspector<Hitman::BloodMoney::ZHM3Actor>::Draw("NONE", reinterpret_cast<Hitman::BloodMoney::ZHM3Actor*>(assignedTo));
                    ImGui::TreePop();
                }
            }
            else if (Glacier::ZCast<Glacier::ZCAMERA>::IsBasedOn(assignedTo)) {
                if (ImGui::TreeNode(fmt::format("ZCamera \"{}\" (type: {} of {})", currentEntity->entityName, type, parent).c_str())) {
                    ImGui::Inspector<Glacier::ZCAMERA>::Draw("None", reinterpret_cast<Glacier::ZCAMERA*>(assignedTo));
                    ImGui::TreePop();
                }
            }
            else if (Glacier::ZCast<Glacier::ZSNDOBJ>::IsBasedOn(assignedTo)) {
                if (ImGui::TreeNode(fmt::format("ZSoundObject \"{}\" (type: {} of {})", currentEntity->entityName, type, parent).c_str())) {
                    ImGui::Inspector<Glacier::ZSNDOBJ>::Draw("None", reinterpret_cast<Glacier::ZSNDOBJ*>(assignedTo));
                    ImGui::TreePop();
                }
            }
            else {
                if (ImGui::TreeNode(fmt::format("Entity \"{}\" (type: {} of {}, type ptr: {:08X})", currentEntity->entityName, type, parent, (int)classInfo).c_str())) {
                    ImGui::Inspector<Glacier::ZEntityLocator>::Draw("NONE", currentEntity);
                    ImGui::TreePop();
                }
            }
            ImGui::PopID();
            ++g_nodeId;

            //ImGui::Separator();
            currentEntity = currentEntity->Next();
        }
    }


    void SceneViewer::draw() {
        DebugWidget::draw();

        ImGui::Begin("Scene viewer", &SceneViewer::g_bIsVisible);

        auto sysInterface = Glacier::getInterface<Glacier::ZSysInterfaceWintel>(Globals::kSysInterfaceAddr);
        if (!sysInterface) { return; }

        auto engineDb = sysInterface->m_engineDataBase;
        if (!engineDb) { return; }

        ImGui::Text("Scene Inspector");
        ImGui::Separator();

        g_nodeId = 0;
        PrepareGroup(engineDb->m_root);

        ImGui::End();
    }

    bool SceneViewer::g_bIsVisible = false;
}