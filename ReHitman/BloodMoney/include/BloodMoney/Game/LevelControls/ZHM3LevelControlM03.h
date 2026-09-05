#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <BloodMoney/Game/LevelControls/ZHM3LevelControl.h>


namespace Hitman::BloodMoney
{
    class ZHM3LevelControlM03 : public ZHM3LevelControl
    {
    public:
        // vtbl
        // data
        Glacier::ZREF    m_rChandelierDebris;
        Glacier::ZREF    m_rMauserClothes;
        Glacier::ZREF    m_rBoxHitmanBomb1;
        Glacier::ZREF    m_rBoxHitmanBomb2;
        Glacier::REFTAB m_rtFightRats;
        Glacier::REFTAB m_rtFightRatsGloves;
        Glacier::REFTAB m_rtPokerRats;
        Glacier::REFTAB m_rtPokerRatsCards;
        Glacier::REFTAB m_rtPokerRatsHats;
        Glacier::REFTAB m_rtPokerRatsCigars;
        Glacier::REFTAB m_rtWaitressRats;
        Glacier::REFTAB m_rtWaitressRatsTrays;
        Glacier::ZREF    m_rOperaSinger;
        Glacier::ZREF    m_rAmbassador;
        Glacier::ZGEOM* m_pCamera;
        Glacier::ZGEOM* m_pCameraFlash;
        bool   m_bIgnoreWeapons;
        bool   m_bHitmanInRatRoom;
        int8_t m_iBombAddedToHitmanGeomNumbe;
        Glacier::ZMSGID m_msg_evM03_EscapeCutEnd;
    };
    RE_VERIFY_SIZE(ZHM3LevelControlM03, 0x6E0); // Verified
}