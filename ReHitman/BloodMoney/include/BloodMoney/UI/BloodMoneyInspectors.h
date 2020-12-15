#pragma once

#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/ZSTL/ZLIST.h>

#include <BloodMoney/UI/ImGuiInspector.h>
#include <BloodMoney/Game/PF/ZPathFollower.h>

#include <imgui.h>

namespace ImGui
{
    //BloodMoneyInspectors
    template <> struct Inspector<Hitman::BloodMoney::ZPathFollower>
    {
        static void Draw(const char* /*id*/, Hitman::BloodMoney::ZPathFollower* pathFollower)
        {
            if (!pathFollower)
            {
                ImGui::TextColored(ImVec4 { 1.f, 1.f, 0.f, 1.f }, "NO ZPathFollower INSTANCE");
                return;
            }

            ImGui::Text("Path Follower:");
            ImGui::Separator();
            ImGui::Text("ZEventBase::EventName: %s", pathFollower->EventName());
            ImGui::Text("PTR: 0x%.8X", (int)pathFollower);

            const int totalWaypointsListsCount = pathFollower->m_listsOfWaypoints.Count();
            ImGui::Text("Count of Waypoint Lists: %d", totalWaypointsListsCount);

            for (int waypointListIndex = 0; waypointListIndex < totalWaypointsListsCount; ++waypointListIndex)
            {
                const int ZLISTEntityId = reinterpret_cast<int>(pathFollower->m_listsOfWaypoints[waypointListIndex]);
                auto pointsList = reinterpret_cast<Glacier::ZLIST*>(Glacier::ZGEOM::RefToPtr(ZLISTEntityId));

                if (!pointsList || !pointsList->m_entries)
                {
                    ImGui::TextColored(ImVec4 { 1.f, 1.f, 0.f, 1.f }, "Waypoint List #%d not found", (waypointListIndex + 1));
                    continue;
                }

//                for (int pointIndex = 0; pointIndex < pointsList->m_entries->Count(); ++pointIndex)
//                {
//                    auto entity = pointsList->At<Glacier::ZGEOM>(pointIndex);
//                    if (!entity)
//                    {
//                        ImGui::
//                    }
//                }

                ImGui::Text("Waypoints List #%d : %s", (waypointListIndex+1), pointsList->m_baseGeom->entityName);
                //const int pointsCount = pointsList->m_entries->Count();
                //TODO: Continue from https://github.com/DronCode/ReHitman/blob/22fb0d77f3bfb593072bdfb8a3c380e7e2dc7c37/HM3CoreKill/HM3CoreKill/ck/HM3InGameTools.cpp#L896
            }
        }
    };
}