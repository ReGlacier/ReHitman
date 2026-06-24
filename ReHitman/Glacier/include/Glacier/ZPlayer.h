#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/STempStripsUniqueId.h>
#include <Glacier/Geom/ZEntityLocator.h> // ZBaseGeom
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/IK/ZLNKWHANDS.h>
#include <Glacier/Glacier.h>
#include <Glacier/PF4/ZMetaNode.h>


namespace Glacier
{
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

    struct ZMoveNotify
    {
        bool m_bDeny;
        RE_ADD_PADDING(3);
        ZBaseGeom* m_pBaseGeom;
        float m_fWeight;
        ZVector3 m_vSize;
        ZMat3x3 m_m0;
        ZVector3 m_p0;
        ZMat3x3 m_m1;
        ZVector3 m_p1;
        float m_t;
    };
    RE_VERIFY_SIZE(ZMoveNotify, 0x7C);

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

        // vtbl
        virtual void DisplayCollisionsBounds(bool);
        virtual void DisableCollision();
        virtual void EnableCollision();
        virtual void DisableT1();
        virtual void EnableT1();
        virtual void DragBody(ZGEOM*);
        virtual void GetCenterMatPos(float*, float*);
        virtual void GetCenterSphere(float*, float*, float*);
        virtual bool GetGroundContact(void);
        virtual void SetGroundContact(bool);
        virtual ZGEOM* GetSuit();
        virtual void MoveCenterToMatPos(float const*, float const*);
        virtual float GetCenterHeight();
        virtual float* CenterSpeed(); // float[3] in ret
        virtual bool GodMode();
        virtual void SetGodMode(bool);
        virtual void Die(const ZVector3&, const ZVector3&, float);
        virtual void SetMoveSpeeds(const float*);
        virtual void GetMoveSpeeds(float*);
        virtual float GetActualSpeed();
        virtual bool ControlsEnabled();
        virtual int GetStatus();
        virtual bool IsWalking();
        virtual int GetActiveMovementType();
        virtual ZMovementBase* GetActiveMovement();
        virtual void SetMovementStatePointer(ZMovementBase**);
        virtual void SetActiveMovement(uint32_t);
        virtual const char* GetAnimNameFromCollision(ZIKLNKOBJ::SIKBoneCollision*, bool, int);
        virtual void ReloadRHandItem(ZItem*);
        virtual void ReloadLHandItem(ZItem*);
        virtual float GetLastAnimPrc();
        virtual bool GetActiveAnimMovement(float*, float*);
        virtual void SetLastAnimPrc(float);
        virtual void CallBackAimInPosition();
        virtual void HumanShieldLockSpineToHero();
        virtual void UpdateBreathing();
        virtual void SetGroupPos(const float*, const float*);
        virtual void UpdateAllDisplay();
        virtual void SetActiveUpperBody(float);
        virtual void GetCurrentUBAnim(Animation::Header**, uint32_t*, float*);
        virtual Animation::Header* GetCurrentUBAnim(uint32_t*);
        virtual void CalcBonesFromMotion(float const*, float const*, float const*, float const*);
        virtual void ControlBody(); /// <<< PURE VIRTUAL
        virtual void GetSpeedModifier(float*);
        virtual void OnCollision(const float*, const float*, ZMoveNotify*);
        virtual bool IsCollisionEnabled();
        virtual void OnMoveNotifyLargeObj(ZMoveNotify*);
        virtual void MovementControl(float*, float*, float*, float*);
        virtual uint32_t GetNearestEnemy(float const*, float const*);
        virtual bool IsValidEnemy(ZGEOM*);
        virtual void RegisterMovement(ZMovementBase*);
        virtual void CreateMovements();
        virtual ZMovementBase* CreateMovementBaseMove();
        virtual ZMovementBase* CreateMovementStand();
        virtual ZMovementBase* CreateMovementGuide();
        virtual ZMovementBase* CreateMovementAnimation();
        virtual ZMovementBase* GetMovement(uint32_t);
        virtual void GetCameraOrientation(float*, float*);
        virtual ZCAMERA* GetCamera();
	};
	RE_VERIFY_SIZE(ZPlayer, 0x768); // Approved by Glacier RTTI on PC
}
