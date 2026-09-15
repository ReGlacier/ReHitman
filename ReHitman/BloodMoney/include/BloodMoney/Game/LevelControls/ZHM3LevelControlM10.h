#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <BloodMoney/Game/LevelControls/ZHM3LevelControl.h>


namespace Hitman::BloodMoney
{
    class ZHM3LevelControlM10 : public ZHM3LevelControl
    {
    public:
        // vtbl (no changes)
        // data
        Glacier::ZREF m_r7thFloorBox;
        Glacier::ZREF m_r8thFloorBox;
        Glacier::ZREF m_r7thFloorEmergencySwitch;
        Glacier::ZREF m_r8thFloorEmergencySwitch;
        Glacier::ZREF m_r7thFloorFireAlarm;
        Glacier::ZREF m_r8thFloorFireAlarm;
        Glacier::ZREF m_rSuitcase_Diamonds;
        Glacier::ZMSGID m_M10_MissionComplete;
        Glacier::ZMSGID m_M10_HMPutDownSuitcase;
        Glacier::ZGEOM* m_pCamera;
        Glacier::ZGEOM* m_pCameraFlash;
        Glacier::ZAction* m_pExitAction;
    };
    RE_VERIFY_SIZE(ZHM3LevelControlM10, 0x604); // Verified
}