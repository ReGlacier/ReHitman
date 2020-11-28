#include <spdlog/spdlog.h>
#include <imgui.h>

#include <BloodMoney/UI/Widgets/SandboxWidget.h>

#include <BloodMoney/Game/ZHM3GameData.h>

#include <BloodMoney/Game/UI/ZHM3MenuElements.h>
#include <BloodMoney/Game/UI/ZHM3MenuFactory.h>
#include <BloodMoney/Game/UI/ZXMLGUISystem.h>
#include <BloodMoney/Game/UI/ZWINDOWS.h>
#include <BloodMoney/Game/UI/ZGUIBase.h>

#include <BloodMoney/Game/Globals.h>

#include <Glacier/ZSysInterfaceWintel.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/ZEntityLocator.h>
#include <Glacier/ZGeomBuffer.h>
#include <Glacier/Glacier.h>
#include <Glacier/ZGROUP.h>
#include <Glacier/ZGEOM.h>

#include <BloodMoney/Game/ZHM3Actor.h>

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
        // Your code here
        if (auto player = gameData->m_Hitman3; player && gameData->m_ActorsInPoolCount && ImGui::Button("Spawn test actor"))
        {
            // Find our "actor", for example actor #4
            auto actorN4 = gameData->m_ActorsPool[4];
            // Cast to ZGEOM (ZHM3Actor, like most entities in Glacier, was derived from ZGEOM class, so we can simply cast it to ZGEOM)
            auto actorGeom = reinterpret_cast<Glacier::ZGEOM*>(actorN4);

            // Get group of base character
            using ZBaseGeom_ParentGroupFn = Glacier::ZGROUP*(__thiscall*)(Glacier::ZEntityLocator*);
            auto ZBaseGeom_ParentGroup = (ZBaseGeom_ParentGroupFn)0x00432640;

            // So, we can attach our ZGEOM to global root group (ZEngineDataBase::m_room as ZROOM (derived from ZGROUP))
            auto globalGroup = reinterpret_cast<Glacier::ZGROUP*>(engineDb->m_room);

            // Or take parent group from original actor
            auto group = ZBaseGeom_ParentGroup(actorGeom->m_baseGeom);

            spdlog::info("Base actor geom group is {:8X}", ((uint32_t)group));

            // If we have a group
            if (group)
            {
                // Allocate another one geom
                auto newGeom = group->CreateGeom("Hitman2", 0x2003E9, true);

                // Take primitive ID from our target actor
                const auto primId = actorGeom->m_baseGeom->m_primitive; //reinterpret_cast<Glacier::ZGEOM*>(player)->m_baseGeom->m_primitive;
                using ZBaseGeom_SetPrimFn = void(__thiscall*)(Glacier::ZEntityLocator*, int);
                auto ZBaseGeom_SetPrim = (ZBaseGeom_SetPrimFn)0x00431DB0;

                // Set primitive id and update view for our new geom
                ZBaseGeom_SetPrim(newGeom->m_baseGeom, primId);

                // Attach geom to our group
                int res = 0;
                using ZGROUP_AttachGeom2Fn = int (__thiscall*)(Glacier::ZGROUP*, Glacier::ZGEOM*, bool);
                auto ZGROUP_AttachGeom2 = (ZGROUP_AttachGeom2Fn)0x004E9FE0;

                res = ZGROUP_AttachGeom2(group, newGeom, true);

                // Add actor to the actors list and resize actors list
                gameData->m_ActorsPool[gameData->m_ActorsInPoolCount] = reinterpret_cast<Hitman::BloodMoney::ZHM3Actor*>(newGeom);
                gameData->m_ActorsPool[gameData->m_ActorsInPoolCount - 1]->m_actorRole = EActorMapRole::UnknownRole_FilledYellow; // actor role on map
                ++gameData->m_ActorsInPoolCount;

                // Setup map icon for our actor
                using ZHM3Actor_InitMapIconFn = void(__thiscall*)(Hitman::BloodMoney::ZHM3Actor*, bool);
                auto ZHM3Actor_InitMapIcon = (ZHM3Actor_InitMapIconFn)0x00637460;

                ZHM3Actor_InitMapIcon(gameData->m_ActorsPool[gameData->m_ActorsInPoolCount - 1], true);

                // Print debug info
                spdlog::info("Old geom at: {:8X}", ((uint32_t)actorN4));
                spdlog::info("New geom at: {:8X} attached with ret code {:8X}", ((uint32_t)newGeom), res);
            }
            else
            {
                spdlog::error("Actor #4 isn't  a member of any group!");
            }
        }

        ImGui::End();
    }

    bool SandboxWidget::g_bIsVisible = false;
}