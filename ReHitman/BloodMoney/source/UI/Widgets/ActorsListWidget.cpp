#include <BloodMoney/UI/Widgets/ActorsListWidget.h>

#include <BloodMoney/Game/PF/ZPathFollower.h>
#include <BloodMoney/Game/ZHM3GameData.h>
#include <BloodMoney/Game/ZHM3Actor.h>
#include <BloodMoney/Game/Globals.h>
#include <BloodMoney/Game/ZGuardQuarterController.h>
#include <BloodMoney/Game/PF4/PF4RunTime.h>
#include <BloodMoney/Game/ZScriptC.h>
#include <BloodMoney/Game/CCheat.h>

#include <BloodMoney/UI/ImGuiInspector.h>
#include <BloodMoney/UI/GlacierInspectors.h>
#include <BloodMoney/UI/BloodMoneyInspectors.h>

#include <Glacier/ZRenderWintelD3DDll.h>
#include <Glacier/ZSysInterfaceWintel.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/Geom/ZGeomBuffer.h>
#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/IK/ZIKLNKOBJ.h>
#include <Glacier/CInventory.h>
#include <Glacier/EventBase/ZEventBuffer.h>
#include <Glacier/Geom/ZGeomBuffer.h>
#include <Glacier/ZActorCommunication.h>

#include <Glacier/Fysix/CRigidBody.h>
#include <Glacier/Geom/ZROOM.h>
#include <HF/HackingFramework.hpp>
#include <imgui.h>

