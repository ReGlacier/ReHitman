#include <spdlog/spdlog.h>
#include <imgui.h>

#include <BloodMoney/UI/Widgets/SandboxWidget.h>
#include <BloodMoney/UI/GlacierInspectors.h>

#include <BloodMoney/Game/ZHM3GameData.h>
#include <BloodMoney/Game/ZHM3DialogControl.h>

#include <BloodMoney/Game/UI/ZHM3MenuElements.h>
#include <BloodMoney/Game/UI/ZHM3MenuFactory.h>
#include <BloodMoney/Game/UI/ZXMLGUISystem.h>
#include <BloodMoney/Game/UI/ZWINDOWS.h>
#include <BloodMoney/Game/UI/ZGUIBase.h>

#include <BloodMoney/Game/Globals.h>

#include <Glacier/ZSysInterfaceWintel.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/Geom/ZEntityLocator.h>
#include <Glacier/Geom/ZGeomBuffer.h>
#include <Glacier/Glacier.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/CInventory.h>
#include <Glacier/CConfiguration.h>
#include <Glacier/ZSTL/REFTAB32.h>
#include <Glacier/Items/ZItem.h>
#include <Glacier/Items/ZItemTemplate.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/Geom/ZTreeGroup.h>
#include <Glacier/ZTypeTraits.h>
#include <Glacier/ZRenderWintelD3D.h>

#include <BloodMoney/Game/ZHM3Actor.h>

#include <BloodMoney/Game/CIngameMap.h>
#include <BloodMoney/Game/ZHM3HmAs.h>
#include <BloodMoney/Game/ZHM3ClothBundle.h>
#include <BloodMoney/Game/UI/ZCHAROBJ.h>
#include <BloodMoney/Game/UI/ZWINOBJ.h>
#include <BloodMoney/Game/UI/ZXMLGUISystem.h>

#include <BloodMoney/Game/ZCloth.h>
#include <BloodMoney/Game/ZTie.h>
#include <BloodMoney/Game/UI/ZLINEOBJ.h>

#include <HF/HackingFramework.hpp>
#include <Glacier/ZRenderWintelD3DDll.h>
#include <Glacier/ZPrimControlWintel.h>
#include <Glacier/ZEngineGeomControl.h>

#include <BloodMoney/Game/OnLevel/ZVCR.h>


namespace Hitman::BloodMoney
{

