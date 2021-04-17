#include <BloodMoney/UI/Widgets/PlayerTeleportWidget.h>
#include <BloodMoney/Game/Globals.h>
#include <BloodMoney/Game/ZHM3GameData.h>
#include <BloodMoney/UI/ImGuiInspector.h>
#include <BloodMoney/Game/CTelePortList.h>

#include <Glacier/ZSysInterfaceWintel.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/ZCTRLIKLNKOBJ.h>
#include <Glacier/ZEventBuffer.h>

#include <Glacier/ZSTL/ZLIST.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/IK/ZLNKWHANDS.h>

#include <HF/HackingFramework.hpp>

#include <imgui.h>
#include <spdlog/spdlog.h>

namespace Hitman::BloodMoney {
    void PlayerTeleportWidget::draw() {
        DebugWidget::draw();

        if (!g_bIsShowed) return;

        auto gameData = Glacier::getInterface<Hitman::BloodMoney::ZHM3GameData>(Globals::kGameDataAddr);
        if (!gameData) { return; }

        auto sysInterface = Glacier::getInterface<Glacier::ZSysInterfaceWintel>(Globals::kSysInterfaceAddr);
        if (!sysInterface) { return; }

        auto engineDb = sysInterface->m_engineDataBase;
        if (!engineDb) { return; }

        auto sceneCom = engineDb->GetSceneCom();
        if (!sceneCom) { return; }

        auto hitman = reinterpret_cast<Glacier::ZLNKWHANDS*>(gameData->m_Hitman3);
        if (!hitman) return;

        ImGui::Begin("Teleport Player");
        ImGui::Text("ZHitman3: 0x%.8X", (std::intptr_t)hitman); ImGui::SameLine(0.f, 5.f);
        if (ImGui::Button("PRINT")) {
            spdlog::info("ZHitman3 Instance at: 0x{:08X}", ((std::intptr_t)hitman));
        }

        Glacier::ZVector3 visionPos;
        reinterpret_cast<Glacier::ZCTRLIKLNKOBJ*>(hitman)->GetVisionPos(&visionPos);
        ImGui::Text("Position: (%.3f; %.3f; %.3f)",
                    visionPos.x,
                    visionPos.y,
                    visionPos.z);

        ImGui::Separator();

        static Glacier::ZVector3 teleportAt { 0.f, 0.f, 0.f };
        ImGui::Text("Teleport at: ");
        ImGui::InputFloat3("New position", reinterpret_cast<float*>(&teleportAt)); ImGui::SameLine(0.f, 5.f);
        if (ImGui::Button("TELEPORT")) {
            hitman->MoveToMatPos(reinterpret_cast<const float*>(&hitman->m_baseGeom->m_transform.data),
                                 reinterpret_cast<const float*>(&teleportAt));
        }

        /**
         * @brief Try to ask rTeleportList and check that we have any teleport point to preview it
         */
        Glacier::ZREF rTeleportListREF = 0;
        using CCom = int;

        HF::Hook::VFHook<CCom>::invoke<void, const char*, Glacier::ZREF*>(
                reinterpret_cast<int*>(sceneCom),
                31,
                "rTeleportList",
                &rTeleportListREF);


        auto pTeleportList = Glacier::ZEventBuffer::EventRefToInstance<CTelePortList>(rTeleportListREF);
        if (pTeleportList) {
            ImGui::Separator();
            auto teleportPoints = reinterpret_cast<Glacier::ZLIST*>(pTeleportList->m_geom)->m_entries;
            const int totalTeleportPoints = teleportPoints->Count();
            ImGui::Text("Teleport points (%d):", totalTeleportPoints);

            if (!totalTeleportPoints) {
                ImGui::TextColored(ImVec4 { 1.f, 1.f, 0.f, 1.f }, "No teleport points!");
            } else {
                for (int pointIndex = 0; pointIndex < totalTeleportPoints; ++pointIndex) {
                    Glacier::ZREF pointRef = teleportPoints->GetRefNr(pointIndex);
                    auto refObj = Glacier::ZGEOM::RefToPtr(pointRef);
                    if (refObj) {
                        Glacier::ZMat3x3 transform;
                        Glacier::ZVector3 position;
                        refObj->GetRootTM(&transform, &position);

                        ImGui::Text("#%d | (%.3f; %.3f; %.3f)", pointIndex + 1, position.x, position.y, position.z);
                        ImGui::SameLine(0.f, 4.f);
                        ImGui::PushID(fmt::format("TELEPORT/ACTION/PT#{}", pointIndex).c_str());
                        if (ImGui::Button("TELEPORT")) {
                            hitman->MoveToMatPos(reinterpret_cast<const float*>(&transform),
                                                 reinterpret_cast<const float*>(&position));
                        }
                        ImGui::PopID();
                    } else {
                        ImGui::TextColored(ImVec4 { 1.f, 0.f, 0.f, 1.f }, "#%d | BAD REF", pointIndex + 1);
                    }
                }
            }

            // Add new teleport point
        }

        ImGui::End();
    }

    bool PlayerTeleportWidget::g_bIsShowed = false;
}