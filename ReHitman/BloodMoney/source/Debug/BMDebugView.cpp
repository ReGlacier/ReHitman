#include <BloodMoney/Debug/BMDebugView.h>
#include <BloodMoney/Debug/BMDebugModel.h>

#include <imgui.h>

namespace Hitman::BloodMoney::DebugUI
{
    void BMDebugView::SetModel(BMDebugModel* model)
    {
        if (IsDrawInProgress())
        {
            m_modelToSwap = model;
        }
        else
        {
            m_model = model;
        }
    }

    void BMDebugView::OnDrawBegin()
    {
        Base::OnDrawBegin();
        // --- Draw your debug stuff (ImGui allowed) ---
        ImGui::Begin("Hello Hitman!");
        ImGui::TextColored(ImVec4 { 0.f, 1.f, 0.f, 1.f }, "HELLO WORLD!");
        ImGui::End();
    }

    void BMDebugView::OnDrawEnd()
    {
        Base::OnDrawEnd();
        // --- End of rendering  ---
        if (m_modelToSwap)
        {
            m_model = m_modelToSwap;
            m_modelToSwap = nullptr;
        }
    }
}