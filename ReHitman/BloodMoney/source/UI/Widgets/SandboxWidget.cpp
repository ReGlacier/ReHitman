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

#include <Glacier/System/ZSysInterfaceWintel.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/Geom/ZGeomBuffer.h>
#include <Glacier/Glacier.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/GameBase/CInventory.h>
#include <Glacier/System/CConfiguration.h>
#include <Glacier/ZSTL/REFTAB32.h>
#include <Glacier/Items/ZItem.h>
#include <Glacier/Items/ZItemTemplate.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/Geom/ZTreeGroup.h>
#include <Glacier/ZTypeTraits.h>
#include <Glacier/Geom/ZLIST.h>
#include <Glacier/Items/ZItemTemplateAmmo.h>
#include <Glacier/Items/ZItemTemplateWeapon.h>
#include <Glacier/Items/ZItemWeapon.h>

#include <BloodMoney/Game/ZHM3Actor.h>

#include <BloodMoney/Game/CIngameMap.h>
#include <BloodMoney/Game/ZHM3HmAs.h>
#include <BloodMoney/Game/ZHM3ClothBundle.h>
#include <BloodMoney/Game/ZHM3BriefingControl.h>
#include <BloodMoney/Game/UI/ZCHAROBJ.h>
#include <BloodMoney/Game/UI/ZWINOBJ.h>
#include <BloodMoney/Game/UI/ZXMLGUISystem.h>

#include <BloodMoney/Game/ZTie.h>
#include <BloodMoney/Game/UI/ZLINEOBJ.h>

#include <HF/HackingFramework.hpp>
#include <Glacier/Geom/ZEngineGeomControl.h>
#include <Glacier/Com/CCom.h>
#include <Glacier/EventBase/ZEventBuffer.h>

#include <BloodMoney/Game/OnLevel/ZVCR.h>
#include <BloodMoney/Game/Items/ZHM3ItemWeaponCustom.h>
#include <BloodMoney/Game/ZCheatMenu.h>

#include <BloodMoney/Engine/ZHM3Camera.h>
#include <BloodMoney/Engine/ZHM3CameraConsole.h>


namespace Hitman::BloodMoney
{

