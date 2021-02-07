#include <spdlog/spdlog.h>
#include <imgui.h>

#include <BloodMoney/UI/Widgets/SandboxWidget.h>

#include <BloodMoney/Game/ZHM3GameData.h>

#include <BloodMoney/Game/UI/ZHM3MenuElements.h>
#include <BloodMoney/Game/UI/ZHM3MenuFactory.h>
#include <BloodMoney/Game/UI/ZXMLGUISystem.h>
#include <BloodMoney/Game/UI/ZWINDOWS.h>
#include <BloodMoney/Game/UI/ZGUIBase.h>

#include <BloodMoney/Game/Globals.h>

#include <Glacier/ZSysInterfaceWintel.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/Geom/ZEntityLocator.h>
#include <Glacier/Geom/ZGeomBuffer.h>
#include <Glacier/Glacier.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/IK/ZLNKOBJ.h>

#include <BloodMoney/Game/ZHM3Actor.h>

#include <BloodMoney/UI/GlacierInspectors.h>
#include <BloodMoney/Game/CIngameMap.h>

namespace Hitman::BloodMoney
{
    void SandboxWidget::draw()
    {
        DebugWidget::draw();

        auto gameData = Glacier::getInterface<Hitman::BloodMoney::ZHM3GameData>(Globals::kGameDataAddr);
        if (!gameData) { return; }

        auto sysInterface = Glacier::getInterface<Glacier::ZSysInterfaceWintel>(Globals::kSysInterfaceAddr);
        if (!sysInterface) { return; }

        auto engineDb = sysInterface->m_engineDataBase;
        if (!engineDb) { return; }

        auto geomBuffer = Glacier::getInterface<Glacier::ZGeomBuffer>(Globals::kGeomBufferAddr);
        if (!geomBuffer) { return; }

        ImGui::Begin("Sandbox (scene tree view)");
        {
            /// Let's make a party
            if (ImGui::Button("Make everybody like 47")) {
                auto pPlayerLNK = reinterpret_cast<Glacier::ZLNKOBJ*>(gameData->m_Hitman3);

                for (size_t i = 0; i < gameData->m_ActorsInPoolCount; i++) {
                    auto pActorLNK = reinterpret_cast<Glacier::ZLNKOBJ*>(gameData->m_ActorsPool[i]);
                    pActorLNK->CopyGeometryFrom(pPlayerLNK);
                }
            }
        }
        ImGui::End();
    }

    bool SandboxWidget::g_bIsVisible = false;
}