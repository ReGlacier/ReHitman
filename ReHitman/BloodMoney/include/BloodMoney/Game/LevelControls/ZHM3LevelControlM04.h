#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/SSplineMover.h>
#include <Glacier/PF4/Fwd.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <BloodMoney/Game/LevelControls/ZHM3LevelControl.h>


namespace Hitman::BloodMoney
{

    enum eAgentSmithState 
    {
        eANormal = 0,
        eAFakeDeath = 1,
        eAInTheMorgue = 2,
        eARevived = 3,
        eADead = 4,
    };

    enum eGlobeLidState 
    {
        eClosed = 0,
        eOpened = 1,
        eOpening = 2,
        eClosing = 3,
    };

    struct SM04Vars 
    {
        uint8_t m_bMainTargetDead    : 1;
        uint8_t m_bHMInDropWeightBox : 1;
        uint8_t m_bExitBoxShown      : 1;
        uint8_t m_bHMInLocker        : 1;
        uint8_t m_bOptTarget1Dead    : 1;
        uint8_t m_bOptTarget2Dead    : 1;
        uint8_t m_HMTalkedToAgent    : 1;
        uint8_t m_bMoveBed           : 1;
    };

    struct SM04HMasClothes 
    {
        uint8_t m_bHMasHitman    : 1;
        uint8_t m_bHMasBodyguard : 1;
        uint8_t m_bHMasPatient   : 1;
        uint8_t m_bHMasGuard     : 1;
        uint8_t m_bHMasWarden    : 1;
        uint8_t m_bHMasTherapist : 1;
        uint8_t m_pad            : 2;
    };

    class ZHM3LevelControlM04 : public ZHM3LevelControl
    {
    public:
        // vtbl
        // data
        Glacier::REFTAB m_rTargets;
        Glacier::ZREF m_rReceptionNurse;
        Glacier::ZREF m_rMetalDoorGuard;
        Glacier::ZREF m_rAgentSmith;
        Glacier::REFTAB m_rTargetPhotos;
        Glacier::ZREF m_rSpecWarden;
        Glacier::ZREF m_rHospitalBed;
        Glacier::ZREF m_rCellDoor;
        Glacier::ZREF m_rElevDoor;
        Glacier::ZREF m_rPathToMorgueSpline;
        Glacier::ZREF m_rGlobe;
        Glacier::ZAUDIOREF m_rGlobeOpenSound;
        Glacier::ZAUDIOREF m_rGlobeCloseSound;
        Glacier::ZREF m_rDumbBell;
        Glacier::ZREF m_rActionEscape;
        Glacier::ZREF m_rLockerBox;
        Glacier::ZREF m_rGasTank;
        Glacier::ZREF m_rBalconyGuy;
        Glacier::ZREF m_rMobilePhone;
        Glacier::ZREF m_rsGasTankSound;
        Glacier::ZMSGID m_M04_MissionComplete;
        Glacier::ZMSGID m_msgEnterBoxEnter;
        Glacier::ZMSGID m_msgExitBoxEnter;
        Glacier::ZMSGID m_msgStartSplineMoving;
        Glacier::REFTAB m_bHMasBodyguardClothe;
        Glacier::REFTAB m_bHMasPatientClothes;
        Glacier::REFTAB m_bHMasGuardClothes;
        Glacier::REFTAB m_bHMasWardenClothes;
        Glacier::REFTAB m_bHMasTherapistClothe;
        Glacier::ZREF m_rFinalTargetRef;
        Glacier::ZREF m_rFinalTargetPhoto;
        Glacier::ZREF m_rOptTarget1;
        Glacier::ZREF m_rOptTarget2;
        eAgentSmithState m_eAgentState;
        Glacier::ZGEOM* m_pEvCamPos1;
        Glacier::ZGEOM* m_pEvCamPos2;
        Glacier::ZREF m_rAgentAction;
        Glacier::ZGEOM* m_pGlobeLid;
        eGlobeLidState m_eGlobeLidState;
        float m_fAnimLength;
        Glacier::ZREF m_rGlobeAction;
        Glacier::ZREF m_rGasAction;
        Glacier::ZREF m_rDumbBellAction;
        int m_iHitmanInMedWing;
        SM04Vars m_Vars;
        Glacier::PF4::ZDynamicObstacle* m_pObstacle;
        Glacier::SSplineMover m_sSplineMover;
        SM04HMasClothes m_HMas;
    };
    RE_VERIFY_SIZE(ZHM3LevelControlM04, 0x77C); // Verified
}