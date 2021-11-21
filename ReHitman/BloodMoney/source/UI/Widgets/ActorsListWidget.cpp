#include <BloodMoney/UI/Widgets/ActorsListWidget.h>

#include <BloodMoney/Game/PF/ZPathFollower.h>
#include <BloodMoney/Game/ZHM3GameData.h>
#include <BloodMoney/Game/ZHM3Actor.h>
#include <BloodMoney/Game/Globals.h>
#include <BloodMoney/Game/ZGuardQuarterController.h>
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
#include <Glacier/ZScriptC.h>
#include <Glacier/CInventory.h>

#include <Glacier/Fysix/CRigidBody.h>
#include <Glacier/Geom/ZROOM.h>
#include <HF/HackingFramework.hpp>
#include <imgui.h>

namespace ImGui
{
    void Inspector<Glacier::ESuitMask>::Draw(const char* id, Glacier::ESuitMask* mask) {
        switch (*mask) {
            case Glacier::ESuitMask::NoActor:
                ImGui::Text("ESuitMask::NoActor");
                break;
            case Glacier::ESuitMask::SkinChangerNotSupported:
                ImGui::Text("ESuitMask::SkinChangerNotSupported");
                break;
            case Glacier::ESuitMask::Nude:
                ImGui::Text("ESuitMask::Nude");
                break;
            case Glacier::ESuitMask::Invisible:
                ImGui::Text("ESuitMask::Invisible");
                break;
            case Glacier::ESuitMask::OriginalView:
                ImGui::Text("ESuitMask::OriginalView");
                break;
            case Glacier::ESuitMask::Agent47_WithoutHeaddress:
                ImGui::Text("ESuitMask::Agent47_WithoutHeaddress");
                break;
            case Glacier::ESuitMask::Agent47_WithHeaddress:
                ImGui::Text("ESuitMask::Agent47_WithHeaddress");
                break;
            default:
                ImGui::TextColored(ImVec4 { 1.f, 1.f, 0.f, 1.f }, "Unknown value: %d", *mask);
                break;
        }
    }

    void Inspector<Hitman::BloodMoney::ZHM3Actor>::Draw(const char* id, Hitman::BloodMoney::ZHM3Actor* actor) {
        //ImGui::Inspector<Glacier::ZEntityLocator>::Draw("actor.entity", actor->ActorInformation->location);
        ImGui::Inspector<Glacier::ZEntityLocator>::Draw("actor.entity", actor->m_baseGeom);
        ImGui::Separator();
        ImGui::Inspector<Glacier::ZEntityLocator>::Draw("actor.group", actor->m_baseGeom->ParentGroup()->m_baseGeom);
        ImGui::Inspector<Glacier::ZHumanBoid>::Draw("Actor boid", actor->m_boid);
        ImGui::Text("Group Info: ");
        ImGui::Inspector<Glacier::ZGROUP>::Draw("ActorGroup", actor->m_baseGeom->ParentGroup());
        ImGui::Inspector<Hitman::BloodMoney::ZPathFollower>::Draw("Actor.PathFollower", reinterpret_cast<Hitman::BloodMoney::ZPathFollower*>(actor->FindEvent(Hitman::BloodMoney::ZPathFollower::Name)));
        ImGui::Inspector<Glacier::CInventory>::Draw("Actor.Inventory", reinterpret_cast<Glacier::CInventory*>(actor->FindEvent(Glacier::CInventory::Name)));
        ImGui::Inspector<Glacier::ESuitMask>::Draw("Actor.SuitMask", &actor->m_suitMask);

        /// ==================
        {
            auto gameData = Glacier::getInterface<Hitman::BloodMoney::ZHM3GameData>(Hitman::BloodMoney::Globals::kGameDataAddr);
            if (gameData && actor->m_suitMask != Glacier::ESuitMask::NoActor && ImGui::Button("Swap bodies"))
            {
                auto actorGeom = reinterpret_cast<Glacier::ZGEOM*>(actor);
                auto playerGeom = reinterpret_cast<Glacier::ZGEOM*>(gameData->m_Hitman3);

                const int actorPrimId  = actorGeom->m_baseGeom->m_primitive;
                const int playerPrimId = playerGeom->m_baseGeom->m_primitive;

                reinterpret_cast<Glacier::ZLNKOBJ*>(actor)->CopyGeometryFrom(playerPrimId);
                reinterpret_cast<Glacier::ZLNKOBJ*>(actor)->UpdateGeometry(true);

                reinterpret_cast<Glacier::ZLNKOBJ*>(playerGeom)->CopyGeometryFrom(actorPrimId);
                reinterpret_cast<Glacier::ZLNKOBJ*>(playerGeom)->UpdateGeometry(true);
            }
        }

        // ==================
        {
            if (ImGui::Button("Clear PF4 path"))
            {
                using PF4_Path_Clear_t = void(__thiscall*)(std::intptr_t*);
                auto PF4_Path_Clear = (PF4_Path_Clear_t)0x004D8D00;

                PF4_Path_Clear(&actor->m_field534);

                actor->PreparePath();
            }
        }

        // ===================
        {
            auto gameData = Glacier::getInterface<Hitman::BloodMoney::ZHM3GameData>(Hitman::BloodMoney::Globals::kGameDataAddr);
            if (gameData && actor->m_suitMask != Glacier::ESuitMask::NoActor && ImGui::Button("Start human shield with"))
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

                spdlog::info("Request to actor '{}' run to {};{};{}", actor->m_baseGeom->entityName, pos.x, pos.y, pos.z);
            }
        }

        // ===================
        {
            auto gameData = Glacier::getInterface<Hitman::BloodMoney::ZHM3GameData>(Hitman::BloodMoney::Globals::kGameDataAddr);
            if (gameData && actor->m_suitMask != Glacier::ESuitMask::NoActor && ImGui::Button("Make clone"))
            {
                auto actorOwnerGroup = reinterpret_cast<Glacier::ZGEOM*>(actor)->m_baseGeom->ParentGroup();
                auto actorRootGroup = reinterpret_cast<Glacier::ZGEOM*>(actorOwnerGroup)->m_baseGeom->ParentGroup();

                spdlog::info("Owner: {} / Root: {}",
                             reinterpret_cast<Glacier::ZGEOM*>(actorOwnerGroup)->m_baseGeom->entityName,
                             reinterpret_cast<Glacier::ZGEOM*>(actorRootGroup)->m_baseGeom->entityName);

                Glacier::ZMat3x3 ownerMat;
                Glacier::ZVector3 ownerPos;

                actorOwnerGroup->GetMatPos(&ownerMat, &ownerPos);
                auto duplicateGroup = actorOwnerGroup->DuplicateInit(
                        actorRootGroup,
                        &ownerMat,
                        &ownerPos,
                        fmt::format("Actor!CloneOf!{}", reinterpret_cast<Glacier::ZGEOM*>(actorOwnerGroup)->m_baseGeom->entityName).c_str(),
                        true);
                auto clonedActor = reinterpret_cast<Hitman::BloodMoney::ZHM3Actor*>(reinterpret_cast<Glacier::ZGROUP*>(duplicateGroup)->FindGeom("Ground", nullptr));

                clonedActor->m_mapIconType = Hitman::BloodMoney::EActorMapRole::VIP;
                using ZHM3Actor_InitMapIconFn = void(__thiscall*)(Hitman::BloodMoney::ZHM3Actor*, bool);
                auto ZHM3Actor_InitMapIcon = (ZHM3Actor_InitMapIconFn)0x00637460;

                ZHM3Actor_InitMapIcon(clonedActor, true);

                reinterpret_cast<Glacier::ZLNKOBJ*>(clonedActor)->CopyPoseFrom(reinterpret_cast<Glacier::ZLNKOBJ*>(actor));

                // Add actor to tracking list
                auto pTrackLinkObjects = *(Glacier::ZLIST**)0x00972DA8;
                pTrackLinkObjects->AddGeom(reinterpret_cast<Glacier::ZGEOM*>(clonedActor));

                reinterpret_cast<Glacier::ZIKLNKOBJ*>(clonedActor)->EnableIK();
                reinterpret_cast<Glacier::ZIKLNKOBJ*>(clonedActor)->EnableControls();

                ((void(__cdecl*)(Glacier::ZREF, bool))0x006AA2B0)(reinterpret_cast<Glacier::ZGEOM*>(clonedActor)->GetRef(), true);

                //clonedActor->SetActorState(Hitman::BloodMoney::ZActor::ACTORSTATE::STATE_0);
                clonedActor->SetActorState(Hitman::BloodMoney::ZActor::ACTORSTATE::STATE_1);
                //clonedActor->SetActorState(Hitman::BloodMoney::ZActor::ACTORSTATE::STATE_2);
                //clonedActor->SetActorState(Hitman::BloodMoney::ZActor::ACTORSTATE::STATE_3);

                // ----------- PRETTY PRINT SOME INFOS -------------
                spdlog::info("TRK: {:08X}", (int)pTrackLinkObjects);
                spdlog::info("OACT: {:08X}", (int)actor);
                spdlog::info("Dup: {:08X} / ADup: {:08X}", (int)duplicateGroup, (int)clonedActor);

                // ----------- REGISTER ACTOR SOMEWHERE ------------
                clonedActor->SetActorState(((Hitman::BloodMoney::ZActor::ACTORSTATE(__thiscall*)(Hitman::BloodMoney::ZHM3Actor*))0x005029A0)(actor));

                clonedActor->m_Mask1 = actor->m_Mask1;
                clonedActor->m_field91C = actor->m_field91C;
                clonedActor->m_fieldA3C = actor->m_fieldA3C;

                spdlog::info("Cloned actor ptr is {:08X}", reinterpret_cast<std::intptr_t>(clonedActor));

                // ------------ ENABLE AI SCRIPTS ------------
                spdlog::info("TRK: {:08X}", (int)pTrackLinkObjects);
                spdlog::info("OACT: {:08X}", (int)actor);
                spdlog::info("Dup: {:08X} / ADup: {:08X}", (int)duplicateGroup, (int)clonedActor);

                Glacier::ZScriptC* pClonedActorScript = nullptr;
                Glacier::CInventory* pClonedActorInventory = nullptr;


                {
                    int* pDefaultStatus = Glacier::ZEventBase::GetDefaultStatus();
                    const int oldDefaultStatus = *pDefaultStatus;

                    // Here we need to change default status to fix ZScriptC event creation
                    *pDefaultStatus = 1;

                    //TODO: Here we need to fix ZGEOM vftable. One method is lost between FindEvent and AddEvent
                    pClonedActorInventory = HF::Hook::VFHook<Hitman::BloodMoney::ZHM3Actor>::invoke<Glacier::CInventory*, const char*>(clonedActor, 66, "ZGEOM_Inventory"); // Add inventory
                    pClonedActorScript = HF::Hook::VFHook<Hitman::BloodMoney::ZHM3Actor>::invoke<Glacier::ZScriptC*, const char*>(clonedActor, 66, "ZGEOM_ScriptC"); // AddEvent

                    // And don't forget to restore it back to avoid other issues
                    *pDefaultStatus = oldDefaultStatus;
                }

                if (!pClonedActorScript) {
                    spdlog::error("Failed to add ZScriptC to cloned actor");
                } else {
                    spdlog::info("Created & registered ZScriptC: {:08X}", (int)pClonedActorScript);

                    constexpr const char* psRequiredScriptName = "Alllevels_Armed";
                    int foundScript = pClonedActorScript->FindScript(psRequiredScriptName);
                    if (!foundScript) {
                        spdlog::error("Failed to find '{}' script!", psRequiredScriptName);
                    } else {
                        // And then call 'create script'
                        pClonedActorScript->m_pScriptsTable = pClonedActorScript->CreateScript(foundScript);
                        spdlog::info("AI script attached ({:08X})", pClonedActorScript->m_pScriptsTable);

                        // Here we need to call internal methods
                        clonedActor->Activate(true);

                        // Activate
                        pClonedActorScript->ActivateFrameUpdate(false);
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
                            ((void(__thiscall*)(int, Glacier::ZGEOM*))0x004C60E0)(sysInterface->m_soundWintelDLL, reinterpret_cast<Glacier::ZGEOM*>(clonedActor));
                            spdlog::info("Cloned actor was register in sound subsystem as sound emitter");
                        }

                        // Try to register actor in actor communication network
                        constexpr auto kRadioChannel = 3u;
                        //TODO: Wrap this call to ZActorCommunication class when it will be reversed
                        // Now we calling that function (ZActorCommunication::RegisterRadioUser) through script interface (SI)
                        ((void(__cdecl*)(Glacier::ZREF, unsigned int))0x006AA2B0)(clonedActor->GetRef(), kRadioChannel);
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
            ImGui::BeginChild("$leftside", ImVec2(300, 0), true);
            for (int actorIndex = 0; actorIndex < gameData->m_ActorsInPoolCount; actorIndex++)
            {
                auto entityName = fmt::format("#{:2d} {}",
                                              (actorIndex + 1),
                                              gameData->m_ActorsPool[actorIndex]->m_baseGeom->ParentGroup()->m_baseGeom->entityName);
                if (ImGui::Selectable(entityName.data(), selectedIndex == actorIndex))
                {
                    selectedIndex = actorIndex;
                }
            }
            ImGui::EndChild();
            ImGui::SameLine();

            //Right side
            Hitman::BloodMoney::ZHM3Actor* currentActor = gameData->m_ActorsPool[selectedIndex];

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

        if (!gameData->m_ActorsInPoolCount)
        {
            DrawErrorDialog("NO ACTORS ON THIS SCENE");
            return;
        }

        DrawActorsListDialog(engineDb, gameData);
    }

    bool ActorsListWidget::g_bIsOpened = false;
}