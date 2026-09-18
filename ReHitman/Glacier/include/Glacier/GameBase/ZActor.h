#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZMessageResolver.h>
#include <Glacier/IK/ZLNKWHANDS.h>
#include <Glacier/PF4/ZMetaNode.h>
#include <Glacier/PF4/ZPath.h>
#include <Glacier/Runtime/Macro.h>
#include <Glacier/ZSTL/ZMath.h>



namespace Glacier
{
    extern bool g_PerformFullUpdate;

    namespace Locomotion
    {
        struct ZSet;
        struct ZTransition;
    }

    enum ELocomotionState
    {
        LOCOSTATE_DISABLED = 0,
        LOCOSTATE_STAND = 1,
        LOCOSTATE_ACTIVEANIM = 2,
        LOCOSTATE_MANUALANIM = 3,
        LOCOSTATE_MOVEFORWARD = 4
    };

    struct ZLocomotionState
    {
        ZVector2 m_ActiveAnimPostTransitionDir;
        Locomotion::ZTransition* m_CurrentTransition;
        Animation::Header* m_CurrentAnim;
        Animation::ActiveAnimation* m_ActiveAnim;
        PF4::ZMetaNode* m_LookAtTarget;
        float m_LastDistanceToMotionChange;
        float m_BlendOutAtFramePct;
        float m_BlendOutToFramePct;
        float m_BlendFrames;
        float m_LastFramePrc;
        float m_ExpectedMoveStopDist;
        float m_ShortestMoveStopDist;
        float m_MoveCycleCorrectionSpeed;
        int8_t m_MoveSetNr;
        int8_t m_PreferredState;
        int8_t m_LastPreferredState;
        int8_t m_LookMode;
        int8_t m_LocomotionSetEntry;
        int8_t m_LocomotionSetTransition;
        int8_t m_CurrentTransitionSub;

        struct
        {
            ELocomotionState m_State : 4;
            ELocomotionState m_NextState : 4;
            uint32_t m_CurrentAnimMirrored : 1;
            uint32_t m_UseGroundAnimFully : 1;
            uint32_t m_InControlThisFrame : 1;
            uint32_t m_OffScreenLastFrame : 1;
            uint32_t m_AbortTransitionOnDistanceGrow : 1;
            uint32_t m_PerformQuickStart : 1;
            uint32_t m_LookAtEnabled : 1;
            uint32_t m_TurnToEndDir : 1;
            uint32_t m_CycleCorrectionAllowPct0 : 1;
            uint32_t m_CycleCorrectionAllowPct50 : 1;
        };

        uint32_t m_RuntimeFlags;
    };
    RE_VERIFY_SIZE(ZLocomotionState, 0x48);

    struct ProgramQueEntry
    {
        uint8_t m_StartIndex;
        uint8_t m_EndIndex;
        int8_t m_Program;
        int8_t m_PathAction;
    };
    RE_VERIFY_SIZE(ProgramQueEntry, 0x4);

    struct ZSetBindings
    {
        float m_VariationSeed;
        int32_t m_VariationMask;
        float m_MoveSpeed;
        float m_PreferredSpeed;
        float m_MaxSpeed;
        float m_Speed[10];
        float m_PhaseShift[10];
        float m_LongestStoppingDistance;
        float m_Distance[20];
        float m_TransitionRotation[20];
        ZVector2 m_TransitionEndDirection[20];
        float m_BlendInFromPhase[20];
        float m_BlendOutToPhase[20];
        float m_QuickStartFramePct[20];
        float m_EndFramePct[20];
    };
    RE_VERIFY_SIZE(ZSetBindings, 0x2E8);

    struct LocomotionInfo
    {
        LocomotionInfo();

