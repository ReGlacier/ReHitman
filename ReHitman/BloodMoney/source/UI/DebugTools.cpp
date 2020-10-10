#include <BloodMoney/UI/DebugTools.h>

#include <imgui.h>

namespace Hitman::BloodMoney
{
    void DebugTools::Draw()
    {
        ImGui::Begin("Test");
        ImGui::TextColored(ImVec4 { 0.f, 1.f, 0.f, 1.f }, "I'm online!");
        ImGui::End();
    }
}