#include <BloodMoney/Delegates/DX9Delegate.h>
#include <BloodMoney/UI/DebugTools.h>

// Debug widgets
#include <BloodMoney/UI/Widgets/SandboxWidget.h>
#include <BloodMoney/UI/Widgets/ActorsListWidget.h>
#include <BloodMoney/UI/Widgets/CutSequencePlayerWidget.h>
#include <BloodMoney/UI/Widgets/SceneViewer.h>
#include <BloodMoney/UI/Widgets/PlayerTeleportWidget.h>

// Gizmos
#include <BloodMoney/Debug/Gizmo/ActorsDebugGizmo.h>

#include <BloodMoney/Game/Globals.h>
#include <BloodMoney/Game/ZHM3Actor.h>
#include <BloodMoney/Game/ZHM3GameData.h>

#include <Glacier/ZSysInterfaceWintel.h>
#include <Glacier/ZRenderWintelD3D.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/ZCameraSpace.h>
#include <Glacier/ZCAMERA.h>
#include <Glacier/Glacier.h>

#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

#include <spdlog/spdlog.h>
#include <HF/HackingFramework.hpp>
#include <Glacier/ZPrimControlWintel.h>
#include <Glacier/ZRenderWintelD3DDll.h>

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

        // Register widgets
        {
            Globals::g_pDebugTools->addChild(std::make_shared<SandboxWidget>());
            Globals::g_pDebugTools->addChild(std::make_shared<ActorsListWidget>());
            Globals::g_pDebugTools->addChild(std::make_shared<CutSequencePlayerWidget>());
            Globals::g_pDebugTools->addChild(std::make_shared<SceneViewer>());
            Globals::g_pDebugTools->addChild(std::make_shared<PlayerTeleportWidget>());
        }

        // Register gizmos
        {
            RegisterGizmo(Debug::EGizmoLayer::ImGui_AfterUI, Debug::ActorDebugGizmo::OnDrawGizmo);
        }

        spdlog::info("DX9Delegate initialised!");
    }

    void DX9Delegate::OnDeviceLost()
    {
        ImGui_ImplDX9_InvalidateDeviceObjects();
    }

    void DX9Delegate::OnReset(IDirect3DDevice9* /*device*/)
    {
    }

    void DX9Delegate::OnDeviceRestored(IDirect3DDevice9* device)
    {
        ImGui_ImplDX9_CreateDeviceObjects();
    }

    void DX9Delegate::OnPresent(IDirect3DDevice9* device)
    {
        DrawGizmo(Debug::EGizmoLayer::Scene_OnEnd, device);
        DrawDebugUI(device);
    }

    void DX9Delegate::DrawDebugUI(IDirect3DDevice9* device) {
        ImGuiIO& io = ImGui::GetIO();

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();

        if (io.DeltaTime <= 0.f) {
            io.DeltaTime = 1.f / 60.f;
        }

        ImGui::NewFrame();

        if (Globals::g_pDebugTools)
        {
            const bool isDebugToolsVisible = Globals::g_pDebugTools->isVisible();

            if (isDebugToolsVisible)
            {
                DrawGizmo(Debug::EGizmoLayer::ImGui_BeforeUI, device);
                Globals::g_pDebugTools->draw();
                DrawGizmo(Debug::EGizmoLayer::ImGui_AfterUI, device);
            }

            io.MouseDrawCursor = isDebugToolsVisible;
        }

        ImGui::EndFrame();
        device->SetRenderState(D3DRS_ZENABLE, false);
        device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
        device->SetRenderState(D3DRS_SCISSORTESTENABLE, false);

        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    }
}