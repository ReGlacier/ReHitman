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
#include <Glacier/ZEntityLocator.h>
#include <Glacier/ZGeomBuffer.h>
#include <Glacier/Glacier.h>
#include <Glacier/ZGROUP.h>
#include <Glacier/ZGEOM.h>

#include <BloodMoney/Game/ZHM3Actor.h>

#include <BloodMoney/UI/GlacierInspectors.h>

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
            auto root = engineDb->m_root;
            ImGui::Inspector<Glacier::ZGROUP>::Draw(root->m_baseGeom->entityName, root);
        }
        ImGui::End();
    }

    bool SandboxWidget::g_bIsVisible = false;
}