#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <BloodMoney/Game/LevelControls/ZHM3LevelControl.h>


namespace Hitman::BloodMoney
{
    class ZHM3LevelControlM08 : public ZHM3LevelControl
    {
    public:
        // vtbl
        virtual void CompleteObjective(int iObjectiveIdx);
        
        // data
        Glacier::ZLIST* m_pRiverBanksList;
        Glacier::ZREF m_rPowerboxGuard;
        Glacier::ZREF m_rRightSidePiston;
        Glacier::ZREF m_rRightSidePlejelstang;
        Glacier::ZREF m_rRightSideValveStick;
        Glacier::REFTAB m_pRightSidePropellarObject;
        Glacier::ZREF m_rDeathBox1;
        Glacier::ZREF m_rDeathBox2;
        Glacier::ZREF m_rLeftSidePiston;
        Glacier::ZREF m_rLeftSidePlejelstang;
        Glacier::ZREF m_rLeftSideValveStick;
        Glacier::REFTAB m_pLeftSidePropellarObjects;
        Glacier::ZREF m_rLeftPartitlesystem1;
        Glacier::ZREF m_rCake;
        Glacier::ZREF m_rCakeHack;
        Glacier::ZREF m_rUtilBoxControlledThings;
        Glacier::ZREF m_rFoodCycleController;
        Glacier::ZREF m_rBuffetOne;
        Glacier::ZREF m_rBuffetTwo;
        Glacier::ZREF m_rCakeGoesHere;
        Glacier::ZREF m_rBoss;
        Glacier::ZItem* m_pCake;
        Glacier::ZItem* m_pCakeHack;
    };
    RE_VERIFY_SIZE(ZHM3LevelControlM08, 0x664); // Verified
}