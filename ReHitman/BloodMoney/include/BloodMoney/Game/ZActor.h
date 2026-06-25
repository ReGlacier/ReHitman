#pragma once

#include <Glacier/IK/ZLNKWHANDS.h>
#include <Glacier/Glacier.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/PF4/ZMetaNode.h>
#include <Glacier/PF4/ZPath.h>
#include <Glacier/ZSTL/ZMath.h>

namespace Hitman::BloodMoney
{
    enum ELocomotionState {
        LOCOSTATE_DISABLED = 0,
        LOCOSTATE_STAND = 1,
        LOCOSTATE_ACTIVEANIM = 2,
        LOCOSTATE_MANUALANIM = 3,
        LOCOSTATE_MOVEFORWARD = 4
    };

    struct ZLocomotionState 
    {
        Glacier::ZVector2 m_ActiveAnimPostTransitionDir;
        struct ZTransition* m_CurrentTransition;
        struct TAnimHeader* m_CurrentAnim;
        Glacier::Animation::ActiveAnimation* m_ActiveAnim;
        Glacier::PF4::ZMetaNode* m_LookAtTarget;
        float            m_LastDistanceToMotionChange;
        float            m_BlendOutAtFramePct;
        float            m_BlendOutToFramePct;
        float            m_BlendFrames;
        float            m_LastFramePrc;
        float            m_ExpectedMoveStopDist;
        float            m_ShortestMoveStopDist;
        float            m_MoveCycleCorrectionSpeed;
        int8_t           m_MoveSetNr;
        int8_t           m_PreferredState;
        int8_t           m_LastPreferredState;
        int8_t           m_LookMode;
        int8_t           m_LocomotionSetEntry;
        int8_t           m_LocomotionSetTransition;
        int8_t           m_CurrentTransitionSub;

        struct {
            ELocomotionState m_State : 4;
            ELocomotionState m_NextState : 4;
            unsigned int m_CurrentAnimMirrored : 1;
            unsigned int m_UseGroundAnimFully : 1;
            unsigned int m_InControlThisFrame : 1;
            unsigned int m_OffScreenLastFrame : 1;
            unsigned int m_AbortTransitionOnDistanceGrow : 1;
            unsigned int m_PerformQuickStart : 1;
            unsigned int m_LookAtEnabled : 1;
            unsigned int m_TurnToEndDir : 1;
            unsigned int m_CycleCorrectionAllowPct0 : 1;
            unsigned int m_CycleCorrectionAllowPct50 : 1;
        };

        // Idk, weird pad or missing data?
        uint8_t pad_3F8[4];
    };
    RE_VERIFY_SIZE(ZLocomotionState, 0x48);

    struct ProgramQueEntry 
    {
        uint8_t m_StartIndex;
        uint8_t m_EndIndex;
        int8_t  m_Program;
        int8_t  m_PathAction;
    };
    RE_VERIFY_SIZE(ProgramQueEntry, 0x4);

    struct ZSetBindings 
    {
        float     m_VariationSeed;
        int32_t   m_VariationMask;
        float     m_MoveSpeed;
        float     m_PreferredSpeed;
        float     m_MaxSpeed;
        float     m_Speed[10];
        float     m_PhaseShift[10];
        float     m_LongestStoppingDistance;
        float     m_Distance[20];
        float     m_TransitionRotation[20];
        Glacier::ZVector2 m_TransitionEndDirection[20];
        float m_BlendInFromPhase[20];
        float m_BlendOutToPhase[20];
        float m_QuickStartFramePct[20];
        float m_EndFramePct[20];
    };
    RE_VERIFY_SIZE(ZSetBindings, 0x2E8);

    struct LocomotionInfo 
    {
        float fDeltaTime;
        int stateChangeCount;
        bool allowActiveAnims;
        bool allowActiveAnimsBase;
        bool beingPushed;
        float currentSpeed;
        Glacier::ZVector2 currentDir;
        Glacier::ZVector2 currentAcc;
        float wantedSpeed;
        Glacier::ZVector2 wantedDir;
        float nextSpeed;
        Glacier::ZVector2 nextDir;
        float distanceToMotionChange;
        bool directLineToWaypoint;
        int stairsMode;
        Glacier::ZVector3 actorPos;
        Glacier::ZMat3x3  actorMat;
        Glacier::ZVector2 actorDir;
        Glacier::ZVector2 dirToTarget;
        struct ZSet* locomotionSet;
        ELocomotionState m_SwitchedFromState;
        float secondsToTransfer;
        float animationSpeedMultiplier;
        float rotationModifier;
        bool skipFramePctIncrease;
    };
    RE_VERIFY_SIZE(LocomotionInfo, 0x9C);

