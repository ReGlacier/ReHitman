#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZSTL/ZHash.h>
#include <BloodMoney/Game/LevelControls/ZHM3LevelControl.h>


namespace Hitman::BloodMoney
{
    class ZHM3LevelControlM09 : public ZHM3LevelControl
    {
    public:
        // vtbl
        virtual void CompleteObjective(int iObjectiveIdx);

        // data
        Glacier::Animation::Header* m_pAnimSquareDance;
        Glacier::ZREF m_rFuseBox01;
        Glacier::ZREF m_rFuseBoxGuard01;
        Glacier::REFTAB m_rtAlligators;
        Glacier::ZREF m_rWaterBox;
        bool m_bIsGroomOnBandstan;
        float m_fWaterBoxSendTime;
        Glacier::REFTAB m_rtNoobs;
        bool m_bNoobsDone;
        Glacier::ZAUDIOREF m_rSoundBoatArrive;
        bool m_bBoatArriveDone;
        Glacier::REFTAB m_rtFemaleDancers;
        bool m_bCanHitmanDance;
        bool m_bDancing;
        float m_fDanceWaitTimer;
        Glacier::ZREF m_rDancePartner;
    };
    RE_VERIFY_SIZE(ZHM3LevelControlM09, 0x65C); // Verified
}