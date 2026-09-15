#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <BloodMoney/Game/LevelControls/ZHM3LevelControl.h>


namespace Hitman::BloodMoney
{
    class ZHM3Actor;

    class ZHM3LevelControlM11 : public ZHM3LevelControl
    {
    public:
        // vtbl (no changes)
        // data
        Glacier::REFTAB m_lNoDisarm;
        Glacier::ZREF m_rBarfingGuy;
        Glacier::ZREF m_rPartyGirlArranger;
        Glacier::ZREF m_rArrangerSword1;
        Glacier::ZREF m_rArrangerSword3;
        Glacier::REFTAB m_lPuke;
        Glacier::REFTAB m_lBurn;
        Glacier::ZREF m_rSpitDrink;
        Glacier::ZREF m_rProjectorFilmRollTop;
        Glacier::ZREF m_rProjectorFilmRollBotto;
        Glacier::ZREF m_rEatingShark;
        Glacier::ZREF m_rWaterBox;
        Glacier::ZREF m_rHeavenMusic;
        Glacier::ZREF m_rHellMusic;
        Glacier::ZREF m_rKaraokeMusic;
        Glacier::ZREF m_rKaraokeVoice;
        Glacier::ZREF m_rHackValidHitmanPos;
        bool m_bKaraokeMusicPlaying;
        bool m_bKaraokeVoicePlaying;
        bool m_bColiCheckArranger;
        bool m_bColiCheckAngel;
        ZHM3Actor* m_pArranger;
        ZHM3Actor* m_pAngel;
        ZHM3Item* m_pColiCheckItem[3];
        ZHM3Actor* m_pColiCheckActor[3];
        float m_fColiCheckLength[3];
        float m_fTimeNoDamage;
        float m_fWaterBoxSendTime;
        bool m_bBurnArranger;
        float m_fTimeBurnArranger;
        Glacier::ZREF m_rSpittingPerson;
        bool m_bProjectorRunning;
        Glacier::SREF m_rKaraokeVocals;
        Glacier::ZREF m_rKaraokeSinger;
        float m_TimeKaraokeStarted;
        Glacier::ZREF m_rCrowdController;
        Glacier::ZMSGID m_msg_evM11_EscapeCutEnd;
    };
    RE_VERIFY_SIZE(ZHM3LevelControlM11, 0x6C0); // Verified
}