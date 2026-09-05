#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZSTL/ZHash.h>
#include <BloodMoney/Game/LevelControls/ZHM3LevelControl.h>


namespace Hitman::BloodMoney
{
    struct M00Data 
    {
        Glacier::ZMSGID m_msgShowTutorial;
        Glacier::ZMSGID m_msgShowHint;
        Glacier::ZMSGID m_msgKnockKnock;
        Glacier::ZMSGID m_msgTesterOverRail;
        Glacier::ZMSGID m_msgGateCutFinished;
        Glacier::ZMSGID m_msgIntroCutFinished;
        int m_bFirstFrame;
        bool m_bTargetStillAliveHintShow;
        float m_fRespawnCountdown;
        Glacier::ZREF m_rCheckPoint;
        int32_t m_iTestersKilled;
        int32_t m_iTestersHidden;
        int32_t m_iGuadsForSnipingKilled;
        bool m_bBombPickedup;
        bool m_bBombRemotePickedup;
        bool m_bAtGate;
        Glacier::TIMETYPE m_tBriefingFirstTime;
        bool m_bPsychoBobHasGun;
        bool m_bPsychoBobDead;
        bool m_bShowHolsterGunHint;
        bool s_Music_M00_ChemistDead;
        bool m_bAddExtraAmmoToShotgun;
        bool m_bAddExtraAmmoToTmp;
        bool m_bAddExtraAmmoToSecretTmp;
    };
    RE_VERIFY_SIZE(M00Data, 0x38);

    class ZHM3LevelControlM00 : public ZHM3LevelControl
    {
    public:
        // vtbl (no changes)
        // data
        Glacier::ZPStrHash<int> m_TutorialsHintsShown;
        bool m_bSwingKingDead;
        bool m_bScoopDead;
        RE_ADD_PADDING(2);
        Glacier::REFTAB m_lBurn;
        Glacier::REFTAB m_lBurnFireEmitters;
        Glacier::REFTAB m_lBurnSmokeEmitters;
        Glacier::ZREF m_rGasolineEmitter;
        Glacier::ZREF m_rGasolineCan;
        Glacier::ZGEOM* m_pGasolineEmitter;
        Glacier::ZItem* m_pGasolineCan;
        Glacier::ZItemContainer* m_pCrateBakingSoda;
        bool m_bBurnLawyer;
        bool m_bWaitForForcedBriefingClose;
        RE_ADD_PADDING(2);
        float m_fTimeEmitBurnLawyer;
        float m_fTimeBurnLawyer;
        float m_fNewEmitTime;
        float m_fTimeSmokeLawyer;
        Glacier::ZREF m_rLawyer;
        Glacier::ZREF m_rLawyerBurnedModel;
        Glacier::ZREF m_rActionEscape;
        bool m_bExitBoxShown;
        RE_ADD_PADDING(3);
        Glacier::ZREF m_rElephant;
        Glacier::ZREF m_rElephantBurned;
        M00Data* _p;
        M00Data* _StoredData;
        bool m_bMoveGasolineEmitter;
        bool m_bSwapLawyerModel;
        bool m_bChemistClothesOn;
        bool m_bChemistKeycardPickedUp;
        bool m_bShowDropCrateTip;
        bool m_bCokeGuard1Down;
        bool m_bCokeGuard2Down;
        RE_ADD_PADDING(1);
        Glacier::ZREF m_rChemist;
        Glacier::ZREF m_rFactoryRoom;
        Glacier::ZREF m_rCrateBakingSoda;
        Glacier::ZREF m_rTestingRoomDoor1;
        Glacier::ZREF m_rTestingRoomDoor2;
        Glacier::ZREF m_rHMPositionAfterGateCut;
    };
    RE_VERIFY_SIZE(ZHM3LevelControlM00, 0x6B8); // Verified
    RE_VERIFY_OFFSET(ZHM3LevelControlM00, m_bSwingKingDead, 0x5f8);
}