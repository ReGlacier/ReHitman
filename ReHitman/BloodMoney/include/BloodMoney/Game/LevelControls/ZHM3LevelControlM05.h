#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/Animation/ZRunMatPosAnim.h>
#include <Glacier/PF4/Fwd.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <BloodMoney/Game/LevelControls/ZHM3LevelControl.h>


namespace Hitman::BloodMoney
{
    enum eGarageState
    {
        eGClosed = 0,
        eGOpened = 1,
        eGOpening = 2,
        eGClosing = 3,
    };

    class ZHM3LevelControlM05 : public ZHM3LevelControl
    {
    public:
        // vtbl
        // data
        int32_t m_iTargetsToKill;

        Glacier::REFTAB m_lBurn;
        Glacier::REFTAB m_lBurnFireEmitters;
        Glacier::REFTAB m_lBurnSmokeEmitters;

        Glacier::ZREF m_rCameraGuard01;
        Glacier::ZREF m_rCameraGuard02;
        Glacier::ZREF m_rCaterer;
        Glacier::ZREF m_rWitness;
        Glacier::ZREF m_rWitnessWife;
        Glacier::ZREF m_rWifeBurnedModel;
        Glacier::ZREF m_rBBQ;
        Glacier::ZREF m_rGarageDoor;
        Glacier::ZREF m_rGarageDoorActionArea;
        Glacier::ZREF m_rExitBox;
        Glacier::ZREF m_rDonutCrate;
        Glacier::ZREF m_rDonutHack;
        Glacier::ZREF m_rAirgun;
        Glacier::ZREF m_rDarts;
        Glacier::ZREF m_rGarageDoorSound;
        Glacier::ZREF m_rGaragePathfinderExitBox;

        Glacier::ZItem* m_pDonutCrate;
        Glacier::ZItem* m_pDonutHack;
        Glacier::ZGEOM* m_pGarageGeom;
        Glacier::ZGEOM* m_pGarageDoorActionArea;

        Glacier::ZRunMatPosAnim m_GarageAnim;
        eGarageState m_eState;
        float m_fGarageAnimTime;
        Glacier::ZREF m_rGarageDoorAction;

        Glacier::ZVector3 m_vGaragePathfinderLinksBoxMin;
        Glacier::ZVector3 m_vGaragePathfinderLinksBoxMax;
        Glacier::ZVector3 m_vGarageDoorOffset;
        Glacier::ZMat3x3 m_mGarageDoorOffset;

        bool m_bBurnWife;
        RE_ADD_PADDING(3);
        float m_fTimeSmokeWife;
        float m_fTimeBurnWife;
        float m_fTimeEmitBurnWife;
        float m_fNewEmitTime;
        bool m_bNecklacePickedUp;
        bool m_bSwapWifeModel;
        bool m_bWifeBurned;
        bool m_bGetMoreGuards;

        Glacier::ZMSGID m_msgAddSuspiciousGuard;
        Glacier::ZMSGID m_msgSubSuspiciousGuard;
        Glacier::ZMSGID m_msgAddThreateningGuard;
        Glacier::ZMSGID m_msgSubThreateningGuard;
        Glacier::ZMSGID m_msgAddDeadBodyFound;
        Glacier::ZMSGID m_msgCallForBackup;
        Glacier::ZMSGID m_msgImmediateBackup;
        Glacier::ZMSGID m_LevelEnd;

        int32_t m_KilledFBIGuards;
        int32_t m_GuardSendTimer;
        int32_t m_iLevelStartTime;
        int32_t m_iSuspiciousGuards;
        int32_t m_iThreateningGuards;
        int32_t m_iBodiesFound;
        int32_t m_iAllowedSuspiciousGuards;
    };
    RE_VERIFY_SIZE(ZHM3LevelControlM05, 0x734); // Verified
    RE_VERIFY_OFFSET(ZHM3LevelControlM05, m_rCameraGuard01, 0x630);
    RE_VERIFY_OFFSET(ZHM3LevelControlM05, m_GarageAnim, 0x680);
    RE_VERIFY_OFFSET(ZHM3LevelControlM05, m_lBurn, 0x5DC);
    RE_VERIFY_OFFSET(ZHM3LevelControlM05, m_lBurnFireEmitters, 0x5F8);
    RE_VERIFY_OFFSET(ZHM3LevelControlM05, m_lBurnSmokeEmitters, 0x614);
    RE_VERIFY_OFFSET(ZHM3LevelControlM05, m_vGarageDoorOffset, 0x6C0); // Verified by ZHM3LevelControlM05::Animate
    RE_VERIFY_OFFSET(ZHM3LevelControlM05, m_mGarageDoorOffset, 0x6CC); // Verified by ZHM3LevelControlM05::Animate
    RE_VERIFY_OFFSET(ZHM3LevelControlM05, m_bGetMoreGuards, 0x707); // Verified by m_bGetMoreGuards
    RE_VERIFY_OFFSET(ZHM3LevelControlM05, m_KilledFBIGuards, 0x718); // Verified by ZHM3LevelControlM05::CharacterKilled
}
