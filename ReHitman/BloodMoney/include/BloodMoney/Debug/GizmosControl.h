#pragma once

#include <d3d9.h>
#include <functional>
#include <unordered_map>

namespace Hitman::BloodMoney::Debug
{
    enum class EGizmoLayer {
        Scene_OnEnd,    ///< Called after d3d9 device present called
        ImGui_BeforeUI, ///< Called when ReHitman ready to draw debug UI and debug UI showed
        ImGui_AfterUI   ///< Called when ReHitman finished to draw debug UI and ready to draw anything else in ImGui render stage
    };

    using GizmoHandler = std::function<void(EGizmoLayer, IDirect3DDevice9*)>;

    class GizmosControl {
    public:
        void RegisterGizmo(EGizmoLayer layer, const GizmoHandler& gizmoHandler);

    protected:
        void DrawGizmo(EGizmoLayer layer, IDirect3DDevice9* device);

    private:
        std::unordered_map<EGizmoLayer, std::vector<GizmoHandler>> m_handlers;
    };
}