    class ZActor : public Glacier::ZLNKWHANDS
    {
    public:
        // pub
        enum ACTORSTATE : int8_t {
            ACTORSTATE_DEAD = 0,
            ACTORSTATE_AWAKE = 1,
            ACTORSTATE_UNCONSCIOUS = 2,
            ACTORSTATE_SLEEPING = 3,
        };

        enum PATH_CANCEL_REASONS : int8_t 
        {
            eNONE = 0,
            eANIMATION = 1,
            eMOVED_TO_POS = 2,
            eREQUEST = 3,
            eUNSPECIFIED = 4,
        };

        enum PATHDONEFLAG : int8_t 
        {
            PATH_RESERVED = 0,
            PATH_FINISHED = 1,
            PATH_CANCELED = 2,
            PATH_CANCELED_LOCKED_DOOR = 3,
            PATH_ERROR = 4,
        };

        enum PATH_ERRORS
        {
            NO_PATH = 0,
            EXEEDED_STOP_DIST = 1,
            EXEEDED_MAX_LEN = 2,
        };

        static constexpr Glacier::ZMSGID kAlarmMsg = 0x81A;

        // vftable
        virtual void SetActorRootTM(float const*,float const*);
        virtual void SetActorPosDir(float const*,float const*);
        virtual void GetActorRootTM(float *,float *);
        virtual void GetActorWorldPosition(float *);
        virtual void SetActorWorldPosition(float const*);
        virtual bool CanPlayAnimSegment(Glacier::Animation::Header*, float, float, float const*, float const*, bool, float, float);
        virtual bool IsUnconscious();
        virtual bool IsAwake();
        virtual bool IsSleeping();
        virtual void SetHero(uint32_t);
        virtual void SetActorState(ZActor::ACTORSTATE);
        virtual void OnViewEnter();
        virtual void OnViewLeave();
        virtual void* GetPathFinder4();
        virtual void EnablePathFinder(bool);
        virtual void SetStopDistance(float);
        virtual void SetEndDir(const Glacier::Vector3*);
        virtual const Glacier::Vector3* GetEndDir();
        virtual void MoveToPosition(Glacier::ZVector3*, Glacier::ZVector3*);
        virtual void SlideToPosition(Glacier::ZVector3*, Glacier::ZVector3*);
        virtual void SetPathNotify(float);
        virtual void SetPathNotifySyncToCycle(bool);
        virtual void SetEndAction(bool);
        virtual void RemovePath(uint8_t, ZActor::PATH_CANCEL_REASONS, bool);
        virtual void SendPathDone(bool);
        virtual void GetMoveFromPos(float *);
        virtual void* FindNextPathBlocker();
        virtual void SetShootIntoGround(bool);
        virtual void SetShootIntoGroundInFullbody(bool);
        virtual void SetStayInsidePath(bool);
        virtual void ShootIntoGround(float *, bool, bool);
        virtual void ShootIntoGroundRegularly(float *, bool, bool, float);
        virtual void SetMoveSpeedMultiplier(float);
        virtual void SetPathFinderEnabled(bool);
        virtual void OnPathRequest(uint32_t);
        virtual void OnPathFinished(uint32_t);
        virtual void OnPathCanceled(ZActor::PATH_CANCEL_REASONS);
        virtual void OnPathCanceledLockedDoor(uint32_t);
        virtual void OnPathError(ZActor::PATH_ERRORS);
        virtual void OnPathNotify(void);
        virtual void OnNewPath(void);
        virtual void OnSound(Glacier::REFTAB*);
        virtual void Die();
        virtual void DieByForce(float const*, float const*, float, uint32_t);
        virtual void Resurrect();
        virtual void Knockout();
        virtual void Revive();
        virtual void* GetKnockedOut();
        virtual void* GetActorList();
        virtual void SetDisableIdleAnimation(bool);
        virtual void* GetAudibleRoomList();
        virtual void LookAt(uint32_t);
        virtual bool IsSeeing();
        virtual void SetSeeing(bool);
        virtual bool IsVisible();
        virtual void SetVisible(bool);
        virtual void WantToLookAt(Glacier::ZGEOM*, bool);
        virtual void VerifyPlayerVisible();
        virtual void DeterminePathLookAt(Glacier::LocomotionInfo *, float *, float *, float *, float *);
        virtual bool CanLookAt(Glacier::ZGEOM*);
        virtual bool IsOnStairs();
        virtual void UpdatePositionOffScreen();
        virtual void UpdatePosition();
        virtual Glacier::Animation::Header* GetCurrentUBAnim(Glacier::ZItem*, Glacier::ZItem *, bool&);
        virtual void SetHoldWeaponUBAnim(Glacier::Animation::Header*);
        virtual uint32_t GetAnimOffset(bool const*);
        virtual uint32_t GetAnimOffset(Glacier::Animation::Header*);
        virtual Glacier::Animation::Header* GetAnimHeader(uint32_t);
        virtual void ShootIntoGroundCallback(Glacier::SExtendedImpactInfo*);