        float fDeltaTime;
        int stateChangeCount;
        bool allowActiveAnims;
        bool allowActiveAnimsBase;
        bool beingPushed;
        float currentSpeed;
        ZVector2 currentDir;
        ZVector2 currentAcc;
        float wantedSpeed;
        ZVector2 wantedDir;
        float nextSpeed;
        ZVector2 nextDir;
        float distanceToMotionChange;
        bool directLineToWaypoint;
        int stairsMode;
        ZVector3 actorPos;
        ZMat3x3 actorMat;
        ZVector2 actorDir;
        ZVector2 dirToTarget;
        Locomotion::ZSet* locomotionSet;
        ELocomotionState m_SwitchedFromState;
        float secondsToTransfer;
        float animationSpeedMultiplier;
        float rotationModifier;
        bool skipFramePctIncrease;
    };
    RE_VERIFY_SIZE(LocomotionInfo, 0x9C);

    class ZActor : public ZLNKWHANDS
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZActor, 0x2007D0u);

        // Static
        STATIC_CLASS_VAR(ZActor, ZMessageResolver, m_msgPathResponse);
        STATIC_CLASS_VAR(ZActor, ZMessageResolver, m_msgCanPenetrate);
        STATIC_CLASS_VAR(ZActor, ZMessageResolver, m_msgPathRequest);
        STATIC_CLASS_VAR(ZActor, ZMessageResolver, m_msgPathFinished);
        STATIC_CLASS_VAR(ZActor, ZMessageResolver, m_msgPathCanceled);
        STATIC_CLASS_VAR(ZActor, ZMessageResolver, m_msgPathCanceledLockedDoor);
        STATIC_CLASS_VAR(ZActor, ZMessageResolver, m_msgPathError);
        STATIC_CLASS_VAR(ZActor, ZMessageResolver, m_msgPathOpenDoor);
        STATIC_CLASS_VAR(ZActor, ZMessageResolver, m_msgEnterCamera);
        STATIC_CLASS_VAR(ZActor, ZMessageResolver, m_msgPathNotify);
        STATIC_CLASS_VAR(ZActor, ZMessageResolver, m_msgEnterView);
        STATIC_CLASS_VAR(ZActor, ZMessageResolver, m_msgLeaveView);

        // types
        enum ACTORSTATE : int8_t
        {
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

        static constexpr ZMSGID kAlarmMsg = 0x81A;

        // vtbl
        ~ZActor() override;
        // ZSerializable
        void LoadSave(ISerializerStream& stream, bool bSaving) override;
        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;
        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void Hide(bool OnOff) override;
        void ClassInit() override;
        void ClassInit2() override;
        void ClassFrameUpdate() override;
        int32_t ClassCommand(ZMSGID Msg, void* pData) override;
        void OnCameraEnter() override;

        // ZLNKOBJ
        Animation::ActiveAnimation* ActivateAnimSegment(Animation::Header* pHeader, int control, float startFrame, float endFrame, float speed) override;
        void MoveToMatPos(float const* pMat, float const* pPos) override;
        void AnimEnd(Animation::ActiveAnimation* pAnim, int lControl) override;

        // ZIKLNKOBJ
        void UpdateFacing() override;

        // ZCTRLIKLNKOBJ
        bool IsDead() const override;

        // ZActor
        virtual void SetActorRootTM(float const*, float const*);
        virtual void SetActorPosDir(float const*, float const*);
        virtual void GetActorRootTM(float*, float*);
        virtual void GetActorWorldPosition(float*);
        virtual void SetActorWorldPosition(float const*);
        virtual bool CanPlayAnimSegment(Animation::Header*, float, float, float const*, float const*, bool, float, float);
        virtual bool IsUnconscious() const;
        virtual bool IsAwake() const;
        virtual bool IsSleeping() const;
        virtual void SetHero(uint32_t);
        virtual void SetActorState(ZActor::ACTORSTATE);
        virtual void OnViewEnter();
        virtual void OnViewLeave();
        virtual void* GetPathFinder4();
        virtual void EnablePathFinder(bool);
        virtual void SetStopDistance(float);
        virtual void SetEndDir(const Vector3*);
        virtual const Vector3* GetEndDir();
        virtual void MoveToPosition(ZVector3*, ZVector3*);
        virtual void SlideToPosition(ZVector3*, ZVector3*);
        virtual void SetPathNotify(float);
        virtual void SetPathNotifySyncToCycle(bool);
        virtual void SetEndAction(int8_t);
        virtual void RemovePath(uint8_t, ZActor::PATH_CANCEL_REASONS, bool);
        virtual void SendPathDone(bool);
        virtual void GetMoveFromPos(float*);
        virtual void* FindNextPathBlocker();
        virtual void SetShootIntoGround(bool);
        virtual void SetShootIntoGroundInFullbody(bool);
        virtual void SetStayInsidePath(bool);
        virtual void ShootIntoGround(float*, bool, bool);
        virtual void ShootIntoGroundRegularly(float*, bool, bool, float);
        virtual void SetMoveSpeedMultiplier(float);
        virtual void SetPathFinderEnabled(bool);
        virtual void OnPathRequest(uint32_t);
        virtual void OnPathFinished(uint32_t);
        virtual void OnPathCanceled(ZActor::PATH_CANCEL_REASONS);
        virtual void OnPathCanceledLockedDoor(uint32_t);
        virtual void OnPathError(ZActor::PATH_ERRORS);
        virtual void OnPathNotify(void);
        virtual void OnNewPath(void);
        virtual void OnSound(REFTAB*);
        virtual void Die();
        virtual void DieByForce(float const*, float const*, float, uint32_t);
        virtual void Resurrect();
        virtual void Knockout();
        virtual void Revive();
        virtual bool GetKnockedOut() const;
        virtual ZREF GetActorList() const;
        virtual void SetDisableIdleAnimation(bool);
        virtual REFTAB* GetAudibleRoomList();
        virtual void LookAt(uint32_t);
        virtual bool IsSeeing() const;
        virtual void SetSeeing(bool);
        virtual bool IsVisible() const;
        virtual void SetVisible(bool);
        virtual bool WantToLookAt(ZGEOM*, uint8_t);
        virtual bool VerifyPlayerVisible();
        virtual void DeterminePathLookAt(LocomotionInfo*, float*, float*, float*, float*);
        virtual bool CanLookAt(ZGEOM*) const;
        virtual bool IsOnStairs() const;
        virtual void UpdatePositionOffScreen();
        virtual void UpdatePosition();
        virtual Animation::Header* GetCurrentUBAnim(ZItem*, ZItem*, bool&);
        virtual void SetHoldWeaponUBAnim(Animation::Header*);
        virtual uint32_t GetAnimOffset(const char*);
        virtual uint32_t GetAnimOffset(Animation::Header*);
        virtual Animation::Header* GetAnimHeader(uint32_t);
        virtual void ShootIntoGroundCallback(SExtendedImpactInfo*);

        // methods
        ZActor(const char* psName, ZBaseGeom* pBaseGeom);
        void ResetCurrentAnimation();
        void RestartLocomotionSystem();
        void SetCurrentAnimation(ZAnimVariationHandle handle, int flags, float random);
        void SetCurrentAnimation(ZAnimVariationHandle handle);
        int8_t GetBestFitAnimation(Locomotion::ZSet* pSet, float speed, int flags) const;
        int8_t GetBestFitTransition(Locomotion::ZSet* pSet, int8_t from, int8_t to, bool allowMultiSub, bool allowGroundFully, float framePct, const float* pDirection, float distance) const;
        void LocoFromMotion(float* pActorPosition, float* pActorDirection, float* pMotionPosition, float* pMotionDirection, bool allowActiveAnims, float fDeltaTime);
        void LocoFromMotionOffScreen(float fDeltaTime);
        void HandleState(LocomotionInfo* pInfo);
        void HandleStateAfterMovement(LocomotionInfo* pInfo);
        void GoToState(LocomotionInfo* pInfo, ELocomotionState state, ELocomotionState nextState);
        ACTORSTATE GetActorState() const;

        // members
        Vector3 m_ContactNormal;
        Vector3 m_LastShootIntoGroundPosition;
        float m_LastShootIntoGroundHeight;
        float m_InterpolatedHeightError;
        ZREF m_rHero;
        ZHumanBoid* m_pkBoid;
        PF4::ZMetaNode m_Entity;
        PF4::ZMetaNode m_Reservation;
        ZLocomotionState m_Locomotion;
        uint16_t m_iQuePos;
        uint16_t m_iProgramQueSize;
        ProgramQueEntry m_ProgramQue[40];
        PF4::ZPath m_Path;
        int8_t m_Action[2];
        int8_t m_iEndAction;
        RE_ADD_PADDING(1);
        int16_t m_iNextPathTarget;
        RE_ADD_PADDING(2);
        REFTAB* m_pAudibleRoomList;
        float m_fOldFootStepFrame;
        uint32_t m_iPathfinderDoorKeyMask;
        ZVector3 m_vInitialPos;
        ZVector3 m_vInitialDir;
        uint32_t m_lInsideView;
        ZREF m_rHeadTarget;
        float m_fMoveSpeedMultiplier;
        float m_FrameTimeAccumulator;
        float m_FrameTimeAccumulatorBound;
        ZVector3 m_vEndDir;
        ZVector3 m_vEndPos;
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

        union
        {
            uint32_t m_Mask1;
            struct
            {
                uint32_t m_bShootIntoGround : 1;
                uint32_t m_bShootIntoGroundFailed : 1;
                uint32_t m_bShootIntoGroundInFullbody : 1;
                uint32_t m_bShootIntoDynamic : 1;
                uint32_t m_bShootIntoGroundAlways : 1;
                uint32_t m_bPathRequest : 1;
                uint32_t m_bPathNotifySyncToCycle : 1;
                uint32_t m_bPathFinderEnabled : 1;
                uint32_t m_bUseEndDir : 1;
                uint32_t m_bStayInsidePath : 1;
                uint32_t m_bMoveBackward : 1;
                uint32_t m_bDisableIdleAnimation : 1;
                uint32_t m_bWounded : 1;
                uint32_t m_bPositionLock : 1;
                uint32_t m_bDontStop : 1;
                uint32_t m_bEnableLookAt : 1;
                uint32_t m_bPathDone : 1;
                uint32_t m_bIgnoreReservations : 1;
                uint32_t m_bUpdatePosition : 1;
                uint32_t m_bIsSeeing : 1;
                uint32_t m_bIsVisible : 1;
                uint32_t m_bScriptUBAnimLoop : 1;
                uint32_t m_bScriptUBAnimMirrored : 1;
                uint32_t m_bPathNotifyMsgSent : 1;
                uint32_t m_bPathDoneMsgSent : 1;
                uint32_t m_bCancelPathWhenPossible : 1;
                uint32_t m_bPerformedFullUpdateLastFrame : 1;
                uint32_t m_bForceOnScreen : 1;
                uint32_t m_bIsScared : 1;
            };
        };

        // Mask2
        union
        {
            unsigned int m_Mask2;
            struct
            {
                unsigned char m_bScriptPaused : 1;
                unsigned char m_bStopMovement : 1;
            };
        };

        uint32_t m_lShootIntoGroundFlag;
        ZREF m_rActorList;
        Animation::Header* m_pAnimUBHoldRifle;
        float m_fRunDustDist;
        Animation::Header* m_pScriptUBAnimToPlay;
        float m_fScriptUBAnimToPlayPrct;
        int32_t m_nCurrentScriptUBAnimID;
        float m_fScriptUBAnimPitch;
    };
    RE_VERIFY_SIZE(ZActor, 0x900); // PC verified
    RE_VERIFY_OFFSET(ZActor, m_Locomotion, 0x448);
    RE_VERIFY_OFFSET(ZActor, m_iQuePos, 0x490);
    RE_VERIFY_OFFSET(ZActor, m_MoveSetBindings, 0x5E4);
    RE_VERIFY_OFFSET(ZActor, m_fCurrentBaseMoveFramePrc, 0x8CC);
    RE_VERIFY_OFFSET(ZActor, m_Banking, 0x8D4);
}
