#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/PF4/Fwd.h>
#include <BloodMoney/Game/LevelControls/ZHM3LevelControl.h>


namespace Hitman::BloodMoney
{
    class ZHM3LevelControlM02 : public ZHM3LevelControl
    {
    public:
        // vtbl (no changes)
        // data
        Glacier::ZREF m_rLaserBeamCheckInside;
        Glacier::ZREF m_rGovernorSon;
        Glacier::ZREF m_rSlapGirl;
        Glacier::ZREF m_rGovSonDrink;
        Glacier::ZREF m_rAphrodisiac;
        Glacier::ZREF m_rlvlctrl2;
        Glacier::ZREF m_rBottleChampagne;
        Glacier::ZREF m_rVIPDrink;
        Glacier::ZREF m_rVIPGuest;
        Glacier::ZREF m_rStaffFuseboxGuard;
        Glacier::ZREF m_rJacuzziGirl1;
        Glacier::ZREF m_rJacuzziGirl2;
        Glacier::ZREF m_rJacuzziGirl3;
        Glacier::ZREF m_rWaiter;
        Glacier::ZREF m_rExitBoxPier;
        Glacier::ZREF m_rExitBoxHelipad;
        Glacier::ZREF m_rPilot;
        Glacier::ZREF m_rPianoBartender;
        Glacier::ZREF m_rPornBoss;
        Glacier::ZREF m_rPoisonGlassAction;
        Glacier::ZREF m_rJacuzziWaterBox;
        Glacier::ZREF m_rEscapeVideoPlayer;
        Glacier::ZREF m_rJacuzziPathfinderCutout;
        bool m_bPornBossDead;
        bool m_bGovenorSonDead;
        bool m_bVideoTapeRetrieved;
        bool m_bHeliKeyRetrieved;
        bool m_bAphroUsed;
        bool m_JacuzziGoner;
        Glacier::ZMSGID m_msgJumpBalcony;
        Glacier::ZMSGID msg_CheckInsideEnter;
        Glacier::ZMSGID msg_CheckInsideLeave;
        Glacier::ZMSGID msg_M02_UseAphrodisiac;
        Glacier::ZMSGID msg_M02_UseAphrodisiacAlternativ;
        Glacier::ZMSGID msg_M02_PourChampagne;
        Glacier::ZMSGID msg_M02_Jacuzzicrash;
        Glacier::ZMSGID msg_M02_ExitBoxHelipad;
        Glacier::ZMSGID msg_M02_ExitBoxPier;
        Glacier::ZMSGID msg_M02_HeliKeyCheck;
        Glacier::ZMSGID msg_M02_CompleteLevelHelipad;
        Glacier::PF4::ZDynamicObstacle* m_pObstacle;
        ZPostFilterCollectionEvent* m_pPFCollection;
        ZPostFilterEvent* m_pPF;
        Glacier::Animation::Header* m_pAniHMPoisonBottle;
        bool m_bCompletedAfterMovie;
    };
    RE_VERIFY_SIZE(ZHM3LevelControlM02, 0x664); // Verified
}