#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/Vehicle/BaseMover.h>


namespace Glacier
{
    enum EMoveState
    {
        eNoDestRequested = 0,
        eMovingToDest = 1,
        eStoppingAtDest = 2,
        eFreeMove = 3,
    };

    enum ETurnState 
    {
        eStrait = 0,
        eTurnDetected = 1,
        eMovingToTurn = 2,
        eTurning = 3,
        eTurnDone = 4,
    };


    class PathMover : public BaseMover
    {
    public:
        // vtbl (no new entries)
        // data
        PathFinderIF* m_pPathFinder;
        float m_fSpd;
        float m_fAcc;
        float m_fTargetSpd;
        float m_fDistToDest;
        bool m_bReachedDest;
        EMoveState m_eMoveState;
        ETurnState m_eTurnState;
        bool m_bStopAtDest;
        bool m_bTempStop;
        float m_fTempStopStartTim;
        float m_fDefaultSpd;
        float m_fDefaultTurnSpd;
        float m_fDefaultAcc;
        float m_fDefaultDecc;
        ZMSGID m_msgPathFinished;
    };
    RE_VERIFY_SIZE(PathMover, 0x360);
}
