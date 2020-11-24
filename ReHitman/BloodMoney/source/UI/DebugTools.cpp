#include <BloodMoney/UI/DebugTools.h>
#include <BloodMoney/Game/Globals.h>
#include <BloodMoney/Game/ZHM3GameData.h>
#include <BloodMoney/Game/CIngameMap.h>

#include <Glacier/ResourceCollection.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/ZSysInterfaceWintel.h>

#include <Glacier/ZGEOM.h>

#include <spdlog/spdlog.h>
#include <imgui.h>
#include <array>
#include <string>

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

            std::array<LevelNameToLevelScene, 13> levels = {
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
                    engineDb->LoadScene(scene.data());
                }
            }

            // ------------
            ImGui::EndMenu();
        }
    }

    void DebugTools::drawTopMenu()
    {
        static bool showActorsViewer = false;
        static bool showInventoryEditor = false;
        static bool showGeomViewer = false;

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Tools"))
            {
                ImGui::MenuItem("Actors viewer", nullptr, &showActorsViewer);
                ImGui::MenuItem("Inventory editor", nullptr, &showInventoryEditor);
                ImGui::MenuItem("GEOM viewer", nullptr, &showGeomViewer);
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


        if (showActorsViewer)
        {
//            showDebugActorsWindow(&showActorsViewer);
        }

        if (showInventoryEditor)
        {
//            showInventoryEditorWindow(&showInventoryEditor);
        }

        if (showGeomViewer)
        {
//            showGeomViewerWindow(&showGeomViewer);
        }
    }
}