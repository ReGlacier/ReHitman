#pragma once


namespace Glacier::PF4
{
    enum EPathWayActions {
        PWA_INVALID = -1,
        PWA_DEFAULT = 0,
        PWA_CLIMB = 1,
        PWA_FALL = 2,
        PWA_JUMP = 3,
        PWA_WALK = 4,
        PWA_STOP = 5,
        PWA_RAPEL = 6,
        PWA_DOOR = 7,
        PWA_ENTERELEVATOR = 8,
        PWA_EXITELEVATOR = 9,
    };
}