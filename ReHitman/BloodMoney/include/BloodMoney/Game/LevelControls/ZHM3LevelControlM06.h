#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/TIMETYPE.h>
#include <Glacier/ZSTL/REFTAB32.h>
#include <BloodMoney/Game/LevelControls/ZHM3LevelControl.h>
#include <BloodMoney/Game/OnLevel/ZMardiGrassFloat.h>

namespace Hitman::BloodMoney
{
    class ZHM3Actor;

    class ZHM3LevelControlM06 : public ZHM3LevelControl
    {
    public:
        // vtbl
        virtual void CompleteObjective(int iObjectiveIdx);

        // data
        bool m_bSniperRifleInCurrentRoom;
        bool m_bTargetInSight;
        bool m_bAssassinInMeeting;
        Glacier::ZREF m_rLeader;
        Glacier::ZREF m_rRaymond;
        Glacier::ZREF m_rAngelina;
        Glacier::ZREF m_rBarfingGuy;
        Glacier::REFTAB m_lPuke;
        Glacier::ZAUDIOREF m_lWalkieStaticResource;
        Glacier::ZREF m_rFloatPolitician;
        Glacier::REFTAB32 m_rtFloatActors;
        Glacier::REFTAB32 m_rtFloatActorsPos;
        Glacier::ZREF m_rRaymondRifle;
        Glacier::ZREF m_rBluesRoom;
        Glacier::ZREF m_rRockRoom;
        Glacier::ZREF m_rRockRoom2;
        Glacier::ZREF m_rSalsaRoom;
        ZHM3Actor* m_apFloatActors[5];
        Glacier::ZGEOM* m_apFloatActorsPos[5];
        Glacier::ZGEOM* m_pLeader;
        Glacier::ZGEOM* m_pRaymond;
        Glacier::ZGEOM* m_pAngelina;
        Glacier::ZGEOM* m_pRaymondRifle;
        ZMardiGrassFloat* m_pFloat;
        Glacier::TIMETYPE m_ttLastMoveTime;
        Glacier::Animation::Header* m_pAnimBlues;
        Glacier::Animation::Header* m_pAnimRock;
        Glacier::Animation::Header* m_pAnimSalsa;
        bool m_bDancing;
        bool m_bFloatsMoving;
        Glacier::ZREF m_rClubMusic;
        Glacier::ZREF m_rRaymondInWalkie;
        Glacier::REFTAB32 m_rtRadioMusicSources;
        int32_t m_lPosEnum;
        int32_t m_lClubMusicId;
        Glacier::ZROOM* m_pOldHitmanRoom;
    };
    RE_VERIFY_SIZE(ZHM3LevelControlM06, 0x88C); // Verified
}