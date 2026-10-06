#include <Glacier/Animation/ActiveAnimation.h>
#include <Glacier/Animation/Header.h>
#include <Glacier/Animation/Model.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Data/ZGameData.h>
#include <Glacier/GameBase/Boid/EBoidState.h>
#include <Glacier/GameBase/Boid/ZHumanBoid.h>
#include <Glacier/GameBase/ZCheckVisible.h>
#include <Glacier/GameBase/ZActor.h>
#include <Glacier/Geom/ZLIST.h>
#include <Glacier/IK/ZBoneModifyBase.h>
#include <Glacier/Locomotion/Locomotion.h>
#include <Glacier/Locomotion/ZFollowPath.h>
#include <Glacier/Locomotion/ZMoveSets.h>
#include <Glacier/Locomotion/ZSet.h>
#include <Glacier/Locomotion/ZState.h>
#include <Glacier/Locomotion/ZStates.h>
#include <Glacier/Locomotion/ZTransition.h>
#include <Glacier/PF4/ZInterface.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZGeomBuffer.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/IK/ZCTRLIKLNKOBJ.h>
#include <Glacier/Items/ZItemTemplateWeapon.h>
#include <Glacier/Items/ZItemWeapon.h>
#include <Glacier/Physics/SExtendedImpactInfo.h>
#include <Glacier/Physics/ZCollisionBase.h>
#include <Glacier/Physics/eGlobalTreeType.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Animation/Manager.h>
#include <Glacier/Com/CGlobalCom.h>
#include <Glacier/GameBase/Boid/ZBoidSystem.h>
#include <Glacier/GameBase/ZPlayer.h>
#include <Glacier/IK/ZIKLNKOBJ.h>
#include <Glacier/Materials/BS_Runtime.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/ZEntityTracker.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZUniAssert.h>
#include <cmath>


namespace Glacier
{
    bool g_PerformFullUpdate = false;
    bool g_BankingEnabled = false;
    float timeSliceSpread = 0.0f;

    // PC 0x0097BCE8: per-level debug count of registered actors, reset on every
    // ZActor construction (PC 0x507780). Only consulted by the debug path PC 0x502DF0.
    static uint32_t g_lRegisteredActorsDebugCount = 0;

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
        Initialize(psName, pBaseGeom);
    }

    // ZActor::Initialize (PC 0x507780) -- the real body of the constructor.
    void ZActor::Initialize(const char* psName, ZBaseGeom* pBaseGeom)
    {
        (void)psName;
        (void)pBaseGeom;

        uint8_t* pMask1 = reinterpret_cast<uint8_t*>(&m_Mask1);
        uint8_t* pRuntimeFlags = reinterpret_cast<uint8_t*>(&m_Locomotion.m_RuntimeFlags);

        m_Locomotion.m_LastFramePrc = -1.0f;
        m_Locomotion.m_ExpectedMoveStopDist = -1.0f;
        pRuntimeFlags[1] |= 0x03;

        pMask1[0] = (pMask1[0] & 0x42) | 0x81;
        m_Mask2 &= 0xFFFFFFFD;

        const uint8_t mask1Byte2 = pMask1[2];
        m_MoveSetBindings.m_MoveSpeed = 170.0f;
        m_MoveSetBindings.m_PreferredSpeed = 170.0f;
        m_MoveSetBindings.m_MaxSpeed = 170.0f;

        pMask1[1] &= 0x80;

        m_Locomotion.m_CurrentAnim = nullptr;
        m_Locomotion.m_ActiveAnim = nullptr;
        m_Locomotion.m_ActiveAnimPostTransitionDir = ZVector2{ 0.0f, 0.0f };
        m_Locomotion.m_MoveSetNr = -1;
        m_Locomotion.m_LocomotionSetTransition = -1;
        m_Locomotion.m_LookMode = 0;
        m_Locomotion.m_LookAtTarget = nullptr;
        m_Locomotion.m_PreferredState = 0;
        m_Locomotion.m_BlendOutAtFramePct = 0.0f;
        m_Locomotion.m_BlendOutToFramePct = 0.0f;
        m_Locomotion.m_ShortestMoveStopDist = 0.0f;
        m_Locomotion.m_MoveCycleCorrectionSpeed = 0.0f;
        m_Locomotion.m_LastDistanceToMotionChange = 999999.0f;
        m_Locomotion.m_LastPreferredState = 0;

        pRuntimeFlags[0] = 10;
        m_Locomotion.m_State = LOCOSTATE_DISABLED;

        m_Action[0] = -1;
        m_Action[1] = -1;
        m_rHeadTarget = 0;
        m_lInsideView = 0;
        m_iEndAction = -1;
        m_fEndDist = 0.0f;
        m_fPathNotify = 0.0f;
        m_fPathNotifyNext = 0.0f;
        m_lPathDoneFlag = PATH_RESERVED;
        m_fMoveSpeedMultiplier = 1.0f;
        m_lShootIntoGroundFlag = 32;
        m_fCurrentBaseMoveFramePrc = 0.0f;
        m_fCurrentBaseMoveDuration = 0.0f;
        m_ActorState = ACTORSTATE_AWAKE;
        m_fOldFootStepFrame = 0.0f;
        m_iPathfinderDoorKeyMask = 0xFFFFFFFF;
        m_pAudibleRoomList = nullptr;
        m_fRunDustDist = 0.0f;

        pMask1[3] = (pMask1[3] & 0xEC) | 0x01;
        m_ePathCancelReason = eNONE;
        pMask1[2] = (mask1Byte2 & 0x61) | 0x9C;

        // PC registers every actor in the game-data actor pool before creating its boid.
        if (g_pGameData)
            g_pGameData->m_ActorsPool.push_back(this);

        // PC 0x507780: reset the per-construction debug actor-registration counter
        // (dword_97BCE8).
        g_lRegisteredActorsDebugCount = 0;

        // PC feeds a fresh id from the boid system into the boid and registers it there.
        ZBoidSystem* pBoidSystem = g_pGameData ? g_pGameData->m_pkBoidSystem : nullptr;
        const int lBoidId = pBoidSystem ? pBoidSystem->GetNextBoidID() : 0;
        m_pkBoid = ZUniMemory::New<ZHumanBoid>(
            lBoidId, static_cast<PF4::ZInterface*>(GetPathFinder4()), &m_Entity, this);
        if (m_pkBoid)
            m_pkBoid->m_eState = eInActive;
        if (pBoidSystem)
            pBoidSystem->AddBoid(m_pkBoid);

        m_pScriptUBAnimToPlay = nullptr;
        m_nCurrentScriptUBAnimID = 0;
        m_fScriptUBAnimToPlayPrct = 0.0f;
        m_fScriptUBAnimPitch = 1.0f;
        pMask1[2] &= 0x9F;

        m_ContactNormal = Vector3{};
        m_LastShootIntoGroundPosition = Vector3{};
        m_LastShootIntoGroundHeight = 0.0f;
        m_InterpolatedHeightError = 0.0f;
        m_FrameTimeAccumulator = 0.0f;

        pMask1[3] &= 0xF3;
        pMask1[1] |= 0x80;
        m_FrameTimeAccumulatorBound = -1.0f;
        m_Mask2 &= 0xFFFFFFFE;
    }

    ZActor::~ZActor()
    {
        ZEntityTracker* pEntityTracker = g_pEngineData->m_pEntityTracker;
        if (pEntityTracker)
            pEntityTracker->m_PathFinder->RemoveNode(&m_Entity);
    }

    void ZActor::LoadSave(ISerializerStream& stream, bool bSaving)
    {
        ZLNKWHANDS::LoadSave(stream, bSaving);

        float mat[9];
        float pos[3];
        GetActorRootTM(mat, pos);

        if (!bSaving)
            g_pEngineData->m_pPathfinder4Data->RemoveNode(&m_Reservation);

        stream.Exchange("m_Mask1", m_Mask1);
        stream.Exchange("m_Mask2", m_Mask2);

        if (m_pkBoid)
            m_pkBoid->LoadSave(stream, bSaving);

        bool bReservationRegistered = m_Reservation.m_Location.m_Component != -1;
        stream.Exchange("Reservation_Registered", bReservationRegistered);
        if (bReservationRegistered)
        {
            PF4::ZLocation location;
            if (bSaving)
            {
                location = m_Reservation.m_Location;
                location.LoadSave(stream, bSaving);
            }
            else
            {
                location.m_Component = -1;
                location.m_Graph = -1;
                location.m_Inside = 0;
                location.LoadSave(stream, false);
                m_Reservation.m_Type = g_pEngineData->m_pEntityTracker->m_ReservedPointId;
                g_pEngineData->m_pPathfinder4Data->AddNode(&m_Reservation, location);
            }
        }

        {
            PF4::ZLocation location;
            if (bSaving)
            {
                location = m_Entity.m_Location;
                location.LoadSave(stream, bSaving);
            }
            else
            {
                location.m_Component = -1;
                location.m_Graph = -1;
                location.m_Inside = 0;
                location.LoadSave(stream, false);
                g_pEngineData->m_pPathfinder4Data->RemoveNode(&m_Entity);
                g_pEngineData->m_pPathfinder4Data->AddNode(&m_Entity, location);
            }
        }

        stream.Exchange("m_ActorState", *reinterpret_cast<uint8_t*>(&m_ActorState));
        stream.Exchange("m_iPathfinderDoorKeyMask", m_iPathfinderDoorKeyMask);
        stream.ExchangeArray("m_ContactNormal", &m_ContactNormal.x, 3);
        stream.Exchange("m_lShootIntoGroundFlag", m_lShootIntoGroundFlag);

        if (!bSaving)
        {
            m_Path.Clear();
            m_Path.m_PathFinder = g_pEngineData->m_pPathfinder4Data;
        }

        bool bPathValid = m_Path.m_Size > 0;
        stream.Exchange("pathIsValid", bPathValid);
        if (bPathValid)
        {
            m_Path.LoadSave(stream, bSaving);
            stream.Exchange("m_iEndAction", reinterpret_cast<char&>(m_iEndAction));
            if (!bSaving)
                PreparePath();

            stream.Exchange("m_iQuePos", reinterpret_cast<int16_t&>(m_iQuePos));
            ZASSERT(static_cast<int>(m_iQuePos) < static_cast<int>(m_iProgramQueSize));
            stream.Exchange("m_iNextPathTarget", m_iNextPathTarget);

            bool bPathNotifyMsgSent = m_bPathNotifyMsgSent;
            stream.Exchange("m_bPathNotifyMsgSent", bPathNotifyMsgSent);
            m_bPathNotifyMsgSent = bPathNotifyMsgSent;

            bool bPathDoneMsgSent = m_bPathDoneMsgSent;
            stream.Exchange("m_bPathDoneMsgSent", bPathDoneMsgSent);
            m_bPathDoneMsgSent = bPathDoneMsgSent;

            bool bCancelPathWhenPossible = m_bCancelPathWhenPossible;
            stream.Exchange("m_bCancelPathWhenPossible", bCancelPathWhenPossible);
            m_bCancelPathWhenPossible = bCancelPathWhenPossible;

            if (!bSaving)
            {
                const int quePos = static_cast<int16_t>(m_iQuePos);
                m_Action[0] = m_ProgramQue[quePos].m_Program;
                m_Action[1] = (quePos + 1 >= static_cast<int16_t>(m_iProgramQueSize))
                    ? -1
                    : m_ProgramQue[quePos + 1].m_Program;

                if (m_Action[0] != -1)
                    Locomotion::GetProgram(m_Action[0])->TakeControl(this, true);
                if (m_Action[0] == 0)
                    Locomotion::ZStates::m_FollowPath.SetBoidTargets(this, quePos);
            }
        }
        else
        {
            stream.Exchange("m_Action0", reinterpret_cast<char&>(m_Action[0]));
            stream.Exchange("m_Action1", reinterpret_cast<char&>(m_Action[1]));
            stream.Exchange("m_iQuePos", reinterpret_cast<int16_t&>(m_iQuePos));
            stream.Exchange("m_iProgramQueSize", reinterpret_cast<int16_t&>(m_iProgramQueSize));
            for (int i = 0; i < static_cast<int16_t>(m_iProgramQueSize); ++i)
            {
                stream.Exchange("m_StartIndex", m_ProgramQue[i].m_StartIndex);
                stream.Exchange("m_EndIndex", m_ProgramQue[i].m_EndIndex);
                stream.Exchange("m_Program", reinterpret_cast<char&>(m_ProgramQue[i].m_Program));
                stream.Exchange("m_PathAction", reinterpret_cast<char&>(m_ProgramQue[i].m_PathAction));
            }
        }

        stream.ExchangeArray("m_vEndDir", &m_vEndDir.x, 3);
        stream.ExchangeArray("m_vEndPos", &m_vEndPos.x, 3);
        stream.Exchange("m_fEndDist", m_fEndDist);
        stream.Exchange("m_fPathNotify", m_fPathNotify);
        stream.Exchange("m_fPathNotifyNext", m_fPathNotifyNext);
        stream.Exchange("m_lPathDoneFlag", *reinterpret_cast<uint8_t*>(&m_lPathDoneFlag));
        stream.Exchange("m_ePathCancelReason", *reinterpret_cast<uint8_t*>(&m_ePathCancelReason));
        stream.Exchange("m_fMoveSpeedMultiplier", m_fMoveSpeedMultiplier);

        if (m_pkBoid)
            m_pkBoid->LoadSaveSubTarget(stream, bSaving);

        stream.Exchange("m_MoveSetNr", reinterpret_cast<char&>(m_Locomotion.m_MoveSetNr));
        stream.Exchange("m_PreferredState", reinterpret_cast<char&>(m_Locomotion.m_PreferredState));
        stream.Exchange("m_LastPreferredState", reinterpret_cast<char&>(m_Locomotion.m_LastPreferredState));
        stream.Exchange("m_VariationSeed", m_MoveSetBindings.m_VariationSeed);
        stream.Exchange("m_VariationMask", m_MoveSetBindings.m_VariationMask);

        if (!bSaving)
        {
            const int8_t moveSetNr = m_Locomotion.m_MoveSetNr;
            if (moveSetNr != -1)
            {
                const int8_t preferredState = m_Locomotion.m_PreferredState;
                m_Locomotion.m_MoveSetNr = -1;
                m_Locomotion.m_PreferredState = 0;
                // PC 0x508560 calls ZHM3Actor::SetMoveSet(moveSetNr, preferredState, 1).
                // That body only touches ZActor fields, so it is ported as SyncMoveSet.
                SyncMoveSet(moveSetNr, preferredState, true);
            }
        }

        LoadSaveAnimations(stream, bSaving);

        int32_t scriptUBAnimIndex = (bSaving && m_pScriptUBAnimToPlay) ? m_pScriptUBAnimToPlay->GetRef() : -1;
        stream.Exchange("m_pScriptUBAnimToPlay", scriptUBAnimIndex);
        if (!bSaving)
            m_pScriptUBAnimToPlay = Animation::Header::RefToPtr(scriptUBAnimIndex);

        stream.Exchange("m_fScriptUBAnimToPlayPrct", m_fScriptUBAnimToPlayPrct);
        stream.Exchange("m_nCurrentScriptUBAnimID", m_nCurrentScriptUBAnimID);
        stream.Exchange("m_fScriptUBAnimPitch", m_fScriptUBAnimPitch);

        int32_t holdRifleIndex = (bSaving && m_pAnimUBHoldRifle) ? m_pAnimUBHoldRifle->GetRef() : -1;
        stream.Exchange("m_pAnimUBHoldRifle", holdRifleIndex);
        if (!bSaving)
            m_pAnimUBHoldRifle = Animation::Header::RefToPtr(holdRifleIndex);

        stream.Exchange("m_rHeadTarget", m_rHeadTarget);

        if (!bSaving)
        {
            // Sync the locomotion program queue with the restored state (PC 0x505860).
            UpdateProgramQueue();
            SetActorRootTM(mat, pos);
        }

        m_baseGeom->SetControl(0x3000000u, 0u);
    }

    // ZHM3Actor::SetMoveSet (PC 0x505F00), ported as a ZActor-only non-virtual helper. It
    // re-resolves the locomotion set's per-entry/transition bindings after a move-set
    // change. Only Glacier fields are touched (m_Locomotion.m_MoveSetNr/.m_PreferredState,
    // m_MoveSetBindings.*), so it can live on ZActor. The two dependencies are:
    //   sub_5184A0(handle, mask, seed)      == ZLNKOBJ::GetAnimHeaderFromVariation
    //   sub_503950(pSet, from, to, ...)     == ZActor::GetBestFitTransition
    void ZActor::SyncMoveSet(int8_t moveSetNr, int8_t preferredState, bool bForce)
    {
        ZASSERT(moveSetNr >= 0 && moveSetNr < Locomotion::ZMoveSets::m_iSize);

        Locomotion::ZSet* pSet = Locomotion::ZMoveSets::Get(moveSetNr);
        ZASSERT(pSet != nullptr);
        if (!pSet)
            return;

        const bool bChanged = m_Locomotion.m_MoveSetNr != moveSetNr;

        // Resolve the preferred state, falling back to the set's default entry.
        int state = preferredState;
        if (state < 0 || state >= pSet->m_iEntries)
            state = pSet->m_DefaultState;
        else if (!bChanged && m_MoveSetBindings.m_Speed[state] <= 0.0f)
            state = pSet->m_DefaultState;
        m_Locomotion.m_PreferredState = static_cast<int8_t>(state);

        // A move-set change (unless forced) re-rolls the animation variation seed.
        if (bChanged && !bForce)
            m_MoveSetBindings.m_VariationSeed = g_pSysInterface->FRand(nullptr, 0);

        // Preferred-entry walk speed -> m_MoveSpeed / m_PreferredSpeed (floored at 5 u/s).
        float fPreferredSpeed = 170.0f;
        if (Animation::Header* pPreferred = GetAnimHeaderFromVariation(
                pSet->m_Entries[state].m_AnimHandle,
                m_MoveSetBindings.m_VariationMask, m_MoveSetBindings.m_VariationSeed))
        {
            const float fDuration = static_cast<float>(pPreferred->m_Frames) * Animation::Header::TIME_SCALE;
            if (fDuration != 0.0f)
            {
                fPreferredSpeed = std::sqrt(
                    pPreferred->m_CycleDist[0] * pPreferred->m_CycleDist[0] +
                    pPreferred->m_CycleDist[2] * pPreferred->m_CycleDist[2]) / fDuration;
            }
            if (fPreferredSpeed < 5.0f)
                fPreferredSpeed = 5.0f;
        }
        m_MoveSetBindings.m_MoveSpeed = fPreferredSpeed;
        m_MoveSetBindings.m_PreferredSpeed = fPreferredSpeed;

        // Per-entry bindings (m_Speed / m_PhaseShift / m_MaxSpeed).
        for (int i = 0; i < pSet->m_iEntries; ++i)
        {
            m_MoveSetBindings.m_Speed[i] = -1.0f;
            m_MoveSetBindings.m_PhaseShift[i] = 0.0f;

            Animation::Header* pAnim = GetAnimHeaderFromVariation(
                pSet->m_Entries[i].m_AnimHandle,
                m_MoveSetBindings.m_VariationMask, m_MoveSetBindings.m_VariationSeed);
            if (!pAnim)
                continue;

            const float fDuration = static_cast<float>(pAnim->m_Frames) * Animation::Header::TIME_SCALE;
            float fSpeed = 0.0f;
            if (fDuration != 0.0f)
            {
                fSpeed = std::sqrt(
                    pAnim->m_CycleDist[0] * pAnim->m_CycleDist[0] +
                    pAnim->m_CycleDist[2] * pAnim->m_CycleDist[2]) / fDuration;
            }
            m_MoveSetBindings.m_Speed[i] = fSpeed;
            if (fSpeed > m_MoveSetBindings.m_MaxSpeed)
                m_MoveSetBindings.m_MaxSpeed = fSpeed;

            // Meta key 2001 marks the phase at which the locomotion cycle starts.
            if (Animation::instance)
            {
                const int lMetaCount = static_cast<int>(
                    Animation::instance->GetMetaKeyDataLength(pAnim->m_MetaDataOffset));
                const Animation::ZMetaKey* pKeys = lMetaCount > 0
                    ? Animation::instance->GetMetaKeyData(pAnim->m_MetaDataOffset)
                    : nullptr;
                for (int k = 0; k < lMetaCount; ++k)
                {
                    if (pKeys[k].lValue == 2001)
                    {
                        m_MoveSetBindings.m_PhaseShift[i] =
                            static_cast<float>(pKeys[k].lFrame) / static_cast<float>(pAnim->m_Frames - 1);
                        break;
                    }
                }
            }
        }

        // Per-transition bindings. The original ground-samples these through
        // Animation::Header::GetGround; here the authored transition data is carried over.
        m_MoveSetBindings.m_LongestStoppingDistance = 0.0f;
        for (int i = 0; i < pSet->m_iTransitions; ++i)
        {
            const Locomotion::ZTransition& transition = pSet->m_Transitions[i];

            m_MoveSetBindings.m_Distance[i] = 0.0f;
            m_MoveSetBindings.m_TransitionRotation[i] =
                std::atan2(transition.m_Direction.x, transition.m_Direction.y);
            m_MoveSetBindings.m_TransitionEndDirection[i] = transition.m_Direction;
            m_MoveSetBindings.m_BlendInFromPhase[i] = transition.m_BlendInFromPhase;
            m_MoveSetBindings.m_BlendOutToPhase[i] = transition.m_BlendOutToPhase;
            m_MoveSetBindings.m_QuickStartFramePct[i] = transition.m_QuickStartFramePct;
            m_MoveSetBindings.m_EndFramePct[i] = transition.m_Sub[0]
                ? transition.m_Sub[0]->m_EndFramePct : 0.0f;
        }

        m_Locomotion.m_MoveSetNr = moveSetNr;
        m_Locomotion.m_LocomotionSetEntry = -1;
        m_Locomotion.m_LocomotionSetTransition = -1;
        m_Locomotion.m_ShortestMoveStopDist = 0.0f;

        RestartLocomotionSystem();
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
        if (onOff)
        {
            ZGEOM::Hide(true);
        }
        else
        {
            ZGEOM::Hide(false);
            if (m_lInsideView)
                OnCameraEnter();
        }
    }

    void ZActor::ClassInit()
    {
        ZLNKWHANDS::ClassInit();

        uint8_t* pMask1 = reinterpret_cast<uint8_t*>(&m_Mask1);
        pMask1[2] &= ~0x01;
        pMask1[1] &= 0xDF;
        m_Banking = 0.0f;
        m_fMoveSpeedMultiplier = 1.0f;
        CameraMessages(true);
        m_lInsideView = 0;

        ZMat3x3 mat;
        GetRootTM(mat, m_vInitialPos);
        m_vInitialDir.x = -mat.data[0];
        m_vInitialDir.y = -mat.data[1];
        m_vInitialDir.z = -mat.data[2];

        const ZAnimVariationHandle animHandle = GetAnimVariationHandle("/Weapon/UB_HoldRifle");
        m_pAnimUBHoldRifle = GetAnimHeaderFromVariation(animHandle, 0, 0.0f);

        SetSoundActive(true);

        m_Entity.m_Data = this;
        m_Entity.m_pBoid = m_pkBoid;

        if (g_pEngineData->m_pEntityTracker)
            g_pEngineData->m_pEntityTracker->AddActor(&m_Entity, m_vInitialPos);

        if (m_pkBoid)
        {
            m_pkBoid->TeleportPosition(m_vInitialPos);

            Vector3 direction{};
            m_pkBoid->SetTarget(m_vInitialPos, direction, 0.0f, true);
        }

        m_Reservation.m_Type = g_pEngineData->m_pEntityTracker->m_ReservedPointId;
        g_pEngineData->m_pPathfinder4Data->AddNode(&m_Reservation, m_Entity.m_Location);
    }

    void ZActor::ClassInit2()
    {
        ZLNKWHANDS::ClassInit2();

        if (g_pEngineData->m_pPathfinder4Data)
            EnablePathFinder(true);

        ZASSERT(ZLIST::m_TrackLinkObjectsInstance != nullptr);
        m_rActorList = ZLIST::m_TrackLinkObjectsInstance->GetRef();

        // PC 0x502570: in multiplayer no hero is bound; otherwise the first player slot
        // (g_pGameData->m_apPlayerIndex[0]) becomes the actor's hero.
        bool bMultiplayer = false;
        if (CGlobalCom* pCom = ZEngineDataBase::GetGlobalCom())
            pCom->GetVal("Multiplayer", &bMultiplayer);

        if (bMultiplayer)
        {
            SetHero(0);
        }
        else
        {
            ZPlayer* pHero = g_pGameData ? g_pGameData->GetPlayer(0) : nullptr;
            SetHero(pHero ? pHero->GetRef() : 0);
        }
    }

    // ZActor::ClassFrameUpdate (PC 0x504CB0).
    void ZActor::ClassFrameUpdate()
    {
        if (m_bForceOnScreen)
        {
            if (m_pBoneModify && !m_pBoneModify->IsRagdollActive())
                m_Mask1 &= 0xF7FFFFFFu; // clear m_bForceOnScreen

            if (m_bForceOnScreen)
                m_baseGeom->SetControl(4096u, 0u);
        }

        // Full-update scheduling: actors perform a full update only when visible, while
        // ragdolling, or once the accumulated frame time passes a per-actor staggered bound.
        const bool bVisible = (m_baseGeom->m_lControl & 0x1000) != 0;
        const bool bFirstUpdate = m_FrameTimeAccumulatorBound == -1.0f;
        g_PerformFullUpdate = false;
        if (bFirstUpdate)
        {
            m_FrameTimeAccumulatorBound = g_pSysInterface->DeltaFrameTime + 0.5f - timeSliceSpread;
            m_FrameTimeAccumulatorBound = g_pSysInterface->FRand(nullptr, 0) * 0.05f - 0.025f
                + m_FrameTimeAccumulatorBound;
            timeSliceSpread += 0.05f;
            if (timeSliceSpread >= 0.5f)
                timeSliceSpread = 0.0f;
            m_FrameTimeAccumulator = 0.0f;
        }
        else if (m_bPerformedFullUpdateLastFrame)
        {
            float fBound = m_FrameTimeAccumulatorBound - m_FrameTimeAccumulator;
            if (fBound <= 0.0f)
                fBound += 0.5f;
            m_FrameTimeAccumulatorBound = fBound;
            m_FrameTimeAccumulator = 0.0f;
        }

        m_bPerformedFullUpdateLastFrame = false;
        m_FrameTimeAccumulator += g_pSysInterface->DeltaFrameTime;
        if (bVisible
            || bFirstUpdate
            || (m_pBoneModify && m_pBoneModify->IsRagdollActive())
            || m_FrameTimeAccumulator >= m_FrameTimeAccumulatorBound)
        {
            m_bPerformedFullUpdateLastFrame = true;
            g_PerformFullUpdate = true;
        }

        ZLNKWHANDS::UpdateVisibilityPosition();

        uint8_t* pRuntimeFlags = reinterpret_cast<uint8_t*>(&m_Locomotion.m_RuntimeFlags);

        if (m_bCutSequence || m_ActorState != ACTORSTATE_AWAKE)
        {
            DisableIK();

            if (m_ActorState == ACTORSTATE_AWAKE)
            {
                ZIKLNKOBJ::ClassFrameUpdate();
            }
            else
            {
                UpdateAnimationsAndGroundLink(-1.0f);
                UpdatePosition();
            }

            pRuntimeFlags[0] ^= (pRuntimeFlags[0] ^ (8 * !bVisible)) & 8; // store off-screen flag
            return;
        }

        if (g_PerformFullUpdate)
        {
            UpdateAnimationsAndGroundLink(m_FrameTimeAccumulator);
        }
        else
        {
            if (m_Model)
                m_Model->PostAnim(0.0f);

            m_Ground._Pos.Reset();
            mreset(m_Ground._Mat);
        }

        if (bVisible)
            UpdatePosition();
        else
            UpdatePositionOffScreen();

        pRuntimeFlags[0] ^= (pRuntimeFlags[0] ^ (8 * !bVisible)) & 8; // store off-screen flag

        UpdateCurrentLnkAction();
        UpdateTargets();

        if (m_rHeadTarget && m_bEnableLookAt)
        {
            ZGEOM* pHeadTarget = ZGEOM::RefToPtr(m_rHeadTarget);
            if (pHeadTarget)
            {
                ZVector3 vHeadPos{};
                if (pHeadTarget->IsDerivedFrom<ZIKLNKOBJ>())
                {
                    ZIKLNKOBJ* pHeadIK = static_cast<ZIKLNKOBJ*>(pHeadTarget);
                    if ((pHeadIK->m_baseGeom->m_lControl & 0x1000) != 0)
                    {
                        ZMat3x3 mBone;
                        pHeadIK->GetIKBoneMatPos(pHeadIK->HeadBoneIndex(), mBone, vHeadPos);
                    }
                    else
                    {
                        vHeadPos.y = 170.0f;
                    }
                }

                pHeadTarget->GetRootPoint(vHeadPos);
                GetLocalPoint(vHeadPos);
                SetHeadTarget(vHeadPos, 1.0f);
            }
        }

        if (!CurrentLnkActionId())
        {
            if (m_pScriptUBAnimToPlay)
            {
                unsigned int lControl = 0;
                if (m_bScriptUBAnimMirrored)
                    lControl = 0x8000;

                if (m_fScriptUBAnimToPlayPrct == 0.0f)
                {
                    const unsigned int lSoundFlags =
                        (~(static_cast<unsigned int>(m_pScriptUBAnimToPlay->m_Mask) >> 3)) & 0xFFFFFF01u;
                    StartAnimSound(m_pScriptUBAnimToPlay->m_SoundIndex,
                        (lSoundFlags & 1) != 0, nullptr, 0.0f, false, 0);
                }

                const float fFrameCount = static_cast<float>(m_pScriptUBAnimToPlay->m_Frames) * Animation::Header::TIME_SCALE;
                if (fFrameCount != 0.0f)
                    m_fScriptUBAnimToPlayPrct += g_pSysInterface->DeltaFrameTime / fFrameCount;

                if (m_fScriptUBAnimToPlayPrct > 1.0f)
                {
                    if (m_bScriptUBAnimLoop)
                    {
                        m_fScriptUBAnimToPlayPrct = std::fmod(m_fScriptUBAnimToPlayPrct, 1.0f);
                        if (m_fScriptUBAnimToPlayPrct != 0.0f)
                        {
                            const unsigned int lSoundFlags =
                                (~(static_cast<unsigned int>(m_pScriptUBAnimToPlay->m_Mask) >> 3)) & 0xFFFFFF01u;
                            const float fFrame = static_cast<float>(m_pScriptUBAnimToPlay->m_Frames)
                                * m_fScriptUBAnimToPlayPrct;
                            StartAnimSound(m_pScriptUBAnimToPlay->m_SoundIndex,
                                (lSoundFlags & 1) != 0, nullptr, fFrame, false, 0);
                        }
                    }
                    else
                    {
                        const unsigned int lSoundFlags =
                            (~(static_cast<unsigned int>(m_pScriptUBAnimToPlay->m_Mask) >> 3)) & 0xFFFFFF01u;
                        m_fScriptUBAnimToPlayPrct = 0.0f;
                        StopAnimSound((lSoundFlags & 1) != 0, 0, false);
                        SendCommand(static_cast<ZMSGID>(2117), &m_nCurrentScriptUBAnimID, nullptr);
                        m_pScriptUBAnimToPlay = nullptr;
                        m_nCurrentScriptUBAnimID = 0;
                    }
                }

                if (m_pScriptUBAnimToPlay)
                {
                    SetBoneFrameBlend(m_pScriptUBAnimToPlay, m_fScriptUBAnimToPlayPrct, 1.0f, false, lControl);
                    m_Mask1 &= 0xFFFFFBFFu; // clear bit 10
                    return;
                }
            }
            else
            {
                bool bMirrored = false;
                ZItem* pLHandItem = GetLHandItem();
                ZItem* pRHandItem = GetRHandItem();
                Animation::Header* pUBAnim = GetCurrentUBAnim(pRHandItem, pLHandItem, bMirrored);
                if (pUBAnim)
                {
                    const float fFrame = std::fmod(m_fCurrentBaseMoveFramePrc, 1.0f);
                    SetBoneFrameBlend(pUBAnim, fFrame, 1.0f, false, 0);
                }
            }
        }

        m_Mask1 &= 0xFFFFFBFFu; // clear bit 10
    }

    int32_t ZActor::ClassCommand(ZMSGID msg, void* pData)
    {
        if (m_msgEnterCamera == msg)
        {
            OnCameraEnter();
        }
        else if (m_msgEnterView == msg)
        {
            OnViewEnter();
            return 0;
        }
        else if (m_msgLeaveView == msg)
        {
            OnViewLeave();
            return 0;
        }
        else if (m_msgPathResponse == msg)
        {
            ZASSERT(false);
            return 0;
        }
        else if (m_msgCanPenetrate == msg)
        {
            const uint32_t rGeom = *reinterpret_cast<const uint32_t*>(static_cast<const uint8_t*>(pData) + 4);
            if (rGeom)
                reinterpret_cast<uint8_t*>(pData)[8] =
                    *reinterpret_cast<const uint8_t*>(rGeom + 68) & 1;
        }

        if (!m_bCutSequence)
            ZLNKWHANDS::ClassCommand(msg, pData);

        return 0;
    }

    void ZActor::OnCameraEnter()
    {
        UpdateGeometry(true);

        Animation::ActiveAnimation* pGroundAnimation = GetGroundAnimation();
        if (!pGroundAnimation || (pGroundAnimation->mode & 0x600) == 0)
        {
            if (m_ActorState != ACTORSTATE_DEAD && m_ActorState != ACTORSTATE_UNCONSCIOUS)
            {
                float vPosition[3];
                GetActorWorldPosition(vPosition);

                ShootIntoGround(vPosition, true, true);

                if (m_pBoneModify && (m_pBoneModify->IsRagdollActive() || m_pBoneModify->m_bPassive))
                    vPosition[1] += 20.0f;

                SetActorWorldPosition(vPosition);
            }
        }
    }

    Animation::ActiveAnimation* ZActor::ActivateAnimSegment(Animation::Header* pHeader,
        int control,
        float startFrame,
        float endFrame,
        float speed)
    {
        if (!pHeader)
            return nullptr;

        if (m_Path.m_Size > 0 && (pHeader->m_OldControl & 0x40) != 0)
        {
            if (m_Action[0] == 4)
                return nullptr;

            if ((control & 4) != 0 || vlen2(pHeader->m_CycleDist) > 0.00012207031f)
                RemovePath(2, eANIMATION, false);
        }

        return ZLNKOBJ::ActivateAnimSegment(pHeader, control, startFrame, endFrame, speed);
    }

    void ZActor::MoveToMatPos(const float* pMat, const float* pPos)
    {
        SetActorRootTM(pMat, pPos);

        if (m_pkBoid)
        {
            const ZVector3 vPos(pPos);
            m_pkBoid->TeleportPosition(vPos);
            m_pkBoid->m_Tracker = vPos;
        }

        if (!m_bCutSequence && m_Path.m_Size > 0)
            RemovePath(2, eMOVED_TO_POS, false);
    }

    void ZActor::AnimEnd(Animation::ActiveAnimation* pAnim, int lControl)
    {
        ZIKLNKOBJ::AnimEnd(pAnim, lControl);

        if (m_Action[0] != -1)
            Locomotion::GetProgram(m_Action[0])->AnimEnd(this, pAnim);
    }

    void ZActor::UpdateFacing()
    {
        if (m_Path.m_Size <= 0)
            ZIKLNKOBJ::UpdateFacing();
    }

    bool ZActor::IsDead() const
    {
        return m_ActorState == ACTORSTATE_DEAD;
    }

    void ZActor::SetActorRootTM(const float* pMat, const float* pPos)
    {
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
        m_baseGeom->ParentGroup()->GetRootTM(*reinterpret_cast<ZMat3x3*>(pMat), *reinterpret_cast<ZVector3*>(pPos));
    }

    void ZActor::GetActorWorldPosition(float* pPos)
    {
        m_baseGeom->ParentGroup()->GetWorldPosition(*reinterpret_cast<ZVector3*>(pPos));
    }

    void ZActor::SetActorWorldPosition(const float* pPos)
    {
        m_baseGeom->ParentGroup()->SetWorldPosition(*reinterpret_cast<const ZVector3*>(pPos));
    }

    bool ZActor::CanPlayAnimSegment(Animation::Header* pHeader, float startFrame, float endFrame, const float* pRootMat, const float* pRootPos, bool bMirror, float fHeight, float fDepth)
    {
        if (m_Path.m_Size > 0 && m_Action[0] == 4)
            return false;

        return ZIKLNKOBJ::CanPlayAnimSegment(pHeader, startFrame, endFrame, pRootMat, pRootPos, bMirror, fHeight, fDepth);
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
        switch (state)
        {
        case ACTORSTATE_DEAD:
            SetSoundActive(false);
            StopAudio();
            RemovePath(2, eREQUEST, false);
            RemoveCurrentLnkAction();
            g_pEngineData->m_pPathfinder4Data->RemoveNode(&m_Reservation);
            break;

        case ACTORSTATE_AWAKE:
            ZCheckVisible::m_pCheckVisible->RemoveSeeableActor(this);
            SetSeeing(true);
            break;

        case ACTORSTATE_UNCONSCIOUS:
            SetSeeing(false);
            ZCheckVisible::m_pCheckVisible->AddSeeableActor(this, 4u);
            StopAllAnims(false);
            RemovePath(2, eREQUEST, false);
            RemoveCurrentLnkAction();
            break;

        default:
            break;
        }

        m_ActorState = state;
    }

    void ZActor::OnViewEnter()
    {
        if (m_lInsideView++ == 0)
        {
            if ((m_baseGeom->m_lControl & 0x800) == 0)
                Hide(false);
        }
    }

    void ZActor::OnViewLeave()
    {
        if (m_lInsideView-- == 1)
        {
            if ((m_baseGeom->m_lControl & 0x800) == 0)
                Hide(true);
        }
    }

    void* ZActor::GetPathFinder4()
    {
        return g_pEngineData->m_pPathfinder4Data;
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
        if (pDirection)
        {
            m_vEndDir.x = pDirection->x;
            m_vEndDir.y = 0.0f;
            m_vEndDir.z = pDirection->z;
            vnorm(&m_vEndDir.x);
            m_bUseEndDir = true;
        }
        else
        {
            m_bUseEndDir = false;
        }
    }
    const Vector3* ZActor::GetEndDir()
    {
        return &m_vEndDir;
    }

    void ZActor::MoveToPosition(ZVector3* pvEndPos, ZVector3* pEndDir)
    {
        PF4::ZInterface* pPathFinder = g_pEngineData->m_pPathfinder4Data;
        uint8_t* pMask1 = reinterpret_cast<uint8_t*>(&m_Mask1);

        pMask1[3] &= 0xFC;
        pMask1[2] &= 0x7F;
        m_Mask2 &= 0xFFFFFFFD;

        if (m_Action[0] != -1 && !Locomotion::GetProgram(m_Action[0])->Interruptable(this))
        {
            OnPathError(NO_PATH);
            return;
        }

        pMask1[1] &= 0xDF;
        pMask1[2] &= ~0x01;

        float vMoveFromPos[3];
        GetMoveFromPos(vMoveFromPos);
        (void)vMoveFromPos;

        m_vEndPos = *pvEndPos;
        SetEndDir(pEndDir);

        PF4::ZLocation dest;
        if (!pPathFinder->MapValidLocation(*pvEndPos, dest))
        {
            pPathFinder->MapLocation(*pvEndPos, dest);
            pPathFinder->MapInside(dest, dest, true);
        }

        if (dest.m_Component == -1 || m_Entity.m_Location.m_Graph == -1)
        {
            OnPathError(NO_PATH);
            return;
        }

        ZASSERT(dest.m_Inside != 0);

        PF4::ZPathRequest request{};
        request.m_Source = &m_Entity.m_Location;
        request.m_Dest = &dest;
        request.m_Reservation = &m_Reservation;
        request.m_Path = &m_Path;
        request.m_GoOutside = false;
        request.m_ActorKeymask = m_iPathfinderDoorKeyMask;
        request.m_IgnoreReservation = m_bIgnoreReservations;
        request.m_StoppingDistance = m_fEndDist;

        m_Reservation.m_Type = g_pEngineData->m_pEntityTracker->m_ReservedPointId;

        if (!pPathFinder->FindPath(&request))
        {
            OnPathError(NO_PATH);
            m_Path.Clear();
            PreparePath();
            return;
        }

        if (!m_Path.m_pathIdx)
        {
            m_Path.Clear();
            PreparePath();
            return;
        }

        if (m_pkBoid->IsMovementPausing())
            m_pkBoid->m_fPauseMovementAtDistanc = -1.0f;

        const float fPathCost = -m_Path.m_Cost;
        m_fPathNotify = m_fPathNotifyNext;
        m_fPathNotifyNext = 0.0f;
        if (!(fPathCost < m_fPathNotify) && !(fPathCost == m_fPathNotify))
            OnPathNotify();
        if (m_fPathNotify > 0.0f)
            m_fPathNotify -= m_Path.m_Cost;

        if (m_fEndDist != 0.0f)
        {
            pMask1[1] &= 0xFE; // clear m_bUseEndDir
            m_fEndDist = 0.0f;
        }

        pMask1[0] |= 0x01; // set m_bShootIntoGround
        pMask1[0] &= ~0x20; // clear m_bPathRequest

        if (m_pkBoid)
            m_pkBoid->m_fPauseMovementAtDistanc = -1.0f;

        OnNewPath();

        if (m_Locomotion.m_State == LOCOSTATE_DISABLED)
            RestartLocomotionSystem();

        if (m_Action[0] != -1)
            Locomotion::GetProgram(m_Action[0])->ReleaseControl(this, true);

        PreparePath();

        if (m_Path.m_Size > 0 && !m_bCancelPathWhenPossible)
        {
            m_Action[0] = 0;
            Locomotion::GetProgram(0)->TakeControl(this, true);
            ++m_iQuePos;
            return;
        }

        OnPathError(NO_PATH);
        m_Path.Clear();
        PreparePath();
    }

    void ZActor::SlideToPosition(ZVector3*, ZVector3*)
    {
        // 0x463710 is a shared debug-break stub: the PC build has no implementation.
        ZASSERT(false);
    }

    void ZActor::SetPathNotify(float value)
    {
        m_fPathNotifyNext = value;
    }
    void ZActor::SetPathNotifySyncToCycle(bool enabled)
    {
        m_bPathNotifySyncToCycle = enabled;
    }
    void ZActor::SetEndAction(int8_t action)
    {
        m_iEndAction = action;
    }

    void ZActor::RemovePath(uint8_t doneFlag, PATH_CANCEL_REASONS reason, bool)
    {
        m_lPathDoneFlag = static_cast<PATHDONEFLAG>(doneFlag);
        m_ePathCancelReason = reason;
        m_iEndAction = -1;

        if (m_Path.m_Size > 0)
        {
            g_pEngineData->m_pPathfinder4Data->FreePath(&m_Path);
            PreparePath();

            if (m_lPathDoneFlag != PATH_FINISHED)
                m_bUseEndDir = false;

            SendPathDone(m_ActorState != ACTORSTATE_DEAD);
        }
    }

    void ZActor::SendPathDone(bool)
    {
        if (m_lPathDoneFlag == PATH_RESERVED)
            return;

        switch (m_lPathDoneFlag)
        {
        case PATH_FINISHED:
            if (m_Path.m_Size <= 0)
                OnPathFinished(0);
            break;

        case PATH_CANCELED:
            ZASSERT(m_ePathCancelReason != eNONE);
            OnPathCanceled(m_ePathCancelReason);
            break;

        case PATH_ERROR:
            OnPathError(NO_PATH);
            break;

        default:
            ZASSERT(false);
            OnPathError(NO_PATH);
            break;
        }

        m_lPathDoneFlag = PATH_RESERVED;
        m_ePathCancelReason = eNONE;
    }

    void ZActor::GetMoveFromPos(float* pPos)
    {
        pPos[0] = 0.0f;
        pPos[1] = 0.0f;
        pPos[2] = 0.0f;
        GetActorWorldPosition(pPos);
    }

    void* ZActor::FindNextPathBlocker()
    {
        return nullptr;
    }

    void ZActor::SetShootIntoGround(bool enabled)
    {
        m_bShootIntoGround = enabled;
        m_LastShootIntoGroundHeight = 0.0f;
        m_InterpolatedHeightError = 0.0f;
    }
    void ZActor::SetShootIntoGroundInFullbody(bool enabled)
    {
        m_bShootIntoGroundInFullbody = enabled;
    }
    void ZActor::SetStayInsidePath(bool enabled)
    {
        m_bStayInsidePath = enabled;
    }

    // ZActor::ShootIntoGround (PC 0x507A70). Gates on the owning room's visibility, probes
    // the ground under the actor, snaps the position to the hit height and (when picking
    // up) links the actor to the hit room and refreshes the foot-dust template.
    void ZActor::ShootIntoGround(float* pPosition, bool bShoot, bool bPickup)
    {
        if (m_bCutSequence)
            return;

        bool bAllowed = false;
        ZBaseGeom* pParentBaseGeom = m_baseGeom->ParentGroup()->BaseGeom();

        if ((pParentBaseGeom->m_lControl & 0x40000) != 0)
        {
            // Container: only shoot while one of the linked rooms is visible.
            if (ZBaseGeomRoomList* pRoomList = pParentBaseGeom->GetRoomListPtr())
            {
                const uint32_t lFrameCount = g_pSysInterface->m_lFrameCount;
                for (uint8_t i = 0; i < pRoomList->m_cNrRooms; ++i)
                {
                    ZROOM* pRoom = pRoomList->m_pRooms[i];
                    if (pRoom && ((pRoom->m_baseGeom->m_lControl & 0x1000) != 0
                        || pRoom->m_iLastVisibleFrameCount == lFrameCount - 1))
                    {
                        bAllowed = true;
                        break;
                    }
                }
            }

            if (!bAllowed && !bShoot)
                return;
        }
        else
        {
            // Otherwise walk the base-geom parent chain up to the owning room.
            const uint32_t lFrameCount = g_pSysInterface->m_lFrameCount;
            for (ZBaseGeom* pWalk = pParentBaseGeom; pWalk; pWalk = pWalk->m_pParent)
            {
                ZGEOM* pGeom = pWalk->GetGeom();
                const bool bIsRoom = pGeom
                    ? (ZROOM::m_Mask & pGeom->GetObjectId()) == ZROOM::m_Id
                    : pWalk->IsDerivedFromStdObj(ZROOM::m_Id);
                if (!bIsRoom)
                    continue;

                if ((pWalk->m_lControl & 0x1000) != 0
                    || (pGeom && static_cast<ZROOM*>(pGeom)->m_iLastVisibleFrameCount == lFrameCount - 1))
                {
                    bAllowed = true;
                }
                break;
            }

            if (!bAllowed && !bShoot)
                return;
        }

        m_LastShootIntoGroundPosition.x = pPosition[0];
        m_LastShootIntoGroundPosition.y = pPosition[1];
        m_LastShootIntoGroundPosition.z = pPosition[2];

        ZREF rGround = 0;
        SExtendedImpactInfo impact;

        if (m_bShootIntoGround)
        {
            const float fStart[3] = { pPosition[0], pPosition[1] + 100.0f, pPosition[2] };
            const float fDir[3] = { 0.0f, -250.0f, 0.0f };
            const bool bCheckDynamic = m_bShootIntoDynamic;
            ZCollisionBase* pColi = ZCollisionBase::s_pCollisionBase;

            if (pColi && pColi->CalcLineColi(&impact, eGlobalTreeType::GT_StdObjs,
                    fStart, fDir, true, m_lShootIntoGroundFlag, true, bCheckDynamic))
            {
                pPosition[1] = impact.vPosition.y;
                m_bShootIntoGroundFailed = false;
                rGround = ZGeomBuffer::m_Instance->GeomPtrToRef(impact.pBaseGeom);
            }
            else
            {
                // Retry from a jittered start point with a long downward ray.
                float vJitter[3] = { g_pSysInterface->FRand(nullptr, 0), 0.0f,
                    g_pSysInterface->FRand(nullptr, 0) };
                if (vnorm(vJitter) == 0.0f)
                {
                    vJitter[0] = 0.0f;
                    vJitter[1] = 0.0f;
                    vJitter[2] = 1.0f;
                }

                float fStartJittered[3];
                vaddscalar(fStartJittered, fStart, vJitter, 10.0f);

                const float fDown[3] = { 0.0f, -5000.0f, 0.0f };
                if (pColi && pColi->CalcLineColi(&impact, eGlobalTreeType::GT_StdObjs,
                        fStartJittered, fDown, true, m_lShootIntoGroundFlag, true, bCheckDynamic))
                {
                    pPosition[1] = impact.vPosition.y;
                    m_bShootIntoGroundFailed = false;
                    rGround = ZGeomBuffer::m_Instance->GeomPtrToRef(impact.pBaseGeom);
                }
                else
                {
                    m_bShootIntoGroundFailed = true;
                }
            }

            if (m_bShootIntoGroundFailed)
            {
                SetContactGeom(0);
                m_ContactNormal = ZVector3{};
            }
            else
            {
                SetContactGeom(rGround);

                float vEdge1[3];
                float vEdge2[3];
                vsub(vEdge1, &impact.vP3.x, &impact.vP1.x);
                vsub(vEdge2, &impact.vP2.x, &impact.vP1.x);
                vcross(&m_ContactNormal.x, vEdge1, vEdge2);
                vnorm(&m_ContactNormal.x);
                if (impact.pBaseGeom)
                    impact.pBaseGeom->GetRootVect(m_ContactNormal);
                vnorm(&m_ContactNormal.x);
            }

            m_LastShootIntoGroundHeight = pPosition[1];
            m_InterpolatedHeightError = 0.0f;
        }

        // Link the actor to the room of the hit surface (and, on PC, create its foot dust).
        if (bPickup && !m_bShootIntoGroundFailed && rGround)
        {
            ZGEOM* pGroundGeom = ZGEOM::RefToPtr(rGround);
            if (pGroundGeom && pGroundGeom->BaseGeom())
            {
                ZGROUP* pRoomGroup = pGroundGeom->BaseGeom()->ParentGroup();
                while (pRoomGroup && (ZROOM::m_Mask & pRoomGroup->GetObjectId()) != ZROOM::m_Id)
                    pRoomGroup = pRoomGroup->BaseGeom()->ParentGroup();

                if (pRoomGroup)
                {
                    ZBaseGeom* pSelfParentBaseGeom = m_baseGeom->ParentGroup()->BaseGeom();
                    if (ZBaseGeomRoomList* pRoomList = pSelfParentBaseGeom->GetRoomListPtr())
                    {
                        if (!pRoomList->Exists(static_cast<ZROOM*>(pRoomGroup)))
                            pSelfParentBaseGeom->AddToRoomList(static_cast<ZROOM*>(pRoomGroup));
                    }
                    else
                    {
                        pSelfParentBaseGeom->SetMainRoom(static_cast<ZROOM*>(pRoomGroup));
                    }
                }

                // PC 0x5049D0 records the hit surface's material (clamped to >= 1) as the
                // actor's contact material. PC 0x507F32 then loads
                // ZMaterialDescriptionDB::m_Instance (0x008BA0FC) and resolves the
                // ContactDust scene resource for that material, which becomes the actor's
                // foot-dust template. (The IDA symbol shown at PC 0x4F5D00 in the callers
                // is a mislabel of that material-description lookup: both call sites pass
                // ecx == m_Instance and the pair (materialId, scenePropertyId).)
                const uint32_t lMaterialId = impact.m_iColiMaterialDescId
                    ? impact.m_iColiMaterialDescId : 1u;
                m_ContactMaterialDescID = TMaterialDescID { static_cast<int>(lMaterialId) };
                m_pFootDustTemplate = ZGEOM::RefToPtr(
                    BS_Runtime::ZMaterialDescriptionDB::Instance().GetSceneResourceProperty(
                        TMaterialDescID { static_cast<int>(lMaterialId) },
                        ZIKLNKOBJ::ContactDepris,
                        TEnumID { 0 }));
            }
        }

        if (rGround)
            ShootIntoGroundCallback(&impact);
    }

    void ZActor::ShootIntoGroundRegularly(float* pPos, bool bShoot, bool bPickup, float fDeltaTime)
    {
        if (m_bCutSequence || !m_bShootIntoGround)
            return;

        if (IsInElevator() || vzero(&m_ContactNormal.x))
        {
            ShootIntoGround(pPos, bShoot, bPickup);
            return;
        }

        ZVector3 vDelta;
        vsub(&vDelta.x, &m_LastShootIntoGroundPosition.x, pPos);

        const float fHorDist = std::sqrt(vDelta.z * vDelta.z + vDelta.x * vDelta.x);

        float fTargetHeight;
        if (m_ContactNormal.y == 0.0f || m_ContactNormal.y == 1.0f)
            fTargetHeight = m_LastShootIntoGroundHeight;
        else
            fTargetHeight = m_LastShootIntoGroundHeight
                - (vDelta.z * m_ContactNormal.z + vDelta.x * m_ContactNormal.x) * (-1.0f / m_ContactNormal.y);

        const bool bNoHeight = m_LastShootIntoGroundHeight == 0.0f;

        if (fHorDist > 14.0f || m_ContactNormal.y < 0.6f || bNoHeight)
        {
            const float fOldError = m_InterpolatedHeightError;
            ShootIntoGround(pPos, bShoot, bPickup);

            if (bNoHeight)
            {
                m_InterpolatedHeightError = 0.0f;
            }
            else
            {
                float fError = fTargetHeight - pPos[1] + fOldError;
                if (fError > 30.0f)
                    fError = 30.0f;
                else if (fError < -30.0f)
                    fError = -30.0f;
                m_InterpolatedHeightError = fError;
            }
        }
        else
        {
            pPos[1] = fTargetHeight;
        }

        PullToValue(m_InterpolatedHeightError, 0.0f, fDeltaTime * 40.0f);
        pPos[1] += m_InterpolatedHeightError;
    }

    void ZActor::SetMoveSpeedMultiplier(float multiplier)
    {
        ZASSERT(!std::isnan(multiplier) && std::isfinite(multiplier));
        m_fMoveSpeedMultiplier = multiplier;
    }
    void ZActor::SetPathFinderEnabled(bool enabled)
    {
        m_bPathFinderEnabled = enabled;
    }

    void ZActor::SetMovePoolWeigh(float weight)
    {
        if (m_pkBoid)
            m_pkBoid->m_Weight = weight;
    }

    void ZActor::DetermineBanking(LocomotionInfo* pInfo)
    {
        if (!m_Model)
            return;

        m_Model->m_Banking[0] = 0.0f;
        m_Model->m_Banking[1] = 0.0f;

        if (g_BankingEnabled && m_Banking != 0.0f)
        {
            ZVector3 vLean;
            vLean.x = -pInfo->currentDir.y;
            vLean.y = 0.0f;
            vLean.z = pInfo->currentDir.x;

            if (pInfo->currentSpeed > 0.0f)
            {
                vscalar(&vLean.x, &vLean.x, m_Banking * 5.0f);
                m_Model->m_Banking[0] = vLean.x;
                m_Model->m_Banking[1] = vLean.z;
            }
        }
    }

    void ZActor::PreparePath()
    {
        m_iQuePos = 0xFFFF;
        m_iProgramQueSize = 0;

        bool bOverflow = false;
        const int pathSize = m_Path.m_Size;
        if (pathSize > 0)
        {
            const int action = m_Path.GetAction(0);
            m_ProgramQue[m_iProgramQueSize].m_StartIndex = 0;
            m_ProgramQue[m_iProgramQueSize].m_PathAction = static_cast<int8_t>(action);
            m_ProgramQue[m_iProgramQueSize].m_Program =
                static_cast<int8_t>(Locomotion::ZStates::m_PathAction2Program[action]);
            int lastAction = action;

            int i = 1;
            if (pathSize > 1)
            {
                for (;;)
                {
                    if (static_cast<int16_t>(m_iProgramQueSize) >= 40)
                    {
                        bOverflow = true;
                        break;
                    }

                    if (m_Path.GetAction(i) != lastAction)
                    {
                        m_ProgramQue[m_iProgramQueSize++].m_EndIndex = static_cast<uint8_t>(i - 1);

                        const int newAction = m_Path.GetAction(i);
                        m_ProgramQue[m_iProgramQueSize].m_PathAction = static_cast<int8_t>(newAction);
                        lastAction = newAction;
                        m_ProgramQue[m_iProgramQueSize].m_Program =
                            static_cast<int8_t>(Locomotion::ZStates::m_PathAction2Program[newAction]);

                        m_ProgramQue[m_iProgramQueSize].m_StartIndex =
                            static_cast<uint8_t>((i <= 0 || m_ProgramQue[m_iProgramQueSize].m_Program) ? i : i - 1);
                    }

                    if (++i >= pathSize)
                        break;
                }
            }

            m_ProgramQue[m_iProgramQueSize++].m_EndIndex = static_cast<uint8_t>(i - 1);
        }

        if (!bOverflow)
        {
            if (m_iEndAction != -1)
            {
                m_ProgramQue[m_iProgramQueSize].m_StartIndex = 0;
                m_ProgramQue[m_iProgramQueSize].m_EndIndex = 0;
                m_ProgramQue[m_iProgramQueSize].m_Program = m_iEndAction;
                m_ProgramQue[m_iProgramQueSize++].m_PathAction = -1;
            }

            if (static_cast<int16_t>(m_iProgramQueSize) < 40)
            {
                m_ProgramQue[m_iProgramQueSize].m_StartIndex = 0;
                m_ProgramQue[m_iProgramQueSize].m_EndIndex = 0;
                m_ProgramQue[m_iProgramQueSize].m_Program = 5;
                m_ProgramQue[m_iProgramQueSize].m_PathAction = -1;
                ++m_iProgramQueSize;

                if (static_cast<int16_t>(m_iProgramQueSize) < 40)
                {
                    m_ProgramQue[m_iProgramQueSize].m_StartIndex = 0;
                    m_ProgramQue[m_iProgramQueSize].m_EndIndex = 0;
                    m_ProgramQue[m_iProgramQueSize].m_Program = -1;
                    m_ProgramQue[m_iProgramQueSize].m_PathAction = -1;

                    if (static_cast<int16_t>(m_iProgramQueSize) < 40)
                        return;
                }
            }
        }

        m_Path.Clear();
        PreparePath();
        m_Mask1 |= 0x02000000u;
    }

    // ZActor::GetDesiredProgram (PC 0x502B50): the locomotion program the actor's current
    // state wants to run (used when syncing m_Action to the movement/path state).
    int8_t ZActor::GetDesiredProgram()
    {
        if ((GroundAnimated() && (GetGroundAnimation()->mode & 0x2000) == 0)
            || (m_pBoneModify && m_pBoneModify->IsRagdollActive()))
        {
            return 1;
        }

        if (m_ActorState == ACTORSTATE_AWAKE)
        {
            if (m_bWounded)
                return 3;

            if (m_Path.m_Size <= 0)
                return 5;

            const int16_t quePos = static_cast<int16_t>(m_iQuePos);
            if (quePos != -1)
                return m_ProgramQue[quePos].m_Program;

            return static_cast<int8_t>(m_iQuePos);
        }

        return 2;
    }

    // ZActor::SyncProgramQueue (PC 0x502C50): advances the locomotion program queue by up
    // to two entries, handing control between the current and next program.
    void ZActor::SyncProgramQueue()
    {
        if (m_Path.m_Size <= 0)
            return;

        int steps = 0;
        for (;;)
        {
            bool bAdvanced = false;
            const int16_t quePos = static_cast<int16_t>(m_iQuePos);
            if (quePos + 1 < static_cast<int16_t>(m_iProgramQueSize))
            {
                const int8_t currProgram = m_ProgramQue[quePos].m_Program;
                if (Locomotion::GetProgram(currProgram)->ReleaseControl(this, false))
                {
                    if (m_Path.m_Size <= 0)
                        return;

                    const int16_t quePos2 = static_cast<int16_t>(m_iQuePos);
                    if (quePos2 + 1 >= static_cast<int16_t>(m_iProgramQueSize))
                        return;

                    Locomotion::GetProgram(m_ProgramQue[quePos2 + 1].m_Program)->TakeControl(this, true);
                    ++m_iQuePos;
                    bAdvanced = true;
                }
                else
                {
                    const int8_t nextProgram = m_ProgramQue[quePos + 1].m_Program;
                    if (!Locomotion::GetProgram(nextProgram)->TakeControl(this, false))
                    {
                        // nothing to advance
                    }
                    else if (m_Path.m_Size <= 0)
                    {
                        Locomotion::GetProgram(nextProgram)->ReleaseControl(this, true);
                        return;
                    }
                    else
                    {
                        Locomotion::GetProgram(currProgram)->ReleaseControl(this, true);
                        ++m_iQuePos;
                        bAdvanced = true;
                    }
                }
            }

            if (m_Path.m_Size <= 0)
            {
                m_Action[0] = -1;
                m_Action[1] = -1;
                return;
            }

            const int16_t curPos = static_cast<int16_t>(m_iQuePos);
            if (curPos + 1 >= static_cast<int16_t>(m_iProgramQueSize))
            {
                m_Action[0] = m_ProgramQue[curPos].m_Program;
                m_Action[1] = -1;
            }
            else
            {
                m_Action[0] = m_ProgramQue[curPos].m_Program;
                m_Action[1] = m_ProgramQue[curPos + 1].m_Program;
            }

            if (!bAdvanced || ++steps >= 2)
                return;
        }
    }

    // ZActor::UpdateProgramQueue (PC 0x505860): re-syncs m_Action with the locomotion state
    // returned by GetDesiredProgram() and with the current path queue.
    void ZActor::UpdateProgramQueue()
    {
        const int8_t desiredProgram = GetDesiredProgram();
        const int8_t currProgram = m_Action[0];
        if (desiredProgram != currProgram
            && (currProgram == -1 || Locomotion::GetProgram(currProgram)->Breakable(this)))
        {
            if (m_Path.m_Size > 0)
                RemovePath(2, eUNSPECIFIED, false);

            m_iEndAction = -1;
            PreparePath();

            const int8_t oldProgram = m_Action[0];
            if (oldProgram != -1)
                Locomotion::GetProgram(oldProgram)->ReleaseControl(this, true);

            m_Action[0] = desiredProgram;
            if (static_cast<uint8_t>(desiredProgram) != 0xFF)
                Locomotion::GetProgram(desiredProgram)->TakeControl(this, true);
            m_Action[1] = -1;
        }
        else if (m_Path.m_Size > 0)
        {
            const int16_t quePos = static_cast<int16_t>(m_iQuePos);
            m_Action[0] = m_ProgramQue[quePos].m_Program;
            m_Action[1] = m_ProgramQue[quePos + 1].m_Program;
            SyncProgramQueue();
        }

        const int nextQue = static_cast<int16_t>(m_iQuePos) + 1;
        if (nextQue >= static_cast<int16_t>(m_iProgramQueSize))
        {
            const int8_t lastProgram = m_Action[0];
            if (lastProgram != -1)
            {
                Locomotion::GetProgram(lastProgram)->ReleaseControl(this, true);
                m_Action[0] = -1;
                m_Action[1] = -1;
            }
        }
    }

    // ZActor::DecreaseHeadTargetWeight (PC 0x502BE0): decays the model's look-at weight.
    void ZActor::DecreaseHeadTargetWeight(float fDeltaTime)
    {
        if (m_Model && m_Model->m_Targets[0].m_Weight2 != 0.0f)
        {
            m_Model->m_Targets[0].m_Weight2 -= fDeltaTime;
            if (m_Model->m_Targets[0].m_Weight2 < 0.0f)
                m_Model->m_Targets[0].m_Weight2 = 0.0f;
        }
    }

    // ZActor::DecayBanking (PC 0x5057F0): relaxes the model's banking offset (PC 0x5048A0).
    void ZActor::DecayBanking(float fDeltaTime)
    {
        if (!m_Model)
            return;

        if (g_BankingEnabled)
        {
            const float fAmount = fDeltaTime * 30.0f;
            PullToValue(m_Model->m_Banking[0], 0.0f, fAmount);
            PullToValue(m_Model->m_Banking[1], 0.0f, fAmount);
        }
        else
        {
            m_Model->m_Banking[0] = 0.0f;
            m_Model->m_Banking[1] = 0.0f;
        }
    }

    // ZActor::ResetLocomotionModel (PC 0x504060): clears the temporary locomotion/model state.
    void ZActor::ResetLocomotionModel()
    {
        uint8_t* pRuntimeFlags = reinterpret_cast<uint8_t*>(&m_Locomotion.m_RuntimeFlags);
        const uint8_t runtimeFlags = pRuntimeFlags[0];

        m_Locomotion.m_State = LOCOSTATE_DISABLED;
        m_Locomotion.m_CurrentAnim = nullptr;
        m_Locomotion.m_ActiveAnim = nullptr;
        m_Locomotion.m_LocomotionSetEntry = 0;
        m_Locomotion.m_LocomotionSetTransition = -1;
        m_Locomotion.m_CurrentTransition = nullptr;
        pRuntimeFlags[0] = runtimeFlags & 0x7F;
        m_fCurrentBaseMoveFramePrc = 0.0f;
    }

    void ZActor::OnPathRequest(uint32_t request)
    {
        ZASSERT(m_msgPathRequest);
        SendCommand(m_msgPathRequest, reinterpret_cast<void*>(static_cast<uintptr_t>(request)), nullptr);
    }

    void ZActor::OnPathFinished(uint32_t response)
    {
        if (!m_bPathDoneMsgSent)
        {
            if (m_fPathNotify < 0.0f)
                OnPathNotify();

            ZASSERT(m_msgPathFinished);
            SendCommand(m_msgPathFinished, reinterpret_cast<void*>(static_cast<uintptr_t>(response)), nullptr);
            m_bPathNotifySyncToCycle = false;
            m_fPathNotify = 0.0f;
        }

        m_bPathDoneMsgSent = true;
    }

    void ZActor::OnPathCanceled(PATH_CANCEL_REASONS reason)
    {
        if (!m_bPathDoneMsgSent)
        {
            ZASSERT(m_msgPathCanceled);
            SendCommand(m_msgPathCanceled, reinterpret_cast<void*>(static_cast<uintptr_t>(reason)), nullptr);
            m_bPathNotifySyncToCycle = false;
            m_fPathNotify = 0.0f;
        }

        m_bPathDoneMsgSent = true;
    }

    void ZActor::OnPathCanceledLockedDoor(uint32_t door)
    {
        if (!m_bPathDoneMsgSent)
        {
            ZASSERT(m_msgPathCanceledLockedDoor);
            SendCommand(m_msgPathCanceledLockedDoor, reinterpret_cast<void*>(static_cast<uintptr_t>(door)), nullptr);
            m_bPathNotifySyncToCycle = false;
            m_fPathNotify = 0.0f;
        }

        m_bPathDoneMsgSent = true;
    }

    void ZActor::OnPathError(PATH_ERRORS error)
    {
        if (!m_bPathDoneMsgSent)
        {
            ZASSERT(m_msgPathError);
            SendCommand(m_msgPathError, reinterpret_cast<void*>(static_cast<uintptr_t>(error)), nullptr);
            m_bPathNotifySyncToCycle = false;
            m_fPathNotify = 0.0f;
        }

        m_bPathDoneMsgSent = true;
    }

    void ZActor::OnPathNotify()
    {
        if (!m_bPathNotifyMsgSent)
        {
            ZASSERT(m_msgPathNotify);
            SendCommand(m_msgPathNotify, nullptr, nullptr);
            m_bPathNotifySyncToCycle = false;
            m_fPathNotify = 0.0f;
        }

        m_bPathNotifyMsgSent = true;
    }

    void ZActor::OnNewPath()
    {
        // Empty in the PC build (0x4715C0 is a shared empty stub).
    }

    void ZActor::OnSound(REFTAB* pSounds)
    {
        if (!pSounds)
            return;

        struct SAudioEventMessage
        {
            uint32_t m_lType;
            uint32_t m_lArg0;
            uint32_t m_lArg1;
            ZVector3 m_vPosition;
        };

        // 2052 is the audio-event script message id as resolved in the PC build.
        static constexpr ZMSGID kAudioEventMsg = 2052;

        RefRun run;
        pSounds->RunInitNxtRef(&run);

        for (uint32_t* pEvent = pSounds->RunNxtRefPtr(&run); pEvent; pEvent = pSounds->RunNxtRefPtr(&run))
        {
            SAudioEventMessage message;
            message.m_lType = pEvent[3];
            message.m_lArg0 = pEvent[5];
            message.m_lArg1 = pEvent[7];
            message.m_vPosition.x = *reinterpret_cast<const float*>(&pEvent[0]);
            message.m_vPosition.y = *reinterpret_cast<const float*>(&pEvent[1]);
            message.m_vPosition.z = *reinterpret_cast<const float*>(&pEvent[2]);

            SendCommand(kAudioEventMsg, &message, nullptr);
        }
    }

    void ZActor::Die()
    {
        SetActorState(ACTORSTATE_DEAD);
    }

    void ZActor::DieByForce(const float*, const float*, float, uint32_t)
    {
        Die();
    }

    void ZActor::Resurrect()
    {
        SetActorState(ACTORSTATE_AWAKE);
    }
    void ZActor::Knockout()
    {
        if (m_ActorState != ACTORSTATE_UNCONSCIOUS)
            SetActorState(ACTORSTATE_UNCONSCIOUS);
    }
    void ZActor::Revive()
    {
        if (!IsDead())
            Resurrect();
    }
    bool ZActor::GetKnockedOut() const
    {
        // Shared return-false stub in the PC build (0x69D5C0).
        return false;
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
        // Shared return-true stub in the PC build (0x7113D0).
        return true;
    }

    bool ZActor::VerifyPlayerVisible()
    {
        // Shared return-true stub in the PC build (0x580400).
        return true;
    }

    // ZLNKOBJ helper (PC 0x5168C0 / iOS ZLNKOBJ::HasRunningUBAnim): true when the model
    // currently plays an animation that blocks head look-at (a non-looping body anim).
    static bool HasRunningUBAnim(const Animation::Model* pModel)
    {
        if (!pModel)
            return false;

        for (int i = 0; i < 4; ++i)
        {
            const int32_t mode = pModel->m_ActiveAnims[i].mode;
            const int32_t type = mode & 7;
            if ((type == 1 || type == 2 || type == 3) && (mode & 0x8000) == 0)
                return true;
        }

        return false;
    }

    // ZActor::DeterminePathLookAt (PC 0x503B80). pOut2 is the actor's facing direction
    // and pOut3 the actor's world position; the result is written into the model's
    // look-at IK target (m_Model->m_Targets[0]).
    void ZActor::DeterminePathLookAt(LocomotionInfo* pInfo, float* pOut1, float* pOut2, float* pOut3, float* pOut4)
    {
        (void)pOut1;
        (void)pOut4;

        const float* pActorDir = pOut2;
        const float* pActorPos = pOut3;

        if (HasRunningUBAnim(m_Model))
        {
            m_Locomotion.m_LookMode = 0;
            m_Locomotion.m_LookAtTarget = nullptr;
            return;
        }

        float vLookOut[3] = { 0.0f, 0.0f, 0.0f };

        const int8_t lookMode = m_Locomotion.m_LookMode;
        if (lookMode < 2)
        {
            if (m_Path.m_Size >= 1)
            {
                float vLook[3];
                const float fDistToTarget = vdist(pActorPos, &m_pkBoid->m_Targets[0].m_vPos.x);

                if (fDistToTarget > 300.0f && (g_pSysInterface->Rand(nullptr, 0) & 0x3FF) > 0x3E8)
                {
                    m_Locomotion.m_LookAtTarget = nullptr;
                    m_Locomotion.m_LookMode = 2;
                }

                if (fDistToTarget > 200.0f || m_pkBoid->m_Targets[0].m_bEndPoint)
                    vsub(vLook, &m_pkBoid->m_Targets[0].m_vPos.x, pActorPos);
                else
                    vsub(vLook, &m_pkBoid->m_Targets[1].m_vPos.x, &m_pkBoid->m_Targets[0].m_vPos.x);

                if (fDistToTarget < 100.0f && m_bShootIntoGround)
                    vscalar(vLook, &m_vEndDir.x, 500.0f);

                float vNormalized[3];
                vnorm(vNormalized, vLook);

                if (vNormalized[1] <= 0.2f)
                {
                    if (vNormalized[1] < -0.4f)
                        vLook[1] = (-0.4f / vNormalized[1]) * vLook[1];
                }
                else
                {
                    vLook[1] = (0.2f / vNormalized[1]) * vLook[1];
                }

                vLookOut[0] = vLook[0];
                vLookOut[1] = vLook[1];
                vLookOut[2] = vLook[2];
            }
        }
        else if (lookMode == 2)
        {
            if (!m_Locomotion.m_LookAtTarget)
            {
                PF4::ZInterface::ZResult results[2];
                const int iFound = g_pEngineData->m_pEntityTracker->GetClosest(
                    m_Entity.m_Location, results, 2, 500.0f);

                for (int i = 0; i < iFound; ++i)
                {
                    if (results[i].pNode != &m_Entity)
                        m_Locomotion.m_LookAtTarget = results[i].pNode;
                }

                if (m_Locomotion.m_LookAtTarget)
                {
                    ZBoid* pBoid = m_Locomotion.m_LookAtTarget->m_pBoid;
                    if (pBoid && !CanLookAt(static_cast<ZGEOM*>(pBoid->m_pActor)))
                        m_Locomotion.m_LookAtTarget = nullptr;
                }
            }

            if (m_Locomotion.m_LookAtTarget)
            {
                vsub(vLookOut, &m_Locomotion.m_LookAtTarget->m_Location.m_vPos.x, pActorPos);

                const float fLookLen = std::sqrt(vLookOut[0] * vLookOut[0]
                    + vLookOut[1] * vLookOut[1] + vLookOut[2] * vLookOut[2]);
                if (vLookOut[0] * pActorDir[0] + vLookOut[1] * pActorDir[1] + vLookOut[2] * pActorDir[2] < 0.3f
                    || (g_pSysInterface->Rand(nullptr, 0) & 0x3FF) > 0x3F2
                    || fLookLen >= 1000.0f)
                {
                    m_Locomotion.m_LookAtTarget = nullptr;
                    m_Locomotion.m_LookMode = 1;
                }
            }
            else
            {
                m_Locomotion.m_LookMode = 1;
            }
        }

        if (vLookOut[0] * vLookOut[0] + vLookOut[1] * vLookOut[1] + vLookOut[2] * vLookOut[2] > 50.0f)
        {
            vnorm(vLookOut);
            vscalar(vLookOut, 1000.0f);

            ZVector3 vTarget;
            vTarget.x = pActorPos[0] + vLookOut[0];
            vTarget.y = pActorPos[1] + vLookOut[1] + 180.0f;
            vTarget.z = pActorPos[2] + vLookOut[2];

            m_Model->m_Targets[0].m_Pos2 = vTarget;

            float fSpeed = pInfo->wantedSpeed - 150.0f;
            if (fSpeed < 0.0f)
                fSpeed = 0.0f;

            m_Model->m_Targets[0].m_LookAt2.m_Mode = 2;
            m_Model->m_Targets[0].m_LookAt2.m_Speed = fSpeed / 150.0f * 1.5f + 1.0f;
            m_Model->m_Targets[0].m_Weight2 = std::min(
                m_Model->m_Targets[0].m_Weight2 + g_pSysInterface->DeltaFrameTime * 4.0f, 1.0f);
        }
    }

    bool ZActor::CanLookAt(ZGEOM*) const
    {
        // Shared return-true stub in the PC build (0x489C50).
        return true;
    }

    bool ZActor::IsOnStairs() const
    {
        // Shared return-false stub in the PC build (0x69D5C0).
        return false;
    }

    void ZActor::UpdatePositionOffScreen()
    {
        UpdatePosition();
    }

    // ZActor::UpdatePosition (PC 0x507F70).
    void ZActor::UpdatePosition()
    {
        if (!m_bUpdatePosition)
            return;

        ZVector3 vPosition;
        GetActorWorldPosition(&vPosition.x);
        vPosition.x = m_pkBoid->m_Tracker.x;
        vPosition.z = m_pkBoid->m_Tracker.z;
        SetActorWorldPosition(&vPosition.x);

        DecreaseHeadTargetWeight(g_pSysInterface->DeltaFrameTime);
        DecayBanking(g_pSysInterface->DeltaFrameTime);

        uint8_t* pRuntimeFlags = reinterpret_cast<uint8_t*>(&m_Locomotion.m_RuntimeFlags);
        const bool bPerformFullUpdate = g_PerformFullUpdate;
        pRuntimeFlags[0] &= ~0x04u;

        if (bPerformFullUpdate)
        {
            const float fAccumulator = m_FrameTimeAccumulator;
            UpdateProgramQueue();

            if (m_Action[0] != -1)
            {
                if (m_Action[0] != 0)
                    Locomotion::GetProgram(m_Action[0])->UpdatePosition(this, fAccumulator);
                else
                    Locomotion::GetProgram(0)->UpdatePosition(this, g_pSysInterface->DeltaFrameTime);
            }

            if (m_bStopMovement)
            {
                if (m_pkBoid->m_fPauseMovementAtDistanc < 0.0f)
                {
                    m_bCancelPathWhenPossible = true;
                    m_bStopMovement = false;
                }
                else if (m_pkBoid->IsMovementPausing()
                    && m_pkBoid->m_ActualSpeed <= 0.0f
                    && (m_pkBoid->m_TrackerDist > 0.0f || m_Locomotion.m_State == LOCOSTATE_STAND))
                {
                    m_bCancelPathWhenPossible = true;
                    m_bStopMovement = false;
                }
            }

            if (m_bCancelPathWhenPossible && m_Path.m_Size > 0)
                RemovePath(2, eUNSPECIFIED, false);

            if (m_Path.m_Size > 0)
            {
                const float fRemaining = m_pkBoid->TotalRemaining();

                if (m_fPathNotify != 0.0f && -m_fPathNotify <= fRemaining)
                    OnPathNotify();

                if (!m_bPathRequest && fRemaining <= 0.0f)
                {
                    m_bPathRequest = true;
                    SetPathNotifySyncToCycle(true);
                }

                if (fRemaining <= 0.0f)
                {
                    RemovePath(PATH_FINISHED, eNONE, false);
                    m_pkBoid->SetSpeed(0.0f);
                    m_pkBoid->m_ActualSpeed = 0.0f;
                }
            }

            if ((m_Locomotion.m_RuntimeFlags & 0x04) == 0)
                ResetLocomotionModel();

            return;
        }

        if (m_Path.m_Size > 0)
        {
            ZVector3 vPos;
            GetActorWorldPosition(&vPos.x);
            vPos.x = m_pkBoid->m_Tracker.x;
            vPos.z = m_pkBoid->m_Tracker.z;
            SetActorWorldPosition(&vPos.x);

            const int16_t quePos = static_cast<int16_t>(m_iQuePos);
            m_Action[0] = m_ProgramQue[quePos].m_Program;
            m_Action[1] = m_ProgramQue[quePos + 1].m_Program;
            SyncProgramQueue();

            if (m_bCancelPathWhenPossible && m_Path.m_Size > 0)
                RemovePath(2, eUNSPECIFIED, false);

            if (m_Path.m_Size > 0)
                Locomotion::GetProgram(m_Action[0])->UpdatePosition(this, g_pSysInterface->DeltaFrameTime);
        }
    }

    Animation::Header* ZActor::GetCurrentUBAnim(ZItem* pRHandItem, ZItem*, bool& bMirrored)
    {
        if (m_Path.m_Size > 0 && m_Action[0] == 4)
            return nullptr;

        if (!pRHandItem)
            return nullptr;

        if (!pRHandItem->IsDerivedFrom<ZItemWeapon>())
            return nullptr;

        if (m_AimTarget.IsEnabled())
            return nullptr;

        bMirrored = false;

        ZItemTemplateWeapon* pTemplate = static_cast<ZItemWeapon*>(pRHandItem)->GetWeaponTemplate();
        const int lWeaponType = pTemplate->GetWeaponType();
        if (lWeaponType == WT_SUBMACHINEGUN || lWeaponType == WT_RIFLE || lWeaponType == WT_PUMPGUN || lWeaponType == WT_SHOTGUN)
            return m_pAnimUBHoldRifle;

        return nullptr;
    }

    void ZActor::SetHoldWeaponUBAnim(Animation::Header* pHeader)
    {
        m_pAnimUBHoldRifle = pHeader;
    }

    uint32_t ZActor::GetAnimOffset(const char* pAnim)
    {
        if (!pAnim)
            return 0;

        return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(pAnim) - reinterpret_cast<uintptr_t>(g_pEngineData->m_pPackedAnims));
    }

    uint32_t ZActor::GetAnimOffset(Animation::Header* pHeader)
    {
        Animation::Header* pAnim = GetAnim(reinterpret_cast<const char*>(pHeader));
        if (!pAnim)
            return 0;

        return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(pAnim) - reinterpret_cast<uintptr_t>(g_pEngineData->m_pPackedAnims));
    }

    Animation::Header* ZActor::GetAnimHeader(uint32_t offset)
    {
        if (!offset)
            return nullptr;

        return reinterpret_cast<Animation::Header*>(reinterpret_cast<uintptr_t>(g_pEngineData->m_pPackedAnims) + offset);
    }

    void ZActor::ShootIntoGroundCallback(SExtendedImpactInfo*)
    {
        // Empty in the PC build (0x6F7550 is a shared empty stub).
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
                        StartAnimSound(m_Locomotion.m_CurrentAnim->GetSoundIndex(),
                            (m_Locomotion.m_CurrentAnim->m_Mask & 8) == 0, nullptr,
                            m_fCurrentBaseMoveFramePrc, false, 0);
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

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        // PC 0x0080D19C (ZActor_Properties_Unk580) is the only ZActor-owned RTP property.
        // It is a ZDataProperty<REFTAB*> -- vtbl 0x008065B4 resolves to
        // RTP::ZDataProperty<REFTAB*>::Load (PC 0x6A15D0) == RTP::VirtualTables::Data_REFTAB_ptr --
        // whose offset is the audible-room-list pointer and whose name is the
        // "m_pAudibleRoomList" string found in the PS2 / XBOX_KL1 / XBOX_KL2 images.
        static RTP::ZDataProperty<REFTAB*> ZActor_Properties_Unk580 {
            .m_Node = { nullptr, "m_pAudibleRoomList", 1 },
            .m_VirtualTable = &RTP::VirtualTables::Data_REFTAB_ptr,
            .m_Offset = CLASS_PROPERTY(ZActor, m_pAudibleRoomList)
        };
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZActor,
        ZLNKWHANDS,
        0x0097BE40,
        "ZActor",
        0x0077156C,
        cProperties::ZActor_Properties_Unk580,
        0x0080D1B0,
        0x0097BD60,
        0x0097BD64);

    STATIC_CLASS_VAR_IMPL(ZActor, ZMessageResolver, m_msgPathResponse, 0x0097BD68, { "PathResponse" });
    STATIC_CLASS_VAR_IMPL(ZActor, ZMessageResolver, m_msgCanPenetrate, 0x0097BD74, { "CanPenetrate" });
    STATIC_CLASS_VAR_IMPL(ZActor, ZMessageResolver, m_msgPathRequest, 0x0097BD80, { "PATHREQUEST" });
    STATIC_CLASS_VAR_IMPL(ZActor, ZMessageResolver, m_msgPathFinished, 0x0097BD8C, { "PATHFINISHED" });
    STATIC_CLASS_VAR_IMPL(ZActor, ZMessageResolver, m_msgPathCanceled, 0x0097BD98, { "PATHCANCELED" });
    STATIC_CLASS_VAR_IMPL(ZActor, ZMessageResolver, m_msgPathCanceledLockedDoor, 0x0097BDA4, { "PATHCANCELEDLOCKEDDOOR" });
    STATIC_CLASS_VAR_IMPL(ZActor, ZMessageResolver, m_msgPathError, 0x0097BDB0, { "PATHERROR" });
    STATIC_CLASS_VAR_IMPL(ZActor, ZMessageResolver, m_msgPathOpenDoor, 0x0097BDBC, { "PATHOPENDOOR" });
    STATIC_CLASS_VAR_IMPL(ZActor, ZMessageResolver, m_msgEnterCamera, 0x0097BDC8, { "CAM_ENTERCAMERA" });
    STATIC_CLASS_VAR_IMPL(ZActor, ZMessageResolver, m_msgPathNotify, 0x0097BDD4, { "PATHNOTIFY" });
    STATIC_CLASS_VAR_IMPL(ZActor, ZMessageResolver, m_msgEnterView, 0x0097BDE0, { "EnterView" });
    STATIC_CLASS_VAR_IMPL(ZActor, ZMessageResolver, m_msgLeaveView, 0x0097BDEC, { "LeaveView" });
#   pragma endregion
}
