#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Fysix/COLI.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Vehicle/BaseMoveModel.h>
#include <Glacier/Vehicle/ZCarMoveModelIF.h>


namespace Glacier
{
    // It's kinda like C++ std::vector but from 2001
    template <typename T> struct ZSTLPVectorBase
    {
        T* _M_start;
        T* _M_finish;
        T* _M_data;
    };
    RE_VERIFY_SIZE(ZSTLPVectorBase<int>, 0xC);  // Should be valid

    class CParticle1D 
    {
    public:
        float m_fPos;
        float m_fPrePos;
        float m_fForce;
        float m_fMinPos;
        float m_fInvMass;

        virtual void LoadSave(ISerializerStream&, bool);
    };
    RE_VERIFY_SIZE(CParticle1D, 0x18);

    class CSpring1D 
    {
    public:
        float m_fCurLen;
        float m_fPreLen;
        float m_fCurSpd;
        float m_fK;
        float m_fD;
        float m_fRestLe;
        CParticle1D* m_pP1;
        CParticle1D* m_pP2;

        virtual void LoadSave(ISerializerStream&, bool);
    };
    RE_VERIFY_SIZE(CSpring1D, 0x24);

    class CDamper
    {
    public:
        // types
        enum ECarSide : int
        {
            eCarLeft = 0x0,
            eCarRight = 0x1,
        };

        enum ECarEnd : int
        {
            eCarFront = 0x0,
            eCarBack = 0x1,
        };

        // vtbl
        virtual void LoadSave(ISerializerStream&, bool);

        // data
        ZVector3 m_vPos;
        float m_fGravityFac;
        bool m_bDamperAtMinLe;
        float m_fGravity;
        float m_fMinLen;
        float m_fMaxLen;
        float m_fMinLenForSoun;
        CSpring1D m_Spring;
        CParticle1D m_Wy0;
        CParticle1D m_Wy1;
        COLI m_Coli;
        uint8_t m_iBoneWheelID;
        ECarEnd m_eCarEnd;
        ECarSide m_eCarSide;
    };
    RE_VERIFY_SIZE(CDamper, 0xF8);

    class OrgCarMoveModel : public BaseMoveModel, public ZCarMoveModelIF
    {
    public:
        // types
        enum EDampers : int
        {
            eFRONT_RIGHT = 0x0,
            eFRONT_LEFT = 0x1,
            eBACK_LEFT = 0x2,
            eBACK_RIGHT = 0x3,
            eBACK_LEFT_2 = 0x4,
            eBACK_RIGHT_2 = 0x5,
            _E_WHEEL_DAMPERS_END = 0x6,
        };

        // vtbl (no new members?)
        // data
        ZVector3 m_vPos;
        ZMat3x3 m_mMat;
        ZSTLPVectorBase<CDamper> m_Dampers;
        ZVector3 m_vFrontAxelPosNew;
        ZVector3 m_vRearAxelPos;
        ZVector3 m_vFrontAxelPos;
        ZVector3 m_vFrontAxelDistance;
        ZVector3 m_vRearAxelDistance;
        int m_iFrontWheelDistance;
        int m_iRearWheelDistance;
        float m_fAngle;
        ZVector3 m_vOldWheelPos[4];
        float m_fWheelRotAngle[4];
        bool m_bReversing;
        RE_ADD_PADDING(3);
        float m_fSteerAngle;
        float m_fTiltAngle[2];
        float m_fRollAngle[2];
        int m_iFrontFindGroundCounte;
        int m_iBackFindGroundCounter;
        float m_fPreUpdateTime;
        ZVector3 m_vHorDir;
        ZVector3 m_vOldHorDir;
        float m_fTurnAngle;
        float m_fTurnAngleSpd;
        float m_bDampersAtMinLen;
        float m_fGravityAccelParam;
        float m_fGravityTurnSpdParam;
        float m_fTimeStep;
        float m_fLeftRightWheelDistX;
        float m_fFrontBackWheelDistZ;
    };
    RE_VERIFY_SIZE(OrgCarMoveModel, 0x164);
}