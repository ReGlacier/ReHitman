#include <BloodMoney/UI/Widgets/CutSequencePlayerWidget.h>

#include <BloodMoney/Editors/CutSequencesPool.h>
#include <BloodMoney/Game/ZCutSequencePlayer.h>

#include <imgui.h>
#include <spdlog/spdlog.h>

namespace Hitman::BloodMoney
{
    void CutSequencePlayerWidget::draw() {
        DebugWidget::draw();

        if (!g_bIsShowed) return;

        if (!CutSequencesPool::GetInstance().IsEmpty())
        {
            ImGui::SetNextWindowSize(ImVec2(200, 300), ImGuiCond_FirstUseEver);

            ImGui::Begin("Cut Sequence Player", &CutSequencePlayerWidget::g_bIsShowed);

            int index = 1;
            CutSequencesPool::GetInstance().ForEach([&index](Hitman::BloodMoney::ZCutSequencePlayer* player) {
                if (ImGui::Button(fmt::format("Play cutscene #{}", index).data()))
                {
                    player->Command(player->m_MSG_CutSequence_StartScene, nullptr);
                }
                ++index;
            });

            if (index == 1)
            {
                ImGui::TextColored(ImVec4 { 1.f, 1.f, 0.f, 1.f }, "No cut sequences here");
            }

            ImGui::End();
        }
    }

    bool CutSequencePlayerWidget::g_bIsShowed = false;
}