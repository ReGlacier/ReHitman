#include <BloodMoney/Delegates/DX9Delegate.h>
#include <BloodMoney/UI/Widgets/SandboxWidget.h>
#include <BloodMoney/UI/DebugTools.h>
#include <BloodMoney/Game/Globals.h>

#include <Glacier/ZSysInterfaceWintel.h>
#include <Glacier/ZRenderWintelD3D.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/Glacier.h>

#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

#include <spdlog/spdlog.h>

namespace Hitman::BloodMoney
{
    namespace Globals
    {
        std::unique_ptr<DebugTools> g_pDebugTools = nullptr;
    }

    void DX9Delegate::OnInitialised(IDirect3DDevice9* device)
    {
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();

        io.MouseDrawCursor = true;
        io.MousePos = ImVec2(0.f, 0.f);
        io.MousePosPrev = io.MousePos;
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

        ImGui::StyleColorsDark();

        auto systemInterface = Glacier::getInterface<Glacier::ZSysInterfaceWintel>(Globals::kSysInterfaceAddr);
        Glacier::ZRenderWintelD3D* renderer = systemInterface ? systemInterface->m_renderer : nullptr;

        const auto hwnd = renderer ? renderer->m_HWND : nullptr;
        if (!hwnd)
            throw std::runtime_error { "Failed to get HWND. No renderer at systemInterface->m_renderer" };

        ImGui_ImplWin32_Init(hwnd);
        ImGui_ImplDX9_Init(device);

        Globals::g_pDebugTools = std::make_unique<DebugTools>();

        {
            Globals::g_pDebugTools->addChild(std::make_shared<SandboxWidget>());
        }

        spdlog::info("DX9Delegate initialised!");
    }

    void DX9Delegate::OnBeginScene(IDirect3DDevice9* device)
    {
        (void)device;
    }

    void DX9Delegate::OnEndScene(IDirect3DDevice9* device)
    {
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (Globals::g_pDebugTools)
        {
            const bool isDebugToolsVisible = Globals::g_pDebugTools->isVisible();

            if (isDebugToolsVisible)
            {
                Globals::g_pDebugTools->draw();
            }

            ImGuiIO& io = ImGui::GetIO();
            io.MouseDrawCursor = isDebugToolsVisible;
        }

        ImGui::EndFrame();
        device->SetRenderState(D3DRS_ZENABLE, false);
        device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
        device->SetRenderState(D3DRS_SCISSORTESTENABLE, false);

        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    }

    void DX9Delegate::OnDeviceLost()
    {
        ImGui_ImplDX9_InvalidateDeviceObjects();
    }

    void DX9Delegate::OnReset(IDirect3DDevice9* device)
    {
        (void)device;
    }

    void DX9Delegate::OnDeviceRestored(IDirect3DDevice9* device)
    {
        ImGui_ImplDX9_CreateDeviceObjects();
    }
}