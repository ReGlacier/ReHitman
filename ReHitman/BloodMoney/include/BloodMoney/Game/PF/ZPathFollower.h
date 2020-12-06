#pragma once

#include <Glacier/REFTAB.h>
#include <Glacier/ZRTTI.h>
#include <Glacier/ZEventBase.h>

namespace Hitman::BloodMoney
{
    class ZEventBase;

    enum EScriptPathType : int {}; //TODO: Reverse all possible values of enum

    class ZPathFollower : public Glacier::ZEventBase
    {
    public:
        /// === consts ===
        static constexpr const char* Name = "PathFollower";

        /// === api ===
        int GetClosestWaypoint();
        int GetRndUsePoint();
        void SetExternalWaypointList(unsigned int REF);
        void SetWaypointIndex(int index);

        /// === members ===
        // --> ZPathFollower
        Glacier::REFTAB m_listsOfWaypoints; //0x30
        int m_currentWaypointList;
        int m_countOfWaypoints;
        int m_totalWaypoints;
        EScriptPathType m_pathType;
        bool m_useWalk;
        bool m_useClosestOnStart;
        bool m_playAnimsWhileWalking;
        bool m_field5F;
        int field_60;
        int m_waypointIndex;
        bool m_isPathFollowerValid;
        char field_69;
        char field_6A;
        char field_6B;
    };

    // Stack checks
    static_assert(offsetof(ZPathFollower, m_listsOfWaypoints) == 0x30, "ZPathFollower| Bad class entry point");
}