    void SandboxWidget::draw()
    {
        DebugWidget::draw();

        auto gameData = Glacier::getInterface<Hitman::BloodMoney::ZHM3GameData>(Globals::kGameDataAddr);
        if (!gameData) { return; }

        auto sysInterface = Glacier::getInterface<Glacier::ZSysInterfaceWintel>(Globals::kSysInterfaceAddr);
        if (!sysInterface) { return; }

        auto engineDb = sysInterface->m_engineDataBase;
        if (!engineDb) { return; }

        auto renderDll = Glacier::getInterface<Glacier::ZRenderWintelD3DDll>(Globals::kD3DDllAddr);
        if (!renderDll) { return; }

        auto geomBuffer = Glacier::getInterface<Glacier::ZGeomBuffer>(Globals::kGeomBufferAddr);
        if (!geomBuffer) { return; }

        ImGui::Begin("Test script");
        if (ImGui::Button("Try to dump few prims")) {
            spdlog::info("Hitman   prim {:08X}", (int)renderDll->m_primControlWintel->GetPrimData(reinterpret_cast<Glacier::ZGEOM*>(gameData->m_Hitman3)->m_baseGeom->m_primitive));
            spdlog::info("Actor[0] prim {:08X}", (int)renderDll->m_primControlWintel->GetPrimData(gameData->m_ActorsPool[0]->m_baseGeom->m_primitive));
            spdlog::info("Actor[1] prim {:08X}", (int)renderDll->m_primControlWintel->GetPrimData(gameData->m_ActorsPool[1]->m_baseGeom->m_primitive));

            char buffer[512] { 0 };

            using GetPrimStats_t = int(__thiscall*)(int, char*, int, int);
            auto GetPrimStats = (GetPrimStats_t)0x00487740;

            auto r = GetPrimStats((int)renderDll->m_primControlWintel, &buffer[0], 511, reinterpret_cast<Glacier::ZGEOM*>(gameData->m_Hitman3)->m_baseGeom->m_primitive);
            spdlog::info("R: {:08X}| {}", r, buffer);
            std::memset(&buffer[0], 0x0, 512);

            for (int i = 0; i < gameData->m_ActorsInPoolCount; i++) {
                GetPrimStats((int)renderDll->m_primControlWintel, &buffer[0], 511, gameData->m_ActorsPool[i]->m_baseGeom->m_primitive);
                spdlog::info("Actor[{}]: {}", i, buffer);
                std::memset(&buffer[0], 0x0, 512);
            }
        }

        ImGui::End();

        //-----------------------------------------------------------------------------------------------
        ImGui::Begin("TEST");
        if (ImGui::Button("Do smth")) {
            using Func_t = int(__cdecl*)(const char*);
            auto func = (Func_t)0x006A8960;

            Glacier::CConfiguration::SetCanShowSubtitles(true);
            func("UI HACKED)))");
        }

//        if (ImGui::Button("Test VTBL")) {
//            spdlog::info("WIDTH: {}", gameData->m_OSD->m_pSubtitles->GetWidth());
//            gameData->m_OSD->m_pSubtitles->SetWidth(300);
//            Glacier::Vector2 sv;
//            sv.x = 1.7f;
//            sv.y = 1.7f;
//            Glacier::Vector3 clr;
//            clr.x = 1.f;
//            clr.y = 0.f;
//            clr.z = 0.f;
//            gameData->m_OSD->m_pSubtitles->SetScale(&sv, true);
//            gameData->m_OSD->m_pSubtitles->SetColor(&clr);
//            spdlog::info("WIDTH_2: {}", gameData->m_OSD->m_pSubtitles->GetWidth());
//        }
//
//        if (ImGui::Button("Make fun")) {
//            reinterpret_cast<Glacier::ZIKLNKOBJ*>(gameData->m_Hitman3)->ActivateRagdoll(true, true, true);
//        }

        {
            using ZHitman3_GetCurrentElevatorREF_t = Glacier::ZREF(__thiscall*)(void*);
            auto ZHitman3_GetCurrentElevatorREF = (ZHitman3_GetCurrentElevatorREF_t)0x00600EF0;

            Glacier::ZREF elevatorREF = ZHitman3_GetCurrentElevatorREF(gameData->m_Hitman3);
            auto pElevatorGroup = elevatorREF ? reinterpret_cast<Glacier::ZGROUP*>(Glacier::ZGEOM::RefToPtr(elevatorREF)) : nullptr;

            if (pElevatorGroup) {
                if (ImGui::Button("Pause current elevator")) {
                    // 1. Take MSG from engine db
                    auto msg = engineDb->RegisterZMsg("PauseElevator", 0, __FILE__, __LINE__);

                    // 2. Send command into the group recursive
                    pElevatorGroup->SendCommandRecursive(msg, nullptr, nullptr);
                }

                if (ImGui::Button("UnPause current elevator")) {
                    // 1. Take MSG from engine db
                    auto msg = engineDb->RegisterZMsg("UnPauseElevator", 0, __FILE__, __LINE__);

                    // 2. Send command into the group recursive
                    pElevatorGroup->SendCommandRecursive(msg, nullptr, nullptr);
                }
            }
        }

        if (ImGui::Button("Show custom message")) {
            gameData->m_OSD->AddInfo("This is very important message for you because you are playing in ReHitman!", true);
        }

        if (ImGui::Button("Show custom warning")) {
            gameData->m_OSD->AddWarning("This is very important message for you because you are playing in ReHitman!", true);
        }

        if (ImGui::Button("Show custom hint")) {
            gameData->m_OSD->AddHint("New player connected", false, true, 0, false, nullptr);
        }

        if (ImGui::Button("Pause all actors scripts")) {
            for (int i = 0; i < gameData->m_ActorsInPoolCount; i++) {
                auto actorRef = gameData->m_ActorsPool[i]->GetRef();
                ((void(__cdecl*)(Glacier::ZREF, bool))0x0059C9D0)(actorRef, true);
            }
        }

        ImGui::End();
    }

    bool SandboxWidget::g_bIsVisible = false;
}