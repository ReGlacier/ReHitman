#pragma once

#include <BloodMoney/Debug/GizmosControl.h>

namespace Hitman::BloodMoney::Debug {
    struct ActorDebugGizmo {
        static void OnDrawGizmo(EGizmoLayer layer, IDirect3DDevice9* device);
    };
}