        // Total size is 0x900, base size is 0x3D0)
        Glacier::Vector3 m_ContactNormal;
        Glacier::Vector3 m_LastShootIntoGroundPosition;
        float m_LastShootIntoGroundHeight;
        float m_InterpolatedHeightError;
        Glacier::ZREF m_rHero;
        Glacier::ZHumanBoid* m_pkBoid;
        Glacier::PF4::ZMetaNode m_Entity;
        Glacier::PF4::ZMetaNode m_Reservation;
        ZLocomotionState m_Locomotion;
        uint16_t m_iQuePos;
        uint16_t m_iProgramQueSize;
        ProgramQueEntry m_ProgramQue[40];
        Glacier::PF4::ZPath m_Path;
        int8_t m_Action[2];
        int8_t m_iEndAction;
        RE_ADD_PADDING(1);
        int16_t m_iNextPathTarget;
        RE_ADD_PADDING(2);
        Glacier::REFTAB* m_pAudibleRoomList;
        float m_fOldFootStepFrame;
        uint32_t m_iPathfinderDoorKeyMask;
        Glacier::ZVector3 m_vInitialPos;
        Glacier::ZVector3 m_vInitialDir;
        uint32_t m_lInsideView;
        Glacier::ZREF m_rHeadTarget;
        float m_fMoveSpeedMultiplier;
        float m_FrameTimeAccumulator;
        float m_FrameTimeAccumulatorBound;
        Glacier::ZVector3 m_vEndDir;
        Glacier::ZVector3 m_vEndPos;
        float m_fEndDist;
        float m_fPathNotify;
        float m_fPathNotifyNext;
        PATHDONEFLAG m_lPathDoneFlag;
        PATH_CANCEL_REASONS m_ePathCancelReason;
        ACTORSTATE m_ActorState;
        RE_ADD_PADDING(1);
        ZSetBindings m_MoveSetBindings;
        float m_fCurrentBaseMoveFramePrc;
        float m_fCurrentBaseMoveDuration;
        float m_Banking;

        union {
            unsigned int m_Mask1;
            struct {
                unsigned int m_bShootIntoGround : 1;
                unsigned int m_bShootIntoGroundFailed : 1;
                unsigned int m_bShootIntoGroundInFullbody : 1;
                unsigned int m_bShootIntoDynamic : 1;
                unsigned int m_bShootIntoGroundAlways : 1;
                unsigned int m_bPathRequest : 1;
                unsigned int m_bPathNotifySyncToCycle : 1;
                unsigned int m_bPathFinderEnabled : 1;
                unsigned int m_bUseEndDir : 1;
                unsigned int m_bStayInsidePath : 1;
                unsigned int m_bMoveBackward : 1;
                unsigned int m_bDisableIdleAnimation : 1;
                unsigned int m_bWounded : 1;
                unsigned int m_bPositionLock : 1;
                unsigned int m_bDontStop : 1;
                unsigned int m_bEnableLookAt : 1;
                unsigned int m_bPathDone : 1;
                unsigned int m_bIgnoreReservations : 1;
                unsigned int m_bUpdatePosition : 1;
                unsigned int m_bIsSeeing : 1;
                unsigned int m_bIsVisible : 1;
                unsigned int m_bScriptUBAnimLoop : 1;
                unsigned int m_bScriptUBAnimMirrored : 1;
                unsigned int m_bPathNotifyMsgSent : 1;
                unsigned int m_bPathDoneMsgSent : 1;
                unsigned int m_bCancelPathWhenPossible : 1;
                unsigned int m_bPerformedFullUpdateLastFrame : 1;
                unsigned int m_bForceOnScreen : 1;
                unsigned int m_bIsScared : 1;
            };
        };

        // Смещение 17504 (0x0690) - m_Mask2 и Битовые поля
        union {
            unsigned int m_Mask2;
            struct {
                unsigned char m_bScriptPaused : 1;
                unsigned char m_bStopMovement : 1;
            };
        };

        uint32_t m_lShootIntoGroundFlag;
        Glacier::ZREF m_rActorList;
        Glacier::Animation::Header* m_pAnimUBHoldRifle;     
        float m_fRunDustDist;                 
        Glacier::Animation::Header* m_pScriptUBAnimToPlay;  
        float m_fScriptUBAnimToPlayPrct;
        int32_t m_nCurrentScriptUBAnimID;
        float m_fScriptUBAnimPitch;
    }; // Verified size 0x900
    RE_VERIFY_SIZE(ZActor, 0x900);
}