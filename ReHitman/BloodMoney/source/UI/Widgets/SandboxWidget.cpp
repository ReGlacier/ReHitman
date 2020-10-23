#include <BloodMoney/UI/Widgets/SandboxWidget.h>

#include <BloodMoney/Game/ZHM3GameData.h>
#include <BloodMoney/Game/ZHM3MenuElements.h>
#include <BloodMoney/Game/ZHM3MenuFactory.h>
#include <BloodMoney/Game/Globals.h>
#include <BloodMoney/Game/ZGUIBase.h>

#include <Glacier/Glacier.h>

#include <imgui.h>
#include <spdlog/spdlog.h>

namespace Hitman::BloodMoney
{
    void SandboxWidget::draw()
    {
        ImGui::Begin("Sandbox");

        if (ImGui::Button("Try to create UI element"))
        {
            auto gameData = Glacier::getInterface<BloodMoney::ZHM3GameData>(Globals::kGameDataAddr);
            if (gameData)
            {
                auto buildID = gameData->m_MenuElements->m_hm3MenuFactory->GUIElementFactory("BuildID");
                buildID->m_position.x = 100.f;
                buildID->m_position.y = 100.f;
            }
        }

        ImGui::End();

        // Draw others
        DebugWidget::draw();
    }
}