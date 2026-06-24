#include <BloodMoney/UI/DebugTools.h>
#include <BloodMoney/Game/Globals.h>
#include <BloodMoney/Game/ZHM3GameData.h>
#include <BloodMoney/Game/CIngameMap.h>
#include <BloodMoney/Game/Items/ZHM3ItemTemplate.h>
#include <BloodMoney/Game/Items/ZHM3ItemAmmo.h>
#include <BloodMoney/Game/Items/ZHM3ItemBomb.h>
#include <BloodMoney/Game/Items/ZHM3ItemContainer.h>
#include <BloodMoney/Game/Items/ZHM3ItemTemplate.h>
#include <BloodMoney/Game/Items/ZHM3ItemTemplateBomb.h>
#include <BloodMoney/Game/Items/ZHM3ItemTemplateAmmoCustom.h>
#include <BloodMoney/Game/Items/ZHM3ItemTemplateContainer.h>
#include <BloodMoney/Game/Items/ZHM3ItemTemplateWeapon.h>
#include <BloodMoney/Game/Items/ZHM3ItemWeaponCustomTemplate.h>
#include <BloodMoney/Game/ZPodiumController.h>
#include <BloodMoney/Game/ZHitman3.h>
#include <BloodMoney/Game/LevelControls/ZHM3LevelControlM13.h>

#include <Glacier/ResourceCollection.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/ZSysInterfaceWintel.h>
#include <Glacier/ZActionManager.h>
#include <Glacier/GUI/ZWINGROUP.h>
#include <Glacier/GUI/ZCONTROL.h>
#include <Glacier/CInventory.h>
#include <Glacier/ZPlayer.h>
#include <Glacier/Items/ZItemState.h>
#include <Glacier/Items/ZItemTemplateContainer.h>
#include <Glacier/Items/ZItemTemplateWeapon.h>
#include <Glacier/Geom/ZEditorGroup.h>

#include <Glacier/Geom/ZGEOM.h>

#include <spdlog/spdlog.h>
#include <imgui.h>
#include <array>
#include <string>

// Widgets
#include <BloodMoney/UI/Widgets/ActorsListWidget.h>
#include <BloodMoney/UI/Widgets/CutSequencePlayerWidget.h>
#include <BloodMoney/UI/Widgets/SceneViewer.h>
#include <BloodMoney/UI/Widgets/PlayerTeleportWidget.h>

namespace Hitman::BloodMoney
{
    namespace Internals
    {
        static void QuitGame()
        {
            auto sysInterface = Glacier::getInterface<Glacier::ZSysInterfaceWintel>(Globals::kSysInterfaceAddr);
            if (sysInterface)
            {
                auto engineDB = sysInterface->m_engineDataBase;
                if (engineDB)
                {
                    engineDB->CloseDown();
                } else spdlog::error("QuitGame| No EngineDataBase instance");
            } else spdlog::error("QuitGame| No SystemInterface instance");
        }
    }

    void DebugTools::draw()
    {
        if (!m_bIsVisible)
            return;

        // TODO: On pre draw
        onPreDraw();
        DebugWidget::draw();
        onPostDraw();
    }

    void DebugTools::toggleVisibility()
    {
        m_bIsVisible = !m_bIsVisible;

        Glacier::ZActionManager* actionManager = Glacier::getInterface<Glacier::ZActionManager>(Hitman::BloodMoney::Globals::kActionManagerAddr);

        actionManager->m_isEnabled = !m_bIsVisible;
    }

    bool DebugTools::isVisible() const
    {
        return m_bIsVisible;
    }

    void DebugTools::onPreDraw()
    {
        drawTopMenu();
    }

    void DebugTools::onPostDraw()
    {}

    void DebugTools::drawLevelsTopMenu()
    {
        auto gameData = Glacier::getInterface<Hitman::BloodMoney::ZHM3GameData>(Globals::kGameDataAddr);
        if (!gameData) { return; }

        auto sysInterface = Glacier::getInterface<Glacier::ZSysInterfaceWintel>(Globals::kSysInterfaceAddr);
        if (!sysInterface) { return; }

        auto engineDb = sysInterface->m_engineDataBase;
        if (!engineDb) { return; }

        if (ImGui::BeginMenu("Levels"))
        {
            using LevelNameToLevelScene = std::pair<std::string_view, std::string_view>;

            std::array<LevelNameToLevelScene, 14> levels = {
                    LevelNameToLevelScene { "Hideout", "hideout/hideout_main.gms" },
                    LevelNameToLevelScene { "M00", "M00/M00_main.gms" },
                    LevelNameToLevelScene { "M01", "M01/M01_main.gms" },
                    LevelNameToLevelScene { "M02", "M02/M02_main.gms" },
                    LevelNameToLevelScene { "M03", "M03/M03_main.gms" },
                    LevelNameToLevelScene { "M04", "M04/M04_main.gms" },
                    LevelNameToLevelScene { "M05", "M05/M05_main.gms" },
                    LevelNameToLevelScene { "M06", "M06/M06_main.gms" },
                    LevelNameToLevelScene { "M08", "M08/M08_main.gms" },
                    LevelNameToLevelScene { "M09", "M09/M09_main.gms" },
                    LevelNameToLevelScene { "M10", "M10/M10_main.gms" },
                    LevelNameToLevelScene { "M11", "M11/M11_main.gms" },
                    LevelNameToLevelScene { "M12", "M12/M12_main.gms" },
                    LevelNameToLevelScene { "M13", "M13/M13_main.gms" }
            };

            for (const auto& level : levels)
            {
                const auto& [ name, scene ] = level;

                if (ImGui::MenuItem(name.data()))
                {
                	engineDb->UnloadScene();
                    engineDb->LoadScene(scene.data());
                }
            }

            // ------------
            ImGui::EndMenu();
        }
    }

    void DebugTools::drawTopMenu()
    {
        static bool showInventoryEditor = false;
        static bool showGeomViewer = false;

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Tools"))
            {
                ImGui::MenuItem("Actors viewer", nullptr, &ActorsListWidget::g_bIsOpened);
                ImGui::MenuItem("Cut Sequences player", nullptr, &CutSequencePlayerWidget::g_bIsShowed);
                ImGui::MenuItem("Scene viewer", nullptr, &SceneViewer::g_bIsVisible);
                ImGui::MenuItem("Teleport", nullptr, &PlayerTeleportWidget::g_bIsShowed);
                ImGui::MenuItem("Inventory editor", nullptr, &showInventoryEditor);
                ImGui::Separator();
                if (ImGui::MenuItem("Close game"))
                {
                    Internals::QuitGame();
                }
                ImGui::EndMenu();
            }

            drawLevelsTopMenu();

            ImGui::EndMainMenuBar();
        }

        showUpgradesEditor();
    }

    void DebugTools::showUpgradesEditor() {
        static const char* upgradeNames[] = {
            "UT_Dummy", "UT_AmmoACP", "UT_AmmoArmorPiercing", "UT_Ammo127mm", "UT_AmmoMagnum",
            "UT_AmmoFlechetteSlugs", "UT_AmmoGaugesSlugs", "UT_AmmoLowVelocity", "UT_Magazine",
            "UT_Silencer1", "UT_Silencer2", "UT_LaserSight", "UT_DualAction", "UT_DualActionAuto",
            "UT_DoubleCapMag", "UT_FullAuto", "UT_ReloadBoost", "UT_BeltFeeding", "UT_BiPod",
            "UT_ScopeType1", "UT_ScopeType2", "UT_ScopeType3", "UT_NightVision", "UT_Lightweight",
            "UT_DefaultAmmo", "UT_DefaultNoScope", "UT_DefaultBarrel", "UT_DefaultMagazine",
            "UT_RailMount", "UT_CarbonBarrel", "UT_Buttstock", "UT_Suitcase", "UT_DefaultButtStock",
            "UT_DefaultGrip", "UT_DefaultHandguard", "UT_DefaultHandle", "UT_DefaultSight",
            "UT_BoltAction", "UT_RedDotSight", "UT_ShortBarrel", "UT_PistolGrip", "UT_HandGuard",
            "UT_RapidFire", "UT_LongSlide", "UT_ClipX2", "UT_ClipX3", "UT_ClipX4"
        };

        using ZHM3ItemTool_GetHM3ItemTemplateWeapon_t = Hitman::BloodMoney::ZHM3ItemTemplate* (__stdcall*)(Glacier::ZItem*);
        using ZHM3ItemTool_IsCustomWeapon_t = bool(__cdecl*)(Hitman::BloodMoney::EHM3ItemType);
        using ZHM3ItemTool_GetHM3Type_t = Hitman::BloodMoney::EHM3ItemType(__cdecl*)(Glacier::ZItem*);
        using ZHM3ItemWeaponCustom_ClearUpgrades_t = int(__thiscall*)(void*);
        using ZHM3ItemWeaponCustom_AddUpgrade_t = int(__thiscall*)(void*, EUpgradeType);
        using ZHM3ItemWeaponCustom_ApplyUpgrades_t = void(__thiscall*)(void*, bool);
        using ZHM3ItemWeaponCustom_UpdateWeaponPartDrawStatus_t = void(__thiscall*)(void*);

        auto ZHM3ItemTool_GetHM3ItemTemplateWeapon = reinterpret_cast<ZHM3ItemTool_GetHM3ItemTemplateWeapon_t>(0x00518D00);
        auto ZHM3ItemTool_IsCustomWeapon = reinterpret_cast<ZHM3ItemTool_IsCustomWeapon_t>(0x00649610);
        auto ZHM3ItemTool_GetHM3Type = reinterpret_cast<ZHM3ItemTool_GetHM3Type_t>(0x0064D190);
        auto ZHM3ItemWeaponCustom_ClearUpgrades = reinterpret_cast<ZHM3ItemWeaponCustom_ClearUpgrades_t>(0x00650520);
        auto ZHM3ItemWeaponCustom_AddUpgrade = reinterpret_cast<ZHM3ItemWeaponCustom_AddUpgrade_t>(0x0064A7C0);
        auto ZHM3ItemWeaponCustom_ApplyUpgrades = reinterpret_cast<ZHM3ItemWeaponCustom_ApplyUpgrades_t>(0x006505E0);
        auto ZHM3ItemWeaponCustom_UpdateWeaponPartDrawStatus = reinterpret_cast<ZHM3ItemWeaponCustom_UpdateWeaponPartDrawStatus_t>(0x00650450);

        // Is game ready
        auto gameData = Glacier::getInterface<Hitman::BloodMoney::ZHM3GameData>(Globals::kGameDataAddr);
        if (!gameData || !gameData->m_Hitman3) { return; }

        auto inventory = reinterpret_cast<Glacier::CInventory*>(reinterpret_cast<Glacier::ZGEOM*>(gameData->m_Hitman3)->FindEvent(Glacier::CInventory::Name));
        if (!inventory) { return; }

        // State
        static bool g_bShowInventoryMods = false;
        static int selectedWeaponIdx = -1;
        static bool selectedUpgrades[UT_NumUpgradeTypes] = { false };

        auto* reftab = inventory->GetInventoryList();

        if (reftab && ImGui::Begin("Upgrade editor", &g_bShowInventoryMods))
        {
            ImGui::Columns(2, "WeaponEditorColumns", true);

            // Left side
            ImGui::Text("Custom Weapons:");
            ImGui::Separator();

            Glacier::ZGEOM* selectedGeom = nullptr;

            for (size_t i = 0; i < reftab->Count(); ++i)
            {
                auto* geom = Glacier::ZGEOM::RefToPtr(reftab->GetRefNr(i));
                if (!geom) continue;

                auto type = ZHM3ItemTool_GetHM3Type(reinterpret_cast<Glacier::ZItem*>(geom));
                if (!ZHM3ItemTool_IsCustomWeapon(type)) { continue; }

                char label[128]{ '\x00' };
                snprintf(label, sizeof(label), "%s##%d", geom->m_baseGeom->m_Name, static_cast<int>(i));

                if (ImGui::Selectable(label, selectedWeaponIdx == static_cast<int>(i))) {
                    selectedWeaponIdx = static_cast<int>(i);
                }

                if (selectedWeaponIdx == static_cast<int>(i)) {
                    selectedGeom = geom;
                }
            }

            ImGui::NextColumn();

            // Right side
            ImGui::Text("Upgrade Setup:");
            ImGui::Separator();

            if (selectedGeom != nullptr)
            {
                static int lastSelectedWeaponIdx = -1;

                if (selectedWeaponIdx != lastSelectedWeaponIdx)
                {
                    memset(selectedUpgrades, 0, sizeof(selectedUpgrades));
                    lastSelectedWeaponIdx = selectedWeaponIdx;
                }

                ImGui::Text("Weapon: %s", selectedGeom->m_baseGeom->m_Name);
                ImGui::Spacing();

                ImGui::Text("Select upgrades to apply:");
                ImGui::BeginChild("UpgradesListList", ImVec2(0, -40), true);
                {
                    for (int i = 1; i < UT_NumUpgradeTypes; ++i)
                    {
                        ImGui::Checkbox(upgradeNames[i], &selectedUpgrades[i]);
                    }
                }
                ImGui::EndChild();

                // Apply changes
                if (ImGui::Button("Apply Selected Upgrades", ImVec2(-1, 30)))
                {
                    void* customWeaponInstance = reinterpret_cast<void*>(selectedGeom);

                    // Clear upgrades
                    ZHM3ItemWeaponCustom_ClearUpgrades(customWeaponInstance);

                    // Apply new upgrades
                    for (int i = 1; i < UT_NumUpgradeTypes; ++i)
                    {
                        if (selectedUpgrades[i])
                        {
                            ZHM3ItemWeaponCustom_AddUpgrade(customWeaponInstance, static_cast<EUpgradeType>(i));
                        }
                    }

                    // Update upgrades & view
                    ZHM3ItemWeaponCustom_ApplyUpgrades(customWeaponInstance, true);
                    ZHM3ItemWeaponCustom_UpdateWeaponPartDrawStatus(customWeaponInstance);
                }
            }
            else
            {
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Select a weapon from the left list.");
            }

            ImGui::Columns(1);
            ImGui::End();
        }
    }
}