namespace ImGui
{
    void Inspector<Hitman::BloodMoney::ZHM3Actor>::Draw(const char* id, Hitman::BloodMoney::ZHM3Actor* actor) {
        // Base actor info
        ImGui::Inspector<Glacier::ZEntityLocator>::Draw("actor.entity", actor->m_baseGeom);
        ImGui::Separator();

        // Another info
        ImGui::Text("m_fHitpoints: %f", actor->m_fHitpoints);
        ImGui::Text("m_fTension: %f", actor->m_fTension);

        // Components

        // ==================
        {
            if (ImGui::Button("Clear PF4 path"))
            {
                using PF4_Path_Clear_t = void(__thiscall*)(std::intptr_t*);
                auto PF4_Path_Clear = (PF4_Path_Clear_t)0x004D8D00;

                PF4_Path_Clear(reinterpret_cast<std::intptr_t*>(&actor->m_Path));

                actor->PreparePath();
            }
        }

        if (auto path = reinterpret_cast<Hitman::BloodMoney::PF4RunTime::ZPath*>(reinterpret_cast<std::intptr_t*>(&actor->m_Path)); path && path->m_Size > 1)
        {
            using ZPlayer_GetCamera_t = Glacier::ZCAMERA* (__thiscall*)(int*);
            using GetLocalPoint_t = void(__thiscall*)(Glacier::ZCAMERA* pCamera, Glacier::ZVector3* pInOutPoint);
            using Proj2D_t = void(__thiscall*)(Glacier::ZCAMERA* pCamera, Glacier::ZVector2* pOutScreen, Glacier::ZVector3* pInLocalPoint);

            ZPlayer_GetCamera_t ZPlayer_GetCamera = (ZPlayer_GetCamera_t)0x00528AA0;
            GetLocalPoint_t GetLocalPoint = (GetLocalPoint_t)0x004E6920;
            Proj2D_t Proj2D = (Proj2D_t)0x004E40A0;

            auto gameData = Glacier::getInterface<Hitman::BloodMoney::ZHM3GameData>(Hitman::BloodMoney::Globals::kGameDataAddr);
            auto cam = ZPlayer_GetCamera((int*)gameData->m_Hitman3);
            Glacier::ZMat3x3 camMat;
            Glacier::ZVector3 camPos;
            cam->GetMatPos(&camMat, &camPos);

            for (int i = 1; i < path->m_Size; ++i)
            {
                ImVec2 l0, l1;
                Glacier::ZVector2 v0, v1;
                Glacier::ZVector3 p0, p1;

                path->GetPosition(i - 1, p0);
                path->GetPosition(i - 0, p1);

                GetLocalPoint(cam, &p0);
                GetLocalPoint(cam, &p1);

                Proj2D(cam, &v0, &p0);
                Proj2D(cam, &v1, &p1);

                ImVec2 display_size = ImGui::GetIO().DisplaySize;
                float cx = display_size.x * 0.5f;
                float cy = display_size.y * 0.5f;

                float custom_scale = 2.0f;

                l0.x = cx + (v0.x * cx * custom_scale);
                l0.y = cy + (v0.y * cy * custom_scale);

                l1.x = cx + (v1.x * cx * custom_scale);
                l1.y = cy + (v1.y * cy * custom_scale);

                ImGui::GetForegroundDrawList()->AddLine(l0, l1, 0xFF'00'FF'FF, 2.f);
            }
        }

        // ===================
        {
            auto gameData = Glacier::getInterface<Hitman::BloodMoney::ZHM3GameData>(Hitman::BloodMoney::Globals::kGameDataAddr);
            // NOTE: Need to check ability to start human shield by another way
            if (gameData && ImGui::Button("Start human shield with"))
            {
                using StartActionHumanShield_t = void(__thiscall*)(std::intptr_t*, Hitman::BloodMoney::ZHM3Actor*);
                auto StartActionHumanShield = (StartActionHumanShield_t)0x00600590;

                StartActionHumanShield(reinterpret_cast<std::intptr_t*>(gameData->m_Hitman3), actor);
            }
        }

        // ===================
        {
            if (ImGui::Button("Die"))
            {
                actor->Die();
            }

            ImGui::SameLine();
        }

        {
            if (ImGui::Button("Knockout"))
            {
                actor->Knockout();
            }

            ImGui::SameLine();
        }

		{
			if (ImGui::Button("Revive"))
			{
				actor->Revive();
			}
		}

        // ===================
        {
            auto gameData = Glacier::getInterface<Hitman::BloodMoney::ZHM3GameData>(Hitman::BloodMoney::Globals::kGameDataAddr);
            if (gameData && gameData->m_Hitman3 && ImGui::Button("Run to player"))
            {
                Glacier::ZVector3 pos;
                reinterpret_cast<Glacier::ZCTRLIKLNKOBJ*>(gameData->m_Hitman3)->GetVisionPos(&pos);

                actor->SetMoveSpeedMultiplier(15.f);
                actor->MoveToPosition(&pos, &pos);

                spdlog::info("Request to actor '{}' run to {};{};{}", actor->m_baseGeom->m_Name, pos.x, pos.y, pos.z);
            }
        }

        // ===================
        {
            auto gameData = Glacier::getInterface<Hitman::BloodMoney::ZHM3GameData>(Hitman::BloodMoney::Globals::kGameDataAddr);
            // TODO: Find another way to check ability of clone op
            if (gameData && ImGui::Button("Make clone"))
            {
                auto actorOwnerGroup = reinterpret_cast<Glacier::ZGEOM*>(actor)->m_baseGeom->ParentGroup();
                auto actorRootGroup = reinterpret_cast<Glacier::ZGEOM*>(actorOwnerGroup)->m_baseGeom->ParentGroup();

                spdlog::info("Owner: {} / Root: {}",
                             reinterpret_cast<Glacier::ZGEOM*>(actorOwnerGroup)->m_baseGeom->m_Name,
                             reinterpret_cast<Glacier::ZGEOM*>(actorRootGroup)->m_baseGeom->m_Name);

                Glacier::ZMat3x3 ownerMat;
                Glacier::ZVector3 ownerPos;

                spdlog::info("<< GetMatPos");
                actorOwnerGroup->GetMatPos(&ownerMat, &ownerPos);
                spdlog::info("<< DuplicateInit");
                auto duplicateGroup = actorOwnerGroup->DuplicateInit(
                        actorRootGroup,
                        &ownerMat,
                        &ownerPos,
                        fmt::format("Actor!CloneOf!{}", reinterpret_cast<Glacier::ZGEOM*>(actorOwnerGroup)->m_baseGeom->m_Name).c_str(),
                        true);

                spdlog::info("<< FindGeom(Ground)");
                auto clonedActor = reinterpret_cast<Hitman::BloodMoney::ZHM3Actor*>(reinterpret_cast<Glacier::ZGROUP*>(duplicateGroup)->FindGeom("Ground", nullptr));
                
                spdlog::info("ClonedActor: {:08X}", reinterpret_cast<std::intptr_t>(clonedActor));
                spdlog::info("<< InitMapIcon");
                clonedActor->m_eCharacterType = Hitman::BloodMoney::eCharacterType::eCharacterType_VIP;
                clonedActor->InitMapIcon(true);

                spdlog::info("<< CopyPoseFrom");
                reinterpret_cast<Glacier::ZLNKOBJ*>(clonedActor)->CopyPoseFrom(reinterpret_cast<Glacier::ZLNKOBJ*>(actor));

                // Add actor to tracking list
                spdlog::info("<< pTrackLinkObjects->AddGeom");
                auto pTrackLinkObjects = *(Glacier::ZLIST**)0x00972DA8;
                pTrackLinkObjects->AddGeom(reinterpret_cast<Glacier::ZGEOM*>(clonedActor));

                spdlog::info("<< EnableIK");
                reinterpret_cast<Glacier::ZIKLNKOBJ*>(clonedActor)->EnableIK();

                spdlog::info("<< EnableControls");
                reinterpret_cast<Glacier::ZIKLNKOBJ*>(clonedActor)->EnableControls();

                // Actorcommunication__Registerradiouser
                spdlog::info("<< Will ask for ref");
                auto ref = reinterpret_cast<Glacier::ZGEOM*>(clonedActor)->GetRef();
                spdlog::info("<< Actorcommunication__Registerradiouser (ref {})", ref);
                ((void(__cdecl*)(Glacier::ZREF, int))0x006AA2B0)(reinterpret_cast<Glacier::ZGEOM*>(clonedActor)->GetRef(), 0);

                spdlog::info("<< SetActorState");
                clonedActor->SetActorState(Hitman::BloodMoney::ZActor::ACTORSTATE::ACTORSTATE_SLEEPING);

                // ----------- PRETTY PRINT SOME INFOS -------------
                spdlog::info("TRK: {:08X}", (int)pTrackLinkObjects);
                spdlog::info("OACT: {:08X}", (int)actor);
                spdlog::info("Dup: {:08X} / ADup: {:08X}", (int)duplicateGroup, (int)clonedActor);

                // Just for debug (copy state from original actor)
                clonedActor->SetActorState(((Hitman::BloodMoney::ZActor::ACTORSTATE(__thiscall*)(Hitman::BloodMoney::ZHM3Actor*))0x005029A0)(actor)); // ZActor::GetActorState

                spdlog::info("Cloned actor ptr is {:08X}", reinterpret_cast<std::intptr_t>(clonedActor));

                // ------------ ENABLE AI SCRIPTS ------------
                spdlog::info("TRK: {:08X}", (int)pTrackLinkObjects);
                spdlog::info("OACT: {:08X}", (int)actor);
                spdlog::info("Dup: {:08X} / ADup: {:08X}", (int)duplicateGroup, (int)clonedActor);

                Hitman::BloodMoney::ZScriptC* pClonedActorScript = nullptr;
                Glacier::CInventory* pClonedActorInventory = nullptr;

                {
                    int* pDefaultStatus = Glacier::ZEventBase::GetDefaultStatus();
                    const int oldDefaultStatus = *pDefaultStatus;

                    // Here we need to change default status to fix ZScriptC event creation
                    *pDefaultStatus = 1;

                    //TODO: Here we need to fix ZGEOM vftable. One method is lost between FindEvent and AddEvent
                    pClonedActorInventory = HF::Hook::VFHook<Hitman::BloodMoney::ZHM3Actor>::invoke<Glacier::CInventory*, const char*>(clonedActor, 66, "ZGEOM_Inventory"); // Add inventory
                    pClonedActorScript = HF::Hook::VFHook<Hitman::BloodMoney::ZHM3Actor>::invoke<Hitman::BloodMoney::ZScriptC*, const char*>(clonedActor, 66, "ZGEOM_ScriptC"); // AddEvent

					// And PathFollower
					HF::Hook::VFHook<Hitman::BloodMoney::ZHM3Actor>::invoke<Hitman::BloodMoney::ZScriptC*, const char*>(clonedActor, 66, "ZGEOM_PathFollower"); // AddEvent

                    // And don't forget to restore it back to avoid other issues
                    *pDefaultStatus = oldDefaultStatus;
                }

                if (!pClonedActorScript) {
                    spdlog::error("Failed to add ZScriptC to cloned actor");
                } else {
                    spdlog::info("Created & registered ZScriptC: {:08X}", (int)pClonedActorScript);

                    static const char* psRequiredScriptName = "Alllevels_Armed";
                    auto* foundScript = pClonedActorScript->FindScript(psRequiredScriptName);
                    if (!foundScript) {
                        spdlog::error("Failed to find '{}' script!", psRequiredScriptName);
                    } else {
                        // And then call 'create script'
						spdlog::info("Script found, start attach...");

                        pClonedActorScript->m_pScriptCreator = pClonedActorScript->CreateScript(foundScript);
                        spdlog::info("AI script attached ({:08X})", (std::intptr_t)(pClonedActorScript->m_pScriptCreator));

                        // Here we need to call internal methods
                        clonedActor->Activate(true);

                        // Activate
                        pClonedActorScript->ActivateFrameUpdate(true);
                        //pClonedActorScript->ActivateTimeUpdate(0.0f);

                        pClonedActorScript->RegisterInstance();

                        // Give some weapons
                        if (pClonedActorInventory) {
                            Hitman::BloodMoney::CCheat::GiveItem(pClonedActorInventory, "SMG_MP7_01");
                            Hitman::BloodMoney::CCheat::GiveItem(pClonedActorInventory, "Gun_HKusp_01");
                            Hitman::BloodMoney::CCheat::GiveItem(pClonedActorInventory, "Ammo_SMG_01", 20);
                        }

                        // Register in some places
                        if (Hitman::BloodMoney::ZGuardQuarterController::g_pCurrentLevelGuardControl) {
                            Hitman::BloodMoney::ZGuardQuarterController::g_pCurrentLevelGuardControl->RegisterActor(clonedActor->GetRef());
                            spdlog::info("Cloned actor was register in guard quarter control");
                        }

                        // And try to register this actor in ZDllSound::ActorRegister
                        auto sysInterface = Glacier::getInterface<Glacier::ZSysInterfaceWintel>(Hitman::BloodMoney::Globals::kSysInterfaceAddr);
                        if (sysInterface && sysInterface->m_soundWintelDLL) {
                            // ZDllSound::ActorRegister
                            ((void(__thiscall*)(int, Glacier::ZGEOM*))0x004C60E0)(sysInterface->m_soundWintelDLL, reinterpret_cast<Glacier::ZGEOM*>(clonedActor));
                            spdlog::info("Cloned actor was register in sound subsystem as sound emitter");
                        }
                    }
                }
            }
        }
    }
}

namespace Hitman::BloodMoney
{
    static constexpr const char* kWindowTitle = "Actors List";

