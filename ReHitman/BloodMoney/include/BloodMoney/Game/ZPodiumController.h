#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZMath.h>

namespace Hitman::BloodMoney
{
    class ZHM3LevelControlM13;

    class ZPodiumController
    {
    public:
        Glacier::ZGEOM* m_pPodium;
        ZHM3LevelControlM13* m_pLevelControl;
        bool m_bMoving;
        bool m_bHitmanOnTable;
        RE_ADD_PADDING(2);
        float m_fMoveEndTime;
        Glacier::ZGROUP* m_pTrapDoor;
        bool m_bTrapDoorClosed;
        RE_ADD_PADDING(3);
        Glacier::ZVector3 m_vCameraTargetPos;
        Glacier::ZVector3 m_vCameraFocusPos;
    };
    RE_VERIFY_SIZE(ZPodiumController, 0x30);
    RE_VERIFY_OFFSET(ZPodiumController, m_vCameraTargetPos, 0x18);
    RE_VERIFY_OFFSET(ZPodiumController, m_vCameraFocusPos, 0x24);
}