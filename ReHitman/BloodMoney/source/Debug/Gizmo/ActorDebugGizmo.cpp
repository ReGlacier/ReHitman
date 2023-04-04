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
        return;

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

        Glacier::ZMat3x3 mHeadMat, mPlayerMat;
        Glacier::ZVector3 vHeadPos, vPlayerPos;
        Glacier::ZVector2 vHeadPos4;

        auto pPlayer = reinterpret_cast<Glacier::ZLNKWHANDS*>(gameData->m_Hitman3);
        if (!pPlayer->m_boneModify) return;

        pPlayer->GetRootCenter(&mPlayerMat, &vPlayerPos);

        // Get self bone
        auto iHeadBoneIdx = pPlayer->HeadBoneIndex();
        pPlayer->GetBoneMatPos(&mHeadMat, &vHeadPos, iHeadBoneIdx);

        // Envy
        ImGui::Begin("Player debug");
        {
            ImGui::Text("Head bone index: %u", iHeadBoneIdx);
            ImGui::InputFloat3("Head pos : ", &vHeadPos.x);
            ImGui::InputFloat2("Head pos4: ", &vHeadPos4.x);
            ImGui::InputFloat3("Head mat[0]: ", &mHeadMat.data[0]);
            ImGui::InputFloat3("Head mat[1]: ", &mHeadMat.data[3]);
            ImGui::InputFloat3("Head mat[2]: ", &mHeadMat.data[6]);
            ImGui::Separator();
            ImGui::InputFloat3("Player pos: ", &vPlayerPos.x);
            ImGui::InputFloat3("Player mat[0]: ", &mPlayerMat.data[0]);
            ImGui::InputFloat3("Player mat[1]: ", &mPlayerMat.data[3]);
            ImGui::InputFloat3("Player mat[2]: ", &mPlayerMat.data[6]);

            if (ImGui::Button("Rotate head by 90"))
            {
                ((void(__fastcall*)(Glacier::ZMat3x3*, int, float, float, float, float))0x00435B40)(&mHeadMat, 0, 90.f, vHeadPos.x, vHeadPos.y, vHeadPos.z);
            }
        }
        ImGui::End();
    }
}