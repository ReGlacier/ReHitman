#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <BloodMoney/Game/LevelControls/ZHM3LevelControl.h>


namespace Hitman::BloodMoney
{
    class ZHM3LevelControlM12 : public ZHM3LevelControl
    {
    public:
        // vtbl (no changes)
        // data
        Glacier::ZREF m_rMrX;
        Glacier::ZREF m_rAlbino;
        Glacier::ZREF m_rAlbinoVictim;
        Glacier::ZREF m_rMontre;
        Glacier::ZREF m_rMontreAlarm;
        Glacier::ZREF m_rCheckInsideMontre;
        Glacier::ZREF m_rCheckInsideEntrance;
        Glacier::ZREF m_rCheckInsideEntranceTo;
        Glacier::ZREF m_rMoveObject;
        Glacier::ZVector3 m_vMoveTo;
        Glacier::ZVector3 m_vMoveFrom;
        float m_fMoveTime;
        float m_fMoveStartTime;
        float m_fXrayYOffset;
        bool m_bHighNoonActive;
        bool m_bHighNoonSilenced;
        Glacier::ZGEOM* m_pCamera;
        Glacier::ZGEOM* m_pCameraFlash;
    };
    RE_VERIFY_SIZE(ZHM3LevelControlM12, 0x62C); // Verified
}