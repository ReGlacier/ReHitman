#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <BloodMoney/Game/LevelControls/ZHM3LevelControl.h>
#include <BloodMoney/Game/ZPodiumController.h>

namespace Hitman::BloodMoney
{
    // Class definition ZHM3LevelControlM13
    class ZHM3LevelControlM13 : public ZHM3LevelControl
    {
    public:
        //Data (starts from +0x5D8)
        bool m_bMissionComplete;
        RE_ADD_PADDING(3);
        Glacier::ZREF m_rTable;
        Glacier::ZREF m_rTrapDoor;
        bool m_bFirstFrame;
        bool m_bHitmanRevived;
        bool m_bHitmanLeftTable;
        RE_ADD_PADDING(1);
        float m_fLeftTableTime;
        float m_fLastHeartBeat;
        int m_sndHeartBeat;
        float m_fHeartBeatBoost;
        Glacier::ZVector3 m_vOldDir;
        Glacier::ZREF m_rWheelChairGuy;
        Glacier::ZREF m_rJournalist;
        Glacier::ZREF m_rLimoDriver;
        Glacier::ZREF m_rPriest;
        Glacier::ZREF m_rEndCut;
        Glacier::ZREF m_rOutroMovie;
        bool m_bTriggerHumanShield;
        RE_ADD_PADDING(3);
        int m_iDeathToll;
        float m_fLevelCompleteTime;
        bool m_bTableMoving;
        RE_ADD_PADDING(3);
        ZPostFilterCollectionEvent* m_pPFCollection;
        ZPostFilterEvent* m_pPF;
        ZPostFilterSetEvent* m_pPFSet;
        ZPodiumController m_podiumControl;
    }; //End of ZHM3LevelControlM13 from 0057E6B8
    RE_VERIFY_SIZE(ZHM3LevelControlM13, 0x668);
    RE_VERIFY_OFFSET(ZHM3LevelControlM13, m_bMissionComplete, 0x5D8);
}