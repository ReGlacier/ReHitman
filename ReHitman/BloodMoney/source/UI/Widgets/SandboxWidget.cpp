#include <BloodMoney/UI/Widgets/SandboxWidget.h>

#include <BloodMoney/Game/ZHM3GameData.h>

#include <BloodMoney/Game/UI/ZHM3MenuElements.h>
#include <BloodMoney/Game/UI/ZHM3MenuFactory.h>
#include <BloodMoney/Game/UI/ZXMLGUISystem.h>
#include <BloodMoney/Game/UI/ZWINDOWS.h>
#include <BloodMoney/Game/UI/ZGUIBase.h>

#include <BloodMoney/Game/Globals.h>

#include <Glacier/Glacier.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/ZSysInterfaceWintel.h>
#include <Glacier/LINKSORTREFTAB.h>

#include <imgui.h>
#include <spdlog/spdlog.h>

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

        ImGui::Begin("Sandbox");
        // Your code here
        ImGui::End();
    }
}