    void DrawErrorDialog(const char* errorMsg)
    {
        ImGui::Begin(kWindowTitle, &ActorsListWidget::g_bIsOpened);
        ImGui::TextColored(ImVec4 { 1.f, 1.f, 0.f, 1.f }, "%s", errorMsg);
        ImGui::End();
    }

    void DrawActorInfo(Hitman::BloodMoney::ZHM3Actor* actor, int actorIndex)
    {
        ImGui::Text("Actor #%.2d (at 0x%.8X)", actorIndex, reinterpret_cast<uint32_t>(actor));
        ImGui::Separator();

        if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
        {
            ImGui::BeginTabItem("General");
            ImGui::Inspector<Hitman::BloodMoney::ZHM3Actor>::Draw("???", actor);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    void DrawActorsListDialog(Glacier::ZEngineDataBase* engineDataBase, Hitman::BloodMoney::ZHM3GameData* gameData)
    {
        ImGui::SetNextWindowSize(ImVec2(650, 440), ImGuiCond_FirstUseEver);

        ImGui::Begin("Actors List", &ActorsListWidget::g_bIsOpened);
        {
            ImGui::Separator();

             //Left side
            static int selectedIndex = 0;

            // crash fix for level reload
            if (selectedIndex >= gameData->m_ActorsPool.m_iSize)
                selectedIndex = 0;

            ImGui::BeginChild("$leftside", ImVec2(300, 0), true);
            for (int actorIndex = 0; actorIndex < gameData->m_ActorsPool.m_iSize; actorIndex++)
            {
                auto entityName = fmt::format("#{:2d} {}",
                                              (actorIndex + 1), gameData->m_ActorsPool.m_Data[actorIndex]->m_baseGeom->ParentGroup()->m_baseGeom->m_Name);
                if (ImGui::Selectable(entityName.data(), selectedIndex == actorIndex))
                {
                    selectedIndex = actorIndex;
                }
            }
            ImGui::EndChild();
            ImGui::SameLine();

            //Right side
            Hitman::BloodMoney::ZHM3Actor* currentActor = gameData->m_ActorsPool.m_Data[selectedIndex];

            ImGui::BeginGroup();
            ImGui::BeginChild("$itemview", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

            DrawActorInfo(currentActor, selectedIndex);

            ImGui::EndChild();
            ImGui::EndGroup();
        }
        ImGui::End();
    }

    void ActorsListWidget::draw()
    {
        DebugWidget::draw();

        if (!g_bIsOpened) return;

        auto sysInterface = Glacier::getInterface<Glacier::ZSysInterfaceWintel>(Globals::kSysInterfaceAddr);
        if (!sysInterface) { return; }
        
        auto engineDb = sysInterface->m_engineDataBase;
        if (!engineDb) { return; }
        
        auto gameData = Glacier::getInterface<Hitman::BloodMoney::ZHM3GameData>(Globals::kGameDataAddr);

        if (!gameData)
        {
            DrawErrorDialog("NO GAME DATA");
            return;
        }

        if (!gameData->m_ActorsPool.m_iSize)
        {
            DrawErrorDialog("NO ACTORS ON THIS SCENE");
            return;
        }

        DrawActorsListDialog(engineDb, gameData);
    }

    bool ActorsListWidget::g_bIsOpened = false;
}