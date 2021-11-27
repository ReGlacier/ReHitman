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
#include <Glacier/ZPrimControlWintel.h>
#include <Glacier/ZRenderWintelD3DDll.h>

namespace Hitman::BloodMoney::Debug {
    void ActorDebugGizmo::OnDrawGizmo(EGizmoLayer layer, IDirect3DDevice9 *device) {
        /// ---- INTERFACES & DATA
        auto systemInterface = Glacier::getInterface<Glacier::ZSysInterfaceWintel>(Globals::kSysInterfaceAddr);
        if (!systemInterface) return;

        auto gameData = Glacier::getInterface<Hitman::BloodMoney::ZHM3GameData>(Globals::kGameDataAddr);
        if (!gameData || !gameData->m_Hitman3 || !gameData->m_ActorsInPoolCount) return;

        auto pCamera = reinterpret_cast<Glacier::ZCAMERA*>(gameData->m_CameraClass);
        if (!pCamera || !pCamera->IsActive()) return;

        auto pD3DDll = Glacier::getInterface<Glacier::ZRenderWintelD3DDll>(Globals::kD3DDllAddr);
        if (!pD3DDll) return;

        auto pPrimControl = pD3DDll->m_primControlWintel;
        if (!pPrimControl) return;

        Glacier::ZCameraSpace cameraSpace {};
        cameraSpace = pCamera;

        using vmmul_t = void(__fastcall*)(Glacier::Vector3*, Glacier::Vector3*, Glacier::ZMat3x3*);
        auto vmmul = (vmmul_t)0x00435F50;

        using v3add_t = void(__fastcall*)(Glacier::ZVector3*,Glacier::ZVector3*);
        auto v3add = (v3add_t)0x00428740;

        struct ZHitman3AutoAim_t {
            int pad0 { 0 };
            Glacier::ZCAMERA* pCamera { nullptr };
        };

        using ZHitman3AutoAim_GetScreenCoord_t = bool(__thiscall*)(ZHitman3AutoAim_t*,Hitman::BloodMoney::ZHM3Actor*,Glacier::ZVector3*);
        auto ZHitman3AutoAim_GetScreenCoord = (ZHitman3AutoAim_GetScreenCoord_t)0x00631A80;

        ZHitman3AutoAim_t autoAim {};
        autoAim.pCamera = pCamera;

        // And now try to draw actor's model
        for (int iActor = 0; iActor < gameData->m_ActorsInPoolCount; iActor++) {
            Hitman::BloodMoney::ZHM3Actor* pActor = gameData->m_ActorsPool[iActor];

            if (!pActor->m_boneModify) {
                // Actor not loaded completely. It's better to avoid work with it
                continue;
            }

            Glacier::Vector3 position;
            if (!ZHitman3AutoAim_GetScreenCoord(&autoAim, pActor, &position)) {
                continue;
            }

            //FIXME: Here we have wrong calculations of position' transform in 2d space. I will fix it later
            ImGui::GetOverlayDrawList()->AddCircleFilled(ImVec2 { position.x, position.y }, 5.f, IM_COL32(255, 0, 0, 255));
        }
    }
}