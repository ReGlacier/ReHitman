#include <spdlog/spdlog.h>
#include <imgui.h>

#include <BloodMoney/UI/Widgets/SandboxWidget.h>
#include <BloodMoney/UI/GlacierInspectors.h>

#include <BloodMoney/Game/ZHM3GameData.h>

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

#include <HF/HackingFramework.hpp>


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

        auto geomBuffer = Glacier::getInterface<Glacier::ZGeomBuffer>(Globals::kGeomBufferAddr);
        if (!geomBuffer) { return; }

        ImGui::Begin("Sandbox");
        ImGui::Text("Mouse pos 3D: %f; %f; %f", sysInterface->m_renderer->m_mousePos.x, sysInterface->m_renderer->m_mousePos.y, sysInterface->m_renderer->m_mousePos.z);
        ImGui::End();
    }

    bool SandboxWidget::g_bIsVisible = false;
}