#include <BloodMoney/UI/Widgets/SandboxWidget.h>

#include <BloodMoney/Game/ZHM3GameData.h>

#include <BloodMoney/Game/UI/ZHM3MenuElements.h>
#include <BloodMoney/Game/UI/ZHM3MenuFactory.h>
#include <BloodMoney/Game/UI/ZXMLGUISystem.h>
#include <BloodMoney/Game/UI/ZWINDOWS.h>
#include <BloodMoney/Game/UI/ZGUIBase.h>

#include <BloodMoney/Game/Globals.h>

#include <Glacier/Glacier.h>

#include <imgui.h>
#include <spdlog/spdlog.h>

namespace Hitman::BloodMoney
{
    void SandboxWidget::draw()
    {
        ImGui::Begin("Sandbox | Mouse position test");

        {
            auto gameData = Glacier::getInterface<ZHM3GameData>(Globals::kGameDataAddr);
            if (gameData)
            {
                auto menuElements = gameData->m_MenuElements;
                if (menuElements)
                {
                    auto xmlGui = menuElements->m_XMLGUISystem;
                    if (xmlGui)
                    {
                        auto windowsSys = xmlGui->GetSystem();
                        if (windowsSys)
                        {
                            Glacier::Vector2 mousePos { 0.f, 0.f };
                            windowsSys->GetMousePos(&mousePos);

                            ImGui::Text("Current cursor pos: (%.4f;%.4f)", mousePos.x, mousePos.x);
                        } else ImGui::Text("No windows system");
                    } else ImGui::Text("No xml gui system");
                } else ImGui::Text("No menu elements");
            } else ImGui::Text("No game data");
        }

        ImGui::End();

        // Draw others
        DebugWidget::draw();
    }
}