    void SandboxWidget::draw()
    {
        DebugWidget::draw();

        auto gameData = Glacier::getInterface<Hitman::BloodMoney::ZHM3GameData>(Globals::kGameDataAddr);
        if (!gameData) { return; }

        auto sysInterface = Glacier::getInterface<Glacier::ZSysInterfaceWintel>(Globals::kSysInterfaceAddr);
        if (!sysInterface) { return; }

        auto engineDb = sysInterface->m_pEngineData;
        if (!engineDb) { return; }

        auto sceneCom = engineDb->GetSceneCom();
        if (!sceneCom) { return; }

        auto geomBuffer = Glacier::getInterface<Glacier::ZGeomBuffer>(Globals::kGeomBufferAddr);
        if (!geomBuffer) { return; }

        auto hitman = reinterpret_cast<Glacier::ZLNKWHANDS*>(gameData->m_Hitman3);
        if (!hitman) return;

        ImGui::Begin("Test script");

		{
			static int g_typeId { 0 };

			ImGui::InputInt("Geom type id: ", &g_typeId); ImGui::SameLine(0.f, .5f);
			if (ImGui::Button("FIND"))
			{
				auto tp = engineDb->GetGeomClassInfo(g_typeId);

				if (!tp)
				{
					spdlog::error("No type found of id {}", g_typeId);
				}
				else
				{
					spdlog::info("Found type '{}' (of id {})", tp->ClassInfoName(), g_typeId);

					spdlog::info("TypeID : {}", tp->Type());
					spdlog::info("Mask   : {}", tp->GetMaskId());
					spdlog::info("ID     : {}", tp->GetClassId());
				}

				g_typeId = 0;
			}
		}

        ImGui::End();

        //-----------------------------------------------------------------------------------------------
        ImGui::Begin("TEST");

        if (ImGui::Button("Give MP7 with tranquilizer ammo")) {
            auto engineDB = sysInterface->m_pEngineData;
            if (!engineDB) {
                return;
            }

            std::vector<Glacier::ZGEOM*> ammoEntries;
            std::vector<Glacier::ZGEOM*> gunWeaponTemplates;

            auto findEntriesExactOfType = [engineDb](const char* psGroupName, std::intptr_t pTypeMask, std::intptr_t pTypeId, std::string_view sEntityName, std::vector<Glacier::ZGEOM*>& result) {
                auto pId = reinterpret_cast<std::intptr_t*>(pTypeId);
                auto pMask = reinterpret_cast<std::intptr_t*>(pTypeMask);

                if (!pId || !pMask) {
                    spdlog::error("Bad type id ({:08X}) or mask ({:08X}) pointer(s)", pTypeId, pTypeMask);
                    return;
                }

                std::intptr_t rpGroup = engineDb->GetSceneVar(psGroupName);
                if (!rpGroup) {
                    spdlog::warn("No '{}' group ptr presented", psGroupName);
                    return;
                }

                Glacier::ZREF rGroup = *reinterpret_cast<Glacier::ZREF*>(rpGroup);
                if (!rGroup) {
                    spdlog::warn("No '{}' group valid ptr presented", psGroupName);
                    return;
                }

                auto pGroup = reinterpret_cast<Glacier::ZGROUP*>(Glacier::ZGEOM::RefToPtr(rGroup));
                if (!pGroup) {
                    spdlog::warn("No {} instance presented", psGroupName);
                    return;
                }

                Glacier::ZBaseGeom* pCurrentEnt = pGroup->m_baseGeom;
                Glacier::ZGEOM* pCurrentGeom = nullptr;

                do {
                    pCurrentGeom = pCurrentEnt->m_pExtraGeom;

                    const bool isAcceptable = (sEntityName.empty() ? true : (std::string_view { pCurrentEnt->m_Name } == sEntityName)) && (((*pMask) & pCurrentGeom->GetObjectId()) == *(pId));
                    if (isAcceptable) {
                        result.emplace_back(pCurrentGeom);
                    }

                    pGroup->RecurGetNext(&pCurrentEnt);
                }
                while (pCurrentEnt != nullptr);
            };

            findEntriesExactOfType("WeaponsGroup", 0x0099BF34, 0x0099BF30, "Rifle_Airrifle_Tranquilizer_01", gunWeaponTemplates); //4
            //findEntriesExactOfType("WeaponsGroup", 0x0099BF34, 0x0099BF30, "Custom_ShotGun", gunWeaponTemplates);
            //findEntriesExactOfType("WeaponsGroup", 0x0099BF34, 0x0099BF30, "SMG_MP7_01", gunWeaponTemplates);

            if (gunWeaponTemplates.empty()) {
                spdlog::warn("No required gun template on level!");
            } else {
                auto pGameData = Glacier::getInterface<Hitman::BloodMoney::ZHM3GameData>(Globals::kGameDataAddr);
                if (!pGameData) {
                    spdlog::error("No game data? U srsly??");
                    return;
                }

                auto pInventory = reinterpret_cast<Glacier::CInventory*>(reinterpret_cast<Glacier::ZGEOM*>(pGameData->m_Hitman3)->FindEvent(Glacier::CInventory::Name));
                if (!pInventory) {
                    spdlog::error("Failed to locate Inventory ZEventBase instance in player (something goes wrong?), player ptr: {:08X}", reinterpret_cast<std::intptr_t>(pGameData->m_Hitman3));
                } else {
                    auto pGunItem = pInventory->AddItem(gunWeaponTemplates[0]->GetRef());
                    if (!pGunItem) {
                        spdlog::error("Failed to give a gun to player :(");
                    }

                    auto pGun = reinterpret_cast<Glacier::ZItemWeapon*>(pGunItem);

                    reinterpret_cast<Glacier::ZItemTemplateWeapon*>(pGun->GetItemTemplate())->m_WeaponOperations = Glacier::EWeaponOperation::WO_FULLAUTO;
                    //spdlog::info("GT: {:08X}", reinterpret_cast<Glacier::ZItemTemplateWeapon*>(pGun->GetItemTemplate())->m_weaponType);
                    reinterpret_cast<Glacier::ZItemTemplateWeapon*>(pGun->GetItemTemplate())->m_eWeaponType = Glacier::WEAPONTYPE::WT_PISTOL;

                    pGun->m_eWeaponOperation = Glacier::EWeaponOperation::WO_FULLAUTO;

                    pGun->SetProjectilesInMagazine(999);
                }
            }
        }

        if (ImGui::Button("ZOMBIE MODE: ON!!!")) {
        	gameData->m_LevelControl->Command(0x952, nullptr);
//	        reinterpret_cast<Glacier::ZEventBase*>(gameData->m_LevelControl)->Command(0x918, nullptr);
//	        reinterpret_cast<Glacier::ZEventBase*>(gameData->m_LevelControl)->Command(0x917, nullptr);
        }

		ImGui::Text("Tutorial highlighting:");
		if (ImGui::Button("T0")) { gameData->m_OSD->m_field8F0 = 0; } ImGui::SameLine(0.f, 0.5f);
		if (ImGui::Button("T1")) { gameData->m_OSD->m_field8F0 = 1; } ImGui::SameLine(0.f, 0.5f);
		if (ImGui::Button("T2")) { gameData->m_OSD->m_field8F0 = 2; } ImGui::SameLine(0.f, 0.5f);
		if (ImGui::Button("T3")) { gameData->m_OSD->m_field8F0 = 3; } ImGui::SameLine(0.f, 0.5f);
		if (ImGui::Button("T4")) { gameData->m_OSD->m_field8F0 = 4; } ImGui::SameLine(0.f, 0.5f);
		if (ImGui::Button("T5")) { gameData->m_OSD->m_field8F0 = 5; } ImGui::SameLine(0.f, 0.5f);
		if (ImGui::Button("T6")) { gameData->m_OSD->m_field8F0 = 6; }

		if (ImGui::Button("DEBUG")) {
			spdlog::info("AUX: {:08X}", reinterpret_cast<std::intptr_t>(gameData->m_OSD->m_pHealthFrame));
		}

        if (ImGui::Button("Toggle AIM")) {
            const bool v = *reinterpret_cast<bool*>(((std::intptr_t)gameData->m_Hitman3) + 0xB58);
            *reinterpret_cast<bool*>(((std::intptr_t)gameData->m_Hitman3) + 0xB58) = !v;
        }

        if (ImGui::Button("Dump actor #0 matpos")) {
            Glacier::ZMat3x3 mat;
            Glacier::ZVector3 pos;

            reinterpret_cast<Glacier::ZGEOM*>(gameData->m_ActorsPool[0])->GetRootMatPos(mat, pos);

            spdlog::info("Actor #0: ");
            spdlog::info("Pos     : {};{};{}", pos.x, pos.y, pos.z);
            spdlog::info("Mat     : {};{};{}", mat.data[0], mat.data[1], mat.data[2]);
            spdlog::info("        : {};{};{}", mat.data[3], mat.data[4], mat.data[5]);
            spdlog::info("        : {};{};{}", mat.data[6], mat.data[7], mat.data[8]);
        }

//        if (gameData->m_LevelControl && ImGui::Button("Arrive enemies")) {
//            // ZHM3LevelControlM05::GetMoreGuards()
//            ((void(__thiscall*)(Hitman::BloodMoney::ZHM3LevelControl*))0x006BF540)(gameData->m_LevelControl);
//        }

		// if (gameData->m_OSD && gameData->m_OSD->m_pCheatsMenu && ImGui::Begin("Cheat Menu Editor Demo")) {
		// 	if (ImGui::Button("Add menu entry")) {
		// 		Glacier::REFTAB* pEntries = &gameData->m_OSD->m_pCheatsMenu->m_rtCommands;

		// 		auto onZombieModActivation = []() {
		// 			auto gameData = Glacier::getInterface<Hitman::BloodMoney::ZHM3GameData>(Globals::kGameDataAddr);
		// 			if (gameData && gameData->m_LevelControl)
		// 			{
		// 				gameData->m_LevelControl->Command(0x952, nullptr);
		// 			}
		// 		};

		// 		auto ent = reinterpret_cast<ZCheatMenu::SEntry*>(pEntries->Add(0));
		// 		ent->pName = "Run Zombie Mode";
		// 		ent->eType = ZCheatMenu::ECheatMenuEntryType::MENU_TOGGLE_CALLABLE_OPTION;
		// 		ent->uOption.pFunction = onZombieModActivation;
		// 	}

		// 	ImGui::End();
		// }

        ImGui::End();
    }

    bool SandboxWidget::g_bIsVisible = false;
}
