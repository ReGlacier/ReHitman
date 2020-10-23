#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZMath.h>

namespace Hitman::BloodMoney
{
    class ZPodiumController
    {
    public:
        char field_0;
        char field_1;
        char field_2;
        char field_3;
        char m_field4;
        char field_5;
        char field_6;
        char field_7;
        bool m_bMoving;
        bool m_bHitmanOnTable;
        bool m_fieldA;
        bool m_fieldB;
        int m_fMoveEndTime;
        char field_10;
        __declspec(align(4)) bool m_bTrapDoorClosed;
        Glacier::Vector3 m_vCameraTargetPos;
        Glacier::Vector3 m_vCameraFocusPos;
    };
}