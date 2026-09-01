#include <Glacier/Animation/ActiveAnimation.h>
#include <Glacier/Animation/Header.h>
#include <Glacier/GameBase/Boid/ZHumanBoid.h>
#include <Glacier/GameBase/ZActor.h>
#include <Glacier/Locomotion/ZMoveSets.h>
#include <Glacier/Locomotion/ZSet.h>
#include <Glacier/Locomotion/ZTransition.h>
#include <Glacier/System/ZSysInterface.h>


namespace Glacier
{
    bool g_PerformFullUpdate = false;

    LocomotionInfo::LocomotionInfo()
        : fDeltaTime(0.0f),
          stateChangeCount(0),
          allowActiveAnims(true),
          allowActiveAnimsBase(true),
          beingPushed(false),
          currentSpeed(0.0f),
          currentDir{},
          currentAcc{},
          wantedSpeed(0.0f),
          wantedDir{},
          nextSpeed(0.0f),
          nextDir{},
          distanceToMotionChange(0.0f),
          directLineToWaypoint(false),
          stairsMode(0),
          actorPos{},
          actorMat{},
          actorDir{},
          dirToTarget{},
          locomotionSet(nullptr),
          m_SwitchedFromState(LOCOSTATE_DISABLED),
          secondsToTransfer(0.0f),
          animationSpeedMultiplier(1.0f),
          rotationModifier(0.0f),
          skipFramePctIncrease(false)
    {
        mreset(actorMat);
    }

    ZActor::ZActor(const char* psName, ZBaseGeom* pBaseGeom)
        : ZLNKWHANDS(psName, pBaseGeom)
    {
        // TODO: Finish me
    }

    ZActor::~ZActor()
    {
        // TODO: Finish me
    }

    void ZActor::LoadSave(ISerializerStream& stream, bool bSaving)
    {
        // TODO: Finish this place after ZActor will be reversed
        ZLNKWHANDS::LoadSave(stream, bSaving);
    }

    const RTP::ZPropertyInfo& ZActor::GetProperties() const
    {
        return ZActor::Info;
    }

    uint32_t ZActor::GetObjectId() const
    {
        return m_Id;
    }

    void ZActor::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = m_Id;
        mask = m_Mask;
    }

    ZGEOMCLASSINFO* ZActor::GetOldClassInfo() const
    {
        return m_OldClassInfo;
    }

    void ZActor::Hide(bool onOff)
    {
        ZLNKWHANDS::Hide(onOff);
    }

    void ZActor::ClassInit()
    {
        // TODO: Finish this place after ZActor will be reversed
        ZLNKWHANDS::ClassInit();
    }

    void ZActor::ClassInit2()
    {
        // TODO: Finish this place after ZActor will be reversed
        ZLNKWHANDS::ClassInit2();
    }

    void ZActor::ClassFrameUpdate()
    {
        // TODO: Finish this place after ZActor will be reversed
        ZLNKWHANDS::ClassFrameUpdate();
    }

    int32_t ZActor::ClassCommand(ZMSGID msg, void* pData)
    {
        // TODO: Finish this place after ZActor will be reversed
        return ZLNKWHANDS::ClassCommand(msg, pData);
    }

    void ZActor::OnCameraEnter()
    {
        // TODO: Finish this place after ZActor will be reversed
        ZLNKWHANDS::OnCameraEnter();
    }

    Animation::ActiveAnimation* ZActor::ActivateAnimSegment(Animation::Header* pHeader,
        int control,
        float startFrame,
        float endFrame,
        float speed)
    {
        // TODO: Finish this place after ZActor will be reversed
        return ZLNKWHANDS::ActivateAnimSegment(pHeader, control, startFrame, endFrame, speed);
    }

    void ZActor::MoveToMatPos(const float* pMat, const float* pPos)
    {
        // TODO: Finish this place after ZActor will be reversed
        ZLNKWHANDS::MoveToMatPos(pMat, pPos);
    }

    void ZActor::AnimEnd(Animation::ActiveAnimation* pAnim, int lControl)
    {
        // TODO: Finish this place after ZActor will be reversed
        ZLNKWHANDS::AnimEnd(pAnim, lControl);
    }

    void ZActor::UpdateFacing()
    {
        // TODO: Finish this place after ZActor will be reversed
        ZLNKWHANDS::UpdateFacing();
    }

    bool ZActor::IsDead() const
    {
        return m_ActorState == ACTORSTATE_DEAD;
    }

    void ZActor::SetActorRootTM(const float* pMat, const float* pPos)
    {
        // TODO: Finish this place after ZActor will be reversed
        SetRootTMParent(const_cast<float*>(pMat), const_cast<float*>(pPos));
    }

    void ZActor::SetActorPosDir(const float* pPos, const float* pDir)
    {
        ZVector3 direction(pDir);
        direction.x = -direction.x;
        direction.y = -direction.y;
        direction.z = -direction.z;
        ZMat3x3 matrix;
        createmat(matrix, direction, nullptr);
        SetActorRootTM(matrix, pPos);
    }

    void ZActor::GetActorRootTM(float* pMat, float* pPos)
    {
        // TODO: Finish this place after ZActor will be reversed
        mreset(pMat);
        vreset(pPos);
    }

    void ZActor::GetActorWorldPosition(float* pPos)
    {
        GetWorldPosition(*reinterpret_cast<ZVector3*>(pPos));
    }

    void ZActor::SetActorWorldPosition(const float* pPos)
    {
        SetWorldPosition(pPos);
    }

    bool ZActor::CanPlayAnimSegment(Animation::Header*, float, float, const float*, const float*, bool, float, float)
    {
        // TODO: Finish this place after ZActor will be reversed
        return true;
    }

    bool ZActor::IsUnconscious() const
    {
        return m_ActorState == ACTORSTATE_UNCONSCIOUS;
    }
    bool ZActor::IsAwake() const
    {
        return m_ActorState == ACTORSTATE_AWAKE;
    }
    bool ZActor::IsSleeping() const
    {
        return m_ActorState == ACTORSTATE_SLEEPING;
    }

    void ZActor::SetHero(uint32_t hero)
    {
        m_rHero = hero;
    }
    void ZActor::SetActorState(ACTORSTATE state)
    {
        m_ActorState = state;
    }

    void ZActor::OnViewEnter()
    {
        // TODO: Finish this place after ZActor will be reversed
    }

    void ZActor::OnViewLeave()
    {
        // TODO: Finish this place after ZActor will be reversed
    }

    void* ZActor::GetPathFinder4()
    {
        return m_pkBoid ? m_pkBoid->m_pkPathfinder : nullptr;
    }

    void ZActor::EnablePathFinder(bool enabled)
    {
        m_bPathFinderEnabled = enabled;
    }
    void ZActor::SetStopDistance(float distance)
    {
        m_fEndDist = distance;
    }
    void ZActor::SetEndDir(const Vector3* pDirection)
    {
        m_vEndDir = *pDirection;
        m_bUseEndDir = true;
    }
    const Vector3* ZActor::GetEndDir()
    {
        return &m_vEndDir;
    }

    void ZActor::MoveToPosition(ZVector3*, ZVector3*)
    {
        // TODO: Finish this place after ZActor will be reversed
    }

    void ZActor::SlideToPosition(ZVector3*, ZVector3*)
    {
        // TODO: Finish this place after ZActor will be reversed
    }

    void ZActor::SetPathNotify(float value)
    {
        m_fPathNotify = value;
    }
    void ZActor::SetPathNotifySyncToCycle(bool enabled)
    {
        m_bPathNotifySyncToCycle = enabled;
    }
    void ZActor::SetEndAction(int8_t action)
    {
        m_iEndAction = action;
    }

    void ZActor::RemovePath(uint8_t, PATH_CANCEL_REASONS reason, bool)
    {
        // TODO: Finish this place after ZActor will be reversed
        m_ePathCancelReason = reason;
    }

    void ZActor::SendPathDone(bool done)
    {
        m_bPathDone = done;
    }

    void ZActor::GetMoveFromPos(float* pPos)
    {
        pPos[0] = m_vInitialPos.x;
        pPos[1] = m_vInitialPos.y;
        pPos[2] = m_vInitialPos.z;
    }

    void* ZActor::FindNextPathBlocker()
    {
        // TODO: Finish this place after ZActor will be reversed
        return nullptr;
    }

    void ZActor::SetShootIntoGround(bool enabled)
    {
        m_bShootIntoGround = enabled;
    }
    void ZActor::SetShootIntoGroundInFullbody(bool enabled)
    {
        m_bShootIntoGroundInFullbody = enabled;
    }
    void ZActor::SetStayInsidePath(bool enabled)
    {
        m_bStayInsidePath = enabled;
    }

    void ZActor::ShootIntoGround(float*, bool, bool)
    {
        // TODO: Finish this place after ZActor will be reversed
    }

    void ZActor::ShootIntoGroundRegularly(float* pPos, bool a, bool b, float)
    {
        // TODO: Finish this place after ZActor will be reversed
        ShootIntoGround(pPos, a, b);
    }

    void ZActor::SetMoveSpeedMultiplier(float multiplier)
    {
        m_fMoveSpeedMultiplier = multiplier;
    }
    void ZActor::SetPathFinderEnabled(bool enabled)
    {
        m_bPathFinderEnabled = enabled;
    }

    void ZActor::OnPathRequest(uint32_t)
    {
        // TODO: Finish this place after ZActor will be reversed
    }

    void ZActor::OnPathFinished(uint32_t)
    {
        // TODO: Finish this place after ZActor will be reversed
    }

    void ZActor::OnPathCanceled(PATH_CANCEL_REASONS reason)
    {
        m_ePathCancelReason = reason;
    }

    void ZActor::OnPathCanceledLockedDoor(uint32_t)
    {
        // TODO: Finish this place after ZActor will be reversed
    }

    void ZActor::OnPathError(PATH_ERRORS)
    {
        // TODO: Finish this place after ZActor will be reversed
    }

    void ZActor::OnPathNotify()
    {
        // TODO: Finish this place after ZActor will be reversed
    }

    void ZActor::OnNewPath()
    {
        // TODO: Finish this place after ZActor will be reversed
    }

    void ZActor::OnSound(REFTAB*)
    {
        // TODO: Finish this place after ZActor will be reversed
    }

    void ZActor::Die()
    {
        SetActorState(ACTORSTATE_DEAD);
    }

    void ZActor::DieByForce(const float*, const float*, float, uint32_t)
    {
        // TODO: Finish this place after ZActor will be reversed
        Die();
    }

    void ZActor::Resurrect()
    {
        SetActorState(ACTORSTATE_AWAKE);
    }
    void ZActor::Knockout()
    {
        SetActorState(ACTORSTATE_UNCONSCIOUS);
    }
    void ZActor::Revive()
    {
        SetActorState(ACTORSTATE_AWAKE);
    }
    bool ZActor::GetKnockedOut() const
    {
        return IsUnconscious();
    }
    ZREF ZActor::GetActorList() const
    {
        return m_rActorList;
    }
    void ZActor::SetDisableIdleAnimation(bool disabled)
    {
        m_bDisableIdleAnimation = disabled;
    }
    REFTAB* ZActor::GetAudibleRoomList()
    {
        return m_pAudibleRoomList;
    }
    void ZActor::LookAt(uint32_t target)
    {
        m_rHeadTarget = target;
    }
    bool ZActor::IsSeeing() const
    {
        return m_bIsSeeing;
    }
    void ZActor::SetSeeing(bool seeing)
    {
        m_bIsSeeing = seeing;
    }
    bool ZActor::IsVisible() const
    {
        return m_bIsVisible;
    }
    void ZActor::SetVisible(bool visible)
    {
        m_bIsVisible = visible;
    }

    bool ZActor::WantToLookAt(ZGEOM*, uint8_t)
    {
        // TODO: Finish this place after ZActor will be reversed
        return false;
    }

    bool ZActor::VerifyPlayerVisible()
    {
        // TODO: Finish this place after ZActor will be reversed
        return m_bIsVisible;
    }

    void ZActor::DeterminePathLookAt(LocomotionInfo*, float*, float*, float*, float*)
    {
        // TODO: Finish this place after ZActor will be reversed
    }

    bool ZActor::CanLookAt(ZGEOM*) const
    {
        // TODO: Finish this place after ZActor will be reversed
        return false;
    }

    bool ZActor::IsOnStairs() const
    {
        // TODO: Finish this place after ZActor will be reversed
        return false;
    }

    void ZActor::UpdatePositionOffScreen()
    {
        // TODO: Finish this place after ZActor will be reversed
        LocoFromMotionOffScreen(-1.0f);
    }

    void ZActor::UpdatePosition()
    {
        // TODO: Finish this place after ZActor will be reversed
    }

    Animation::Header* ZActor::GetCurrentUBAnim(ZItem*, ZItem*, bool& mirrored)
    {
        // TODO: Finish this place after ZActor will be reversed
        mirrored = false;
        return nullptr;
    }

    void ZActor::SetHoldWeaponUBAnim(Animation::Header* pHeader)
    {
        m_pAnimUBHoldRifle = pHeader;
    }

    uint32_t ZActor::GetAnimOffset(const char*)
    {
        // TODO: Finish this place after ZActor will be reversed
        return 0;
    }

    uint32_t ZActor::GetAnimOffset(Animation::Header*)
    {
        // TODO: Finish this place after ZActor will be reversed
        return 0;
    }

    Animation::Header* ZActor::GetAnimHeader(uint32_t)
    {
        // TODO: Finish this place after ZActor will be reversed
        return nullptr;
    }

    void ZActor::ShootIntoGroundCallback(SExtendedImpactInfo*)
    {
        // TODO: Finish this place after ZActor will be reversed
    }

    void ZActor::ResetCurrentAnimation()
    {
        StopAllAnims(true);
        m_Locomotion.m_LocomotionSetEntry = -1;
        m_Locomotion.m_LocomotionSetTransition = -1;
        m_Locomotion.m_CurrentTransition = nullptr;
        m_Locomotion.m_CurrentAnim = nullptr;
        m_Locomotion.m_ActiveAnim = nullptr;
        m_Locomotion.m_CurrentTransitionSub = 0;
        m_Locomotion.m_LastFramePrc = -1.0f;
        m_Locomotion.m_ExpectedMoveStopDist = -1.0f;
        m_Locomotion.m_MoveCycleCorrectionSpeed = 0.0f;
        m_Locomotion.m_CurrentAnimMirrored = false;
        m_Locomotion.m_UseGroundAnimFully = true;
        m_Locomotion.m_AbortTransitionOnDistanceGrow = false;
        m_Locomotion.m_PerformQuickStart = false;
        m_Locomotion.m_CycleCorrectionAllowPct0 = true;
        m_Locomotion.m_CycleCorrectionAllowPct50 = true;
        uint8_t* pRuntimeFlags = reinterpret_cast<uint8_t*>(&m_Locomotion.m_RuntimeFlags);
        pRuntimeFlags[0] = (pRuntimeFlags[0] & 0xCC) | 0x02;
        pRuntimeFlags[1] |= 0x03;
        m_fCurrentBaseMoveFramePrc = 0.0f;
        m_nCurrentScriptUBAnimID = 0;
    }

    void ZActor::RestartLocomotionSystem()
    {
        StopAllAnims(true);
        m_Locomotion.m_CurrentAnim = nullptr;
        m_Locomotion.m_LocomotionSetEntry = 0;

        const float fSpeed = m_pkBoid->GetSpeed();
        const float fRemaining = m_pkBoid->TotalRemaining();
        if (fSpeed > 0.0f && fRemaining > 0.5f)
        {
            m_Locomotion.m_State = LOCOSTATE_MOVEFORWARD;
            m_Locomotion.m_LocomotionSetEntry = -1;
        }
        else
        {
            m_Locomotion.m_State = LOCOSTATE_STAND;
        }

        ResetCurrentAnimation();
        m_Locomotion.m_LastDistanceToMotionChange = 999999.0f;
        m_Locomotion.m_LastPreferredState = 0;
        m_pkBoid->SetSpeed(fSpeed);
    }

    void ZActor::SetCurrentAnimation(ZAnimVariationHandle handle, int flags, float random)
    {
        m_Locomotion.m_CurrentAnim = GetAnimHeaderFromVariation(handle, flags, random);
        if (m_Locomotion.m_CurrentAnim)
            m_Locomotion.m_BlendFrames = m_Locomotion.m_CurrentAnim->m_BlendFrames;
    }

    void ZActor::SetCurrentAnimation(ZAnimVariationHandle handle)
    {
        SetCurrentAnimation(handle, m_MoveSetBindings.m_VariationMask, m_MoveSetBindings.m_VariationSeed);
    }

    int8_t ZActor::GetBestFitAnimation(Locomotion::ZSet* pSet, float speed, int flags) const
    {
        int8_t bestEntry = -1;
        float bestScore = -100000000.0f;
        for (int i = 0; i < pSet->m_iEntries; ++i)
        {
            const Locomotion::ZEntry& entry = pSet->m_Entries[i];
            const float entrySpeed = m_MoveSetBindings.m_Speed[i];
            if (entrySpeed < 0.0f || !entry.m_AnimHandle.IsValid() || entry.m_Flags != flags)
                continue;

            float score = -std::fabs(entrySpeed - speed);
            if ((entrySpeed <= 1.0f) != (speed == 0.0f))
                score -= 10000.0f;
            if (score > bestScore)
            {
                bestScore = score;
                bestEntry = static_cast<int8_t>(i);
            }
        }
        return bestEntry;
    }

    int8_t ZActor::GetBestFitTransition(Locomotion::ZSet* pSet, int8_t from, int8_t to, bool allowMultiSub, bool allowGroundFully, float framePct, const float* pDirection, float distance) const
    {
        if (from < 0 || to < 0)
            return -1;

        ZVector2 direction = pDirection
            ? ZVector2{pDirection[0], pDirection[1]}
            : ZVector2{0.0f, 1.0f};
        const float directionLength = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (directionLength != 0.0f)
        {
            direction.x /= directionLength;
            direction.y /= directionLength;
        }

        const float angle = std::atan2(direction.x, direction.y);
        int8_t bestTransition = -1;
        float bestScore = -9999999.0f;
        for (int i = 0; i < pSet->m_iTransitions; ++i)
        {
            const Locomotion::ZTransition& transition = pSet->m_Transitions[i];
            if (transition.m_Transition[0] != from || transition.m_Transition[1] != to || transition.m_SubCount <= 0 || (!allowMultiSub && transition.m_SubCount != 1) || (!allowGroundFully && transition.m_UseGroundAnimFully))
            {
                continue;
            }

            float phaseDelta = framePct - m_MoveSetBindings.m_BlendOutToPhase[i];
            if (phaseDelta > 0.5f)
                phaseDelta -= 1.0f;
            else if (phaseDelta < -0.5f)
                phaseDelta += 1.0f;

            float score = -std::fabs(angle - m_MoveSetBindings.m_TransitionRotation[i]) * 3.0f - std::fabs(phaseDelta) * 1000.0f;
            if (distance >= 0.0f)
                score -= std::fabs(m_MoveSetBindings.m_Distance[i] - distance) * 0.001f;
            if (score > bestScore)
            {
                bestScore = score;
                bestTransition = static_cast<int8_t>(i);
            }
        }
        return bestTransition;
    }

    void ZActor::GoToState(LocomotionInfo* pInfo, ELocomotionState state, ELocomotionState nextState)
    {
        if (++pInfo->stateChangeCount <= 10)
        {
            pInfo->m_SwitchedFromState = m_Locomotion.m_State;
            m_Locomotion.m_State = state;
            m_Locomotion.m_NextState = nextState;
            HandleState(pInfo);
            return;
        }

        m_Locomotion.m_State = LOCOSTATE_DISABLED;
        m_Locomotion.m_CurrentAnim = nullptr;
        m_Locomotion.m_ActiveAnim = nullptr;
        m_Locomotion.m_LocomotionSetEntry = 0;
        m_Locomotion.m_LocomotionSetTransition = -1;
        m_Locomotion.m_CurrentTransition = nullptr;
        m_Locomotion.m_TurnToEndDir = false;
        m_fCurrentBaseMoveFramePrc = 0.0f;
    }

    void ZActor::HandleState(LocomotionInfo* pInfo)
    {
        switch (m_Locomotion.m_State)
        {
        case LOCOSTATE_DISABLED:
            GoToState(pInfo, LOCOSTATE_STAND, LOCOSTATE_DISABLED);
            break;

        case LOCOSTATE_STAND:
            if (pInfo->currentSpeed > 0.0f || pInfo->wantedSpeed > 0.0f)
            {
                const int8_t entry = GetBestFitAnimation(
                    pInfo->locomotionSet, pInfo->wantedSpeed, pInfo->stairsMode);
                if (entry >= 0)
                {
                    m_Locomotion.m_LocomotionSetEntry = entry;
                    SetCurrentAnimation(pInfo->locomotionSet->m_Entries[entry].m_AnimHandle);
                    GoToState(pInfo, LOCOSTATE_MOVEFORWARD, LOCOSTATE_DISABLED);
                }
            }
            else if (!m_Locomotion.m_CurrentAnim)
            {
                const int8_t entry = GetBestFitAnimation(pInfo->locomotionSet, 0.0f, 0);
                if (entry >= 0)
                {
                    m_Locomotion.m_LocomotionSetEntry = entry;
                    SetCurrentAnimation(pInfo->locomotionSet->m_Entries[entry].m_AnimHandle);
                }
            }
            break;

        case LOCOSTATE_MOVEFORWARD:
        {
            if (pInfo->currentSpeed <= 0.0f && pInfo->wantedSpeed <= 0.0f)
            {
                ResetCurrentAnimation();
                GoToState(pInfo, LOCOSTATE_STAND, LOCOSTATE_DISABLED);
                break;
            }

            const int8_t entry = GetBestFitAnimation(
                pInfo->locomotionSet, pInfo->wantedSpeed, pInfo->stairsMode);
            if (entry >= 0 && entry != m_Locomotion.m_LocomotionSetEntry)
            {
                m_Locomotion.m_LocomotionSetEntry = entry;
                SetCurrentAnimation(pInfo->locomotionSet->m_Entries[entry].m_AnimHandle);
            }
            break;
        }

        case LOCOSTATE_ACTIVEANIM:
        {
            if (!m_Locomotion.m_CurrentAnim)
            {
                const int transitionIndex = m_Locomotion.m_LocomotionSetTransition;
                if (transitionIndex < 0)
                {
                    const ELocomotionState nextState = m_Locomotion.m_NextState;
                    ResetCurrentAnimation();
                    GoToState(pInfo, nextState, LOCOSTATE_DISABLED);
                    break;
                }

                Locomotion::ZTransition& transition =
                    pInfo->locomotionSet->m_Transitions[transitionIndex];
                if (m_Locomotion.m_CurrentTransition != &transition)
                {
                    m_Locomotion.m_CurrentTransition = &transition;
                    m_Locomotion.m_CurrentTransitionSub = 0;
                }

                const int subIndex = m_Locomotion.m_CurrentTransitionSub;
                Locomotion::ZSubTransition* pSub = transition.m_Sub[subIndex];
                SetCurrentAnimation(pSub->m_AnimHandle);
                if (!m_Locomotion.m_CurrentAnim)
                {
                    const ELocomotionState nextState = m_Locomotion.m_NextState;
                    ResetCurrentAnimation();
                    GoToState(pInfo, nextState, LOCOSTATE_DISABLED);
                    break;
                }

                if (subIndex + 1 >= transition.m_SubCount)
                {
                    m_Locomotion.m_BlendOutAtFramePct =
                        m_MoveSetBindings.m_EndFramePct[transitionIndex];
                    m_Locomotion.m_BlendOutToFramePct =
                        m_MoveSetBindings.m_BlendOutToPhase[transitionIndex];
                }
                else
                {
                    m_Locomotion.m_BlendOutAtFramePct = pSub->m_EndFramePct;
                    m_Locomotion.m_BlendOutToFramePct =
                        transition.m_Sub[subIndex + 1]->m_StartFramePct;
                }

                float startFramePct = pSub->m_StartFramePct;
                const uint8_t runtimeFlags = static_cast<uint8_t>(m_Locomotion.m_RuntimeFlags);
                if (subIndex == 0 && (runtimeFlags & 0x20) != 0)
                    startFramePct = m_MoveSetBindings.m_QuickStartFramePct[transitionIndex];
                if (pInfo->secondsToTransfer > 0.0f)
                {
                    const float frameCount = static_cast<float>(m_Locomotion.m_CurrentAnim->m_Frames - 1);
                    if (frameCount > 0.0f)
                        startFramePct += pInfo->secondsToTransfer * 25.0f / frameCount;
                    startFramePct = std::min(startFramePct, pSub->m_EndFramePct);
                }
                if (pSub->m_BlendFrames >= 0.0f)
                    m_Locomotion.m_BlendFrames = pSub->m_BlendFrames;

                const int mode = pSub->m_Mirrored ? 0x8080 : 0x80;
                const float endFrame = static_cast<float>(m_Locomotion.m_CurrentAnim->m_Frames - 1);
                m_Locomotion.m_ActiveAnim = ActivateAnimSegment(m_Locomotion.m_CurrentAnim,
                    mode,
                    startFramePct * endFrame,
                    endFrame,
                    1.0f);
                m_Locomotion.m_CurrentAnimMirrored = pSub->m_Mirrored;

                const ZVector2& transitionDirection =
                    m_MoveSetBindings.m_TransitionEndDirection[transitionIndex];
                m_Locomotion.m_ActiveAnimPostTransitionDir = transitionDirection;
                if (m_pkBoid->GetMode() != eInActive)
                {
                    const float speed = std::max(
                        m_MoveSetBindings.m_Speed[transition.m_Transition[0]], 0.0f);
                    m_pkBoid->SetSpeed(speed);
                }
            }

            if (m_Locomotion.m_NextState == LOCOSTATE_DISABLED)
                m_Locomotion.m_NextState = LOCOSTATE_STAND;

            const bool animationRunning = m_Locomotion.m_CurrentAnim && m_Locomotion.m_ActiveAnim && m_Locomotion.m_CurrentTransition && m_fCurrentBaseMoveFramePrc < m_Locomotion.m_BlendOutAtFramePct && (m_Locomotion.m_ActiveAnim->mode & 7) != 0;
            const uint8_t runtimeFlags = static_cast<uint8_t>(m_Locomotion.m_RuntimeFlags);
            const bool directionValid = (runtimeFlags & 0x02) == 0 || m_Locomotion.m_ActiveAnimPostTransitionDir.x * pInfo->wantedDir.x + m_Locomotion.m_ActiveAnimPostTransitionDir.y * pInfo->wantedDir.y >= 0.69f;
            const bool distanceValid = (runtimeFlags & 0x10) == 0 || pInfo->distanceToMotionChange <= m_Locomotion.m_LastDistanceToMotionChange;
            if (animationRunning && directionValid && distanceValid)
                break;
            if (animationRunning)
            {
                RestartLocomotionSystem();
                pInfo->allowActiveAnims = pInfo->allowActiveAnimsBase;
                GoToState(pInfo, m_Locomotion.m_State, LOCOSTATE_DISABLED);
                break;
            }

            Locomotion::ZTransition* pTransition = m_Locomotion.m_CurrentTransition;
            if (pTransition && m_Locomotion.m_CurrentTransitionSub + 1 < pTransition->m_SubCount)
            {
                pInfo->secondsToTransfer = std::max(
                    m_fCurrentBaseMoveFramePrc - m_Locomotion.m_BlendOutAtFramePct, 0.0f);
                ++m_Locomotion.m_CurrentTransitionSub;
                m_Locomotion.m_CurrentAnim = nullptr;
                m_fCurrentBaseMoveFramePrc = 0.0f;
                GoToState(pInfo, LOCOSTATE_ACTIVEANIM, m_Locomotion.m_NextState);
                break;
            }

            const ELocomotionState nextState = m_Locomotion.m_NextState;
            if (pTransition)
            {
                m_pkBoid->SetSpeed(std::max(
                    m_MoveSetBindings.m_Speed[pTransition->m_Transition[1]], 0.0f));
            }
            pInfo->secondsToTransfer = std::max(
                m_fCurrentBaseMoveFramePrc - m_Locomotion.m_BlendOutAtFramePct, 0.0f);
            pInfo->skipFramePctIncrease = true;
            ResetCurrentAnimation();
            m_Locomotion.m_CurrentTransition = nullptr;
            GoToState(pInfo, nextState, LOCOSTATE_DISABLED);
            break;
        }

        case LOCOSTATE_MANUALANIM:
        {
            if (!m_Locomotion.m_CurrentAnim)
            {
                const int transitionIndex = m_Locomotion.m_LocomotionSetTransition;
                if (transitionIndex >= 0)
                {
                    Locomotion::ZTransition& transition =
                        pInfo->locomotionSet->m_Transitions[transitionIndex];
                    Locomotion::ZSubTransition* pSub =
                        transition.m_Sub[transition.m_SubCount - 1];
                    SetCurrentAnimation(pSub->m_AnimHandle);
                    m_Locomotion.m_CurrentAnimMirrored = pSub->m_Mirrored;
                    m_Locomotion.m_BlendOutAtFramePct =
                        m_MoveSetBindings.m_EndFramePct[transitionIndex];
                    m_Locomotion.m_BlendOutToFramePct =
                        m_MoveSetBindings.m_BlendOutToPhase[transitionIndex];
                    m_fCurrentBaseMoveFramePrc =
                        m_MoveSetBindings.m_QuickStartFramePct[transitionIndex];
                }
                m_Locomotion.m_LocomotionSetTransition = -1;
            }

            if (m_Locomotion.m_NextState == LOCOSTATE_DISABLED)
                m_Locomotion.m_NextState = LOCOSTATE_STAND;
            if (!m_Locomotion.m_CurrentAnim || m_fCurrentBaseMoveFramePrc >= m_Locomotion.m_BlendOutAtFramePct)
            {
                const ELocomotionState nextState = m_Locomotion.m_NextState;
                pInfo->secondsToTransfer = std::max(
                    m_fCurrentBaseMoveFramePrc - m_Locomotion.m_BlendOutAtFramePct, 0.0f);
                ResetCurrentAnimation();
                GoToState(pInfo, nextState, LOCOSTATE_DISABLED);
                break;
            }

            if ((static_cast<uint8_t>(m_Locomotion.m_RuntimeFlags) & 0x10) != 0 && pInfo->distanceToMotionChange > m_Locomotion.m_LastDistanceToMotionChange)
            {
                RestartLocomotionSystem();
                pInfo->allowActiveAnims = pInfo->allowActiveAnimsBase;
                GoToState(pInfo, m_Locomotion.m_State, LOCOSTATE_DISABLED);
            }
            break;
        }
        }
    }

    void ZActor::HandleStateAfterMovement(LocomotionInfo* pInfo)
    {
        pInfo->currentSpeed = m_pkBoid->m_ActualSpeed;
        if (m_Locomotion.m_State != LOCOSTATE_MOVEFORWARD)
            return;

        if (pInfo->currentSpeed <= 0.0f && pInfo->distanceToMotionChange == 0.0f)
        {
            if (m_Locomotion.m_CurrentAnim)
            {
                StopAnimSound((m_Locomotion.m_CurrentAnim->m_Mask & 8) == 0,
                    0,
                    false);
            }
            ResetCurrentAnimation();
            GoToState(pInfo, LOCOSTATE_STAND, LOCOSTATE_DISABLED);
            return;
        }

        if (m_Locomotion.m_LocomotionSetEntry >= 0)
        {
            const float animationSpeed =
                m_MoveSetBindings.m_Speed[m_Locomotion.m_LocomotionSetEntry];
            if (animationSpeed > 0.0f)
                pInfo->animationSpeedMultiplier = pInfo->currentSpeed / animationSpeed;
        }

        if (m_Locomotion.m_ExpectedMoveStopDist >= 0.0f && (pInfo->distanceToMotionChange > m_Locomotion.m_ShortestMoveStopDist || !pInfo->allowActiveAnims || pInfo->nextSpeed != 0.0f || !pInfo->directLineToWaypoint))
        {
            m_Locomotion.m_ExpectedMoveStopDist = -1.0f;
            m_Locomotion.m_MoveCycleCorrectionSpeed = 0.0f;
            m_Locomotion.m_CycleCorrectionAllowPct0 = true;
            m_Locomotion.m_CycleCorrectionAllowPct50 = true;
        }

        if (pInfo->allowActiveAnims && pInfo->nextSpeed == 0.0f && pInfo->directLineToWaypoint && m_Locomotion.m_ExpectedMoveStopDist < 0.0f)
        {
            const int8_t stopEntry = GetBestFitAnimation(pInfo->locomotionSet, 0.0f, 0);
            const int8_t transition = GetBestFitTransition(pInfo->locomotionSet,
                m_Locomotion.m_LocomotionSetEntry,
                stopEntry,
                true,
                true,
                m_fCurrentBaseMoveFramePrc,
                &pInfo->wantedDir.x,
                pInfo->distanceToMotionChange);
            if (transition >= 0)
            {
                m_Locomotion.m_ExpectedMoveStopDist =
                    m_MoveSetBindings.m_Distance[transition];
                m_Locomotion.m_MoveCycleCorrectionSpeed =
                    pInfo->locomotionSet->m_Entries[m_Locomotion.m_LocomotionSetEntry].m_CycleCorrectionSpeed;
            }
        }

        if (m_Locomotion.m_ExpectedMoveStopDist >= 0.0f && pInfo->distanceToMotionChange >= m_Locomotion.m_ExpectedMoveStopDist && m_Locomotion.m_CurrentAnim)
        {
            const float cycleDistance = std::sqrt(
                m_Locomotion.m_CurrentAnim->m_CycleDist[0] * m_Locomotion.m_CurrentAnim->m_CycleDist[0] + m_Locomotion.m_CurrentAnim->m_CycleDist[2] * m_Locomotion.m_CurrentAnim->m_CycleDist[2]);
            const float duration = m_Locomotion.m_CurrentAnim->m_Frames * Animation::Header::TIME_SCALE;
            if (cycleDistance > 0.0f && duration > 0.0f)
            {
                const float targetPhase = std::fmod(
                    (pInfo->distanceToMotionChange - m_Locomotion.m_ExpectedMoveStopDist) / cycleDistance,
                    1.0f);
                float phaseDelta = targetPhase - m_fCurrentBaseMoveFramePrc;
                if (phaseDelta > 0.5f)
                    phaseDelta -= 1.0f;
                else if (phaseDelta < -0.5f)
                    phaseDelta += 1.0f;

                const float maxCorrection = pInfo->fDeltaTime * std::max(m_Locomotion.m_MoveCycleCorrectionSpeed, 0.0f);
                phaseDelta = std::clamp(phaseDelta, -maxCorrection, maxCorrection);
                if (std::fabs(phaseDelta) > 0.0001f)
                {
                    m_fCurrentBaseMoveFramePrc = std::fmod(
                        m_fCurrentBaseMoveFramePrc + phaseDelta + 1.0f, 1.0f);
                }
            }
        }
    }

    void ZActor::LocoFromMotion(float* pActorPosition, float* pActorDirection, float* pMotionPosition, float* pMotionDirection, bool allowActiveAnims, float fDeltaTime)
    {
        if (fDeltaTime < 0.0f)
            fDeltaTime = g_pSysInterface->DeltaFrameTime;

        m_Locomotion.m_InControlThisFrame = true;
        if (m_Locomotion.m_State == LOCOSTATE_DISABLED)
            RestartLocomotionSystem();

        LocomotionInfo info;
        info.fDeltaTime = fDeltaTime;
        info.allowActiveAnims = allowActiveAnims;
        info.allowActiveAnimsBase = allowActiveAnims;
        info.beingPushed = (m_pkBoid->m_Mask & 4) != 0;
        GetActorRootTM(info.actorMat, info.actorPos);
        info.actorDir = {-info.actorMat.data[0], -info.actorMat.data[2]};

        if (m_pkBoid->m_Tracker.x != info.actorPos.x || m_pkBoid->m_Tracker.z != info.actorPos.z)
        {
            info.actorPos = m_pkBoid->m_Tracker;
            ShootIntoGroundRegularly(pMotionPosition, false, false, fDeltaTime);
            SetActorWorldPosition(info.actorPos);
        }

        if (m_Locomotion.m_ActiveAnim)
            info.allowActiveAnims = false;

        m_pkBoid->GetLocomotionInfo(info.currentSpeed, &info.currentDir.x, &info.wantedDir.x, info.distanceToMotionChange, info.nextSpeed, &info.nextDir.x);
        if (info.distanceToMotionChange < 0.0f)
            info.distanceToMotionChange = 0.0f;
        if (info.currentSpeed == 0.0f)
            info.currentDir = info.actorDir;
        if (info.wantedDir.x == 0.0f && info.wantedDir.y == 0.0f)
            info.wantedDir = info.actorDir;
        if (info.currentDir.x == 0.0f && info.currentDir.y == 0.0f)
            info.currentDir = info.wantedDir;

        if (m_Locomotion.m_MoveSetNr < 0)
        {
            if (!m_bPositionLock)
            {
                ZVector3 direction(info.currentDir.x, 0.0f, info.currentDir.y);
                vnorm(direction);
                direction.x = -direction.x;
                direction.z = -direction.z;
                ZMat3x3 matrix;
                const ZVector3 up(0.0f, 1.0f, 0.0f);
                createmat(matrix, direction, up);
                ShootIntoGroundRegularly(pMotionPosition, true, true, fDeltaTime);
                SetActorRootTM(matrix, pMotionPosition);
            }
            return;
        }

        info.locomotionSet = Locomotion::ZMoveSets::Get(m_Locomotion.m_MoveSetNr);
        info.directLineToWaypoint = m_pkBoid->m_Targets[0].m_bEndPoint && !m_pkBoid->IsFollowingSubTarget();
        m_pkBoid->TargetDirection(&info.dirToTarget.x);
        info.wantedSpeed = m_MoveSetBindings.m_PreferredSpeed * m_fMoveSpeedMultiplier;

        HandleState(&info);

        if (!m_Locomotion.m_ActiveAnim)
        {
            const float currentSpeed = m_pkBoid->m_fSpeed;
            float newSpeed = currentSpeed;
            float moveDistance = 0.0f;
            if (info.distanceToMotionChange == 0.0f || m_Locomotion.m_State == LOCOSTATE_STAND)
            {
                newSpeed = 0.0f;
            }
            else
            {
                const float speedScale = info.wantedSpeed / 127.0f;
                const float acceleration = std::max(
                                               info.locomotionSet->m_AccelerationFactor * (info.beingPushed ? 9999.0f : 400.0f),
                                               10.0f) *
                    speedScale;
                const float deceleration = std::max(
                                               info.locomotionSet->m_DecelerationFactor * (info.beingPushed ? 9999.0f : 200.0f),
                                               10.0f) *
                    speedScale;
                const float stoppingAcceleration = currentSpeed > 0.0f
                    ? -(currentSpeed * currentSpeed) / (2.0f * info.distanceToMotionChange)
                    : 0.0f;

                if (info.nextSpeed == 0.0f && stoppingAcceleration < -deceleration)
                {
                    newSpeed = std::max(currentSpeed + stoppingAcceleration * fDeltaTime, 0.0f);
                    moveDistance = (currentSpeed + newSpeed) * fDeltaTime * 0.5f;
                }
                else
                {
                    newSpeed = std::clamp(currentSpeed + acceleration * fDeltaTime,
                        0.0f,
                        info.wantedSpeed);
                    moveDistance = newSpeed * fDeltaTime;
                }
                moveDistance = std::clamp(moveDistance, 0.0f, info.distanceToMotionChange);
            }

            m_pkBoid->MoveTrackerAndSetSpeed(newSpeed, moveDistance, fDeltaTime);
            *reinterpret_cast<ZVector3*>(pMotionPosition) = m_pkBoid->m_Tracker;
            info.currentSpeed = m_pkBoid->m_ActualSpeed;

            ZVector2 facing(-info.actorMat.data[0], -info.actorMat.data[2]);
            const float facingLength = std::sqrt(facing.x * facing.x + facing.y * facing.y);
            if (facingLength > 0.0f)
            {
                facing.x /= facingLength;
                facing.y /= facingLength;
            }
            ZVector2 wanted = info.wantedDir;
            const float wantedLength = std::sqrt(wanted.x * wanted.x + wanted.y * wanted.y);
            if (wantedLength > 0.0f)
            {
                wanted.x /= wantedLength;
                wanted.y /= wantedLength;
            }
            const float cross = facing.x * wanted.y - facing.y * wanted.x;
            const float dot = std::clamp(facing.x * wanted.x + facing.y * wanted.y,
                -1.0f,
                1.0f);
            const float angle = std::atan2(cross, dot);
            const float turnSpeed = std::max(info.locomotionSet->m_fTurnSpeed * (info.wantedSpeed / 127.0f) * 0.6f * std::fabs(angle), 2.5f);
            const float rotation = std::clamp(angle,
                -turnSpeed * fDeltaTime,
                turnSpeed * fDeltaTime);

            ZMat3x3 rotationMatrix;
            mrotaxis(rotationMatrix, rotation * -57.295776f, 0.0f, 1.0f, 0.0f);
            mmmul(info.actorMat, rotationMatrix);
            ShootIntoGroundRegularly(pMotionPosition, true, true, fDeltaTime);
            SetActorRootTM(info.actorMat, pMotionPosition);
            info.rotationModifier = rotation;
            HandleStateAfterMovement(&info);
        }
        else
        {
            m_Locomotion.m_ActiveAnim->SetCurrentFrame(m_fCurrentBaseMoveFramePrc);
            m_pkBoid->MoveTracker(-1.0f, 0.0f, 0.0f, fDeltaTime);
            *reinterpret_cast<ZVector3*>(pMotionPosition) = m_pkBoid->m_Tracker;
            ShootIntoGroundRegularly(pMotionPosition, true, true, fDeltaTime);
            SetActorRootTM(info.actorMat, pMotionPosition);
            HandleStateAfterMovement(&info);
        }

        if (m_Locomotion.m_CurrentAnim && !info.skipFramePctIncrease)
        {
            m_fCurrentBaseMoveDuration = m_Locomotion.m_CurrentAnim->m_Frames * Animation::Header::TIME_SCALE;
            if (m_fCurrentBaseMoveDuration > 0.0f)
            {
                m_fCurrentBaseMoveFramePrc += fDeltaTime * info.animationSpeedMultiplier / m_fCurrentBaseMoveDuration;
            }
        }
        if (m_fCurrentBaseMoveFramePrc != 1.0f)
            m_fCurrentBaseMoveFramePrc = std::fmod(m_fCurrentBaseMoveFramePrc, 1.0f);

        (void)pActorPosition;
        (void)pActorDirection;
        (void)pMotionDirection;

        m_pkBoid->SetMaxSpeed(info.wantedSpeed);
        m_Locomotion.m_LastDistanceToMotionChange = info.distanceToMotionChange;
        m_Locomotion.m_LastPreferredState = m_Locomotion.m_PreferredState;
    }

    void ZActor::LocoFromMotionOffScreen(float fDeltaTime)
    {
        if (fDeltaTime < 0.0f)
            fDeltaTime = g_pSysInterface->DeltaFrameTime;

        m_Locomotion.m_InControlThisFrame = true;
        m_Locomotion.m_State = LOCOSTATE_DISABLED;
        const float wantedSpeed = m_MoveSetBindings.m_MoveSpeed * m_fMoveSpeedMultiplier;
        m_pkBoid->MoveTracker(wantedSpeed, 0.0f, 0.0f, fDeltaTime);

        const ZVector3 trackerPosition = m_pkBoid->m_Tracker;
        ZVector2 trackerDirection;
        m_pkBoid->TrackerDirection(&trackerDirection.x);
        if (trackerDirection.x == 0.0f && trackerDirection.y == 0.0f)
        {
            SetActorWorldPosition(trackerPosition);
        }
        else
        {
            trackerDirection.x = -trackerDirection.x;
            trackerDirection.y = -trackerDirection.y;
            const float length = std::sqrt(trackerDirection.x * trackerDirection.x + trackerDirection.y * trackerDirection.y);
            trackerDirection.x /= length;
            trackerDirection.y /= length;

            const ZVector3 direction(trackerDirection.x, 0.0f, trackerDirection.y);
            ZMat3x3 matrix;
            createmat(matrix, direction, nullptr);
            SetActorRootTM(matrix, trackerPosition);
        }

        m_pkBoid->SetMaxSpeed(wantedSpeed);
        m_pkBoid->SetSpeed(wantedSpeed);

        if (m_rHero)
        {
            ZGEOM* pHero = ZGEOM::RefToPtr(m_rHero);
            ZVector3 heroPosition;
            pHero->GetWorldPosition(heroPosition);
            if (vdist(heroPosition, trackerPosition) < 2000.0f)
            {
                if (m_pkBoid->m_ActualSpeed <= 0.1f)
                {
                    if (m_Locomotion.m_CurrentAnim)
                    {
                        StopAnimSound((m_Locomotion.m_CurrentAnim->m_Mask & 8) == 0,
                            0,
                            false);
                    }
                    m_Locomotion.m_CurrentAnim = nullptr;
                }
                else
                {
                    if (!m_Locomotion.m_CurrentAnim && m_Locomotion.m_MoveSetNr >= 0)
                    {
                        Locomotion::ZSet* pSet = Locomotion::ZMoveSets::Get(
                            m_Locomotion.m_MoveSetNr);
                        m_Locomotion.m_LocomotionSetEntry = GetBestFitAnimation(
                            pSet, wantedSpeed, 0);
                        if (m_Locomotion.m_LocomotionSetEntry >= 0)
                        {
                            SetCurrentAnimation(pSet->m_Entries[m_Locomotion.m_LocomotionSetEntry].m_AnimHandle);
                        }
                    }

                    if (m_Locomotion.m_CurrentAnim && (m_Locomotion.m_LastFramePrc == -1.0f || m_Locomotion.m_LastFramePrc > m_fCurrentBaseMoveFramePrc))
                    {
                        // TODO: Finish this place after ZLNKOBJ will be reversed
                        // StartAnimSound(m_Locomotion.m_CurrentAnim->GetSoundIndex(),
                        //     (m_Locomotion.m_CurrentAnim->m_Mask & 8) == 0, nullptr,
                        //     m_fCurrentBaseMoveFramePrc, false, 0);
                    }
                    m_Locomotion.m_LastFramePrc = m_fCurrentBaseMoveFramePrc;
                }

                if (!m_Locomotion.m_ActiveAnim)
                {
                    if (m_fCurrentBaseMoveFramePrc != 1.0f)
                        m_fCurrentBaseMoveFramePrc = std::fmod(m_fCurrentBaseMoveFramePrc, 1.0f);
                    if (m_Locomotion.m_CurrentAnim)
                    {
                        m_fCurrentBaseMoveDuration = m_Locomotion.m_CurrentAnim->m_Frames * Animation::Header::TIME_SCALE;
                        if (m_fCurrentBaseMoveDuration != 0.0f)
                            m_fCurrentBaseMoveFramePrc += fDeltaTime / m_fCurrentBaseMoveDuration;
                    }
                }
            }
        }

        m_Locomotion.m_LastDistanceToMotionChange = 999999.0f;
        m_Locomotion.m_LastPreferredState = m_Locomotion.m_PreferredState;
    }

    ZActor::ACTORSTATE ZActor::GetActorState() const
    {
        return m_ActorState;
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZActor,
        ZLNKWHANDS,
        0x0097BE40,
        "ZActor",
        0x0077156C,
        nullptr, // TODO: Finish me
        0x0080D1B0,
        0x0097BD60,
        0x0097BD64);
}
