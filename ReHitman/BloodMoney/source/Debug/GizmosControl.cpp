#include <BloodMoney/Debug/GizmosControl.h>

namespace Hitman::BloodMoney::Debug {
    void GizmosControl::RegisterGizmo(EGizmoLayer layer, const GizmoHandler& gizmoHandler) {
        m_handlers[layer].push_back(gizmoHandler);
    }

    void GizmosControl::DrawGizmo(EGizmoLayer layer, IDirect3DDevice9* device) {
        auto it = m_handlers.find(layer);
        if (it != m_handlers.end()) {
            for (const auto& handler: it->second) {
                handler(layer, device);
            }
        }
    }
}