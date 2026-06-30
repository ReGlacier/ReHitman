#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <BloodMoney/Game/LevelControls/ZHM3LevelControl.h>


namespace Hitman::BloodMoney
{
    class ZHM3LevelControlM01 : public ZHM3LevelControl
    {
    public:
        // vtbl
        virtual void CompleteObjective(int iObjectiveIdx);

        // data
        bool m_bDonKilled;
        bool m_bAllTargetsDead;
        Glacier::ZREF m_rGuardHouseGuard;
        Glacier::ZMSGID m_M01_MissionComplete;
        Glacier::ZGEOM* m_pCamera;
        Glacier::ZGEOM* m_pCameraFlash;
    };
    RE_VERIFY_SIZE(ZHM3LevelControlM01, 0x5EC); // Verified
}