#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/CBaseEvent.h>
#include <Glacier/ZRunMatPosAnim.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Geom/ZGROUP.h>


namespace Hitman::BloodMoney
{
    enum eBCState 
    {
        STATE_OPEN = 0,
        STATE_CLOSE = 1,
        STATE_OPENING = 2,
        STATE_CLOSING = 3,
    };

    enum eBCType 
    {
        BC_CRATE = 0,
        BC_DUMPSTER = 1,
        BC_GARBAGETRUCK = 2,
    };

    // used in "ContainerState" ZMSG (via SendCommand("ContainerState", &state))
    struct sBodyContainerState 
    {
        int      m_iNumBodies;
        eBCState m_eState;
    };

    class CBodyContainer : public Glacier::CBaseEvent<Glacier::ZGROUP>
    {
    public:
        // vtbl
        // data
        Glacier::ZGEOM* m_pDoor;
        Glacier::ZGEOM* m_pOperatePoint;
        eBCState m_eState;
        eBCType m_eType;
        Glacier::ZAction* m_pAction;
        Glacier::ZREF m_rContainedBody;
        Glacier::ZAUDIOREF m_sndOpen;
        Glacier::ZAUDIOREF m_sndOpened;
        Glacier::ZAUDIOREF m_sndClose;
        Glacier::ZAUDIOREF m_sndClosed;
        Glacier::ZREF m_rDoor;
        Glacier::ZREF m_rOperatePoint;
        float m_fAnimTime;
        eBCState m_eInitialState;
        bool m_bRemoveBodies;
        Glacier::ZRunMatPosAnim m_Animation;
        Glacier::ZGEOM* m_pEndPosGeom;
        Glacier::ZMat3x3 m_mActorEndMat;
        Glacier::ZVector3 m_vActorEndPos;
        float m_fOpenTime;
    }; // Size 0xC0
    RE_VERIFY_SIZE(CBodyContainer, 0xC0);
}