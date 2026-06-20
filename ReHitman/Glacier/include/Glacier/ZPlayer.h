#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/IK/ZLNKWHANDS.h>
#include <Glacier/Glacier.h>
#include <Glacier/PF4/ZMetaNode.h>


namespace Glacier
{
    union STempStripsUniqueId
    {
        struct
        {
            uint32_t lIdLo;
            uint32_t lIdHi;
        };
        uint64_t lId;
    };
    RE_VERIFY_SIZE(STempStripsUniqueId, 0x8);

    struct ZActiveImpactPrim
    {
        uint32_t m_lType;
        uint32_t m_lImpactTriangleNr;
        ZVector3 m_vCorner1;
        ZVector3 m_vCorner2;
        ZVector3 m_vCorner3;
        ZVector3 m_vNormal;
        ZBaseGeom* m_pBaseGeom;
        RE_ADD_PADDING(4);
        STempStripsUniqueId m_StripId;
        bool m_bInvalid;
        bool m_bNoEnter;
        RE_ADD_PADDING(6);
    };
    RE_VERIFY_SIZE(ZActiveImpactPrim, 0x50);

	struct ZSphereImpact
	{
		unsigned int m_lNrActiveImpacts;
		unsigned int m_lNrFramesOffGround;
		STempStripsUniqueId m_StripId;
		ZActiveImpactPrim m_ActiveImpacts[4];
		int m_iGroundMaterial;
		bool m_bLocal;
		bool m_bGroundContact;
		RE_ADD_PADDING(2);
	};
	RE_VERIFY_SIZE(ZSphereImpact, 0x158);

    struct ZMovementBase;

	struct ZPlayer : public ZLNKWHANDS
	{
		// Data starts from 0x3D0, total size is 0x768
        bool m_bReady;
        RE_ADD_PADDING(3);
        uint32_t m_rDragTarget;
        uint32_t m_CurrentStatus;
        PF4::ZMetaNode m_Entity;
        void* m_pkBoid; // wtf?
        int32_t m_iPlayerNum;
        RE_ADD_PADDING(4);
        ZSphereImpact m_SphereImpact;
        Animation::Header* m_pRemLastAnim;
        float m_fRemLastAnimStartTime;
        bool m_bGroundContact;
        RE_ADD_PADDING(3);
        ZVector3 m_vGravity;
        ZVector3 m_vCenterSize;
        ZVector3 m_vCenterSpeed;
        float m_fSpeed;
        float m_fCenterHeight;
        int32_t m_lControlsLockCount;
        bool m_bBreathe_Inhale;
        RE_ADD_PADDING(3);
        float m_fBreathe_Interval;
        float m_fBreathe_Intensity;
        float m_fBreathe_WantedIntensity;
        float m_fPelvisBobPhase;
        float m_fPelvisBobAmplitude;
        float m_fStrideLength;
        float m_fPelvisBob;
        bool m_UseCamFacing;
        RE_ADD_PADDING(3);
        float m_fCamHorizontalDelta;
        float m_fCamVerticalDelta;
        ZMovementBase* m_Movements[32];
        ZMovementBase* (*m_pMovements)[32];
        ZMovementBase* m_pActiveMovement;
        uint32_t m_lHighestMovementType;
        ZMat3x3 m_mPrevContact;
        ZVector3 m_vPrevContact;
        ZMat3x3 m_mPushBoxMatrixFrom;
        ZVector3 m_vPushBoxPositionFrom;
        ZMat3x3 m_mPushBoxMatrixTo;
        ZVector3 m_vPushBoxPositionTo;
        ZVector3 m_vPushBoxSize;
        bool m_bPushBox;
        RE_ADD_PADDING(3);
        float m_fLastUpdateTime;
        ZVector3 m_mRealPos;
        bool m_bRealPosSet;
        RE_ADD_PADDING(3);
        uint32_t m_rRemContactGeom; // ZREF
        ZMat3x3 m_mRemPush;
        ZVector3 m_vRemPush;
        bool m_bT1Disabled;
        bool m_bCollisionEnabled;
        bool m_bControlsEnabled;
        bool m_bDisplayCollisionsBounds;
        int32_t m_lCollisionLockCount;
        int32_t m_lPushPopStateLockCount;
        float m_fLastAnimPrc;
        uint16_t m_msgDragDeadBody;
        uint16_t m_msgDestroyDrag;
        uint16_t m_msgEnableControls;
        uint16_t m_msgDisableControls;
        uint16_t m_msgMoveModify;
        RE_ADD_PADDING(2);
        int32_t m_iDisplayParentBox;
        bool m_bControlGotDisabled;
        RE_ADD_PADDING(3);
	};
	RE_VERIFY_SIZE(ZPlayer, 0x768);
}