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