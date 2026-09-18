#include <Glacier/IK/ZIKLNKOBJ.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/Materials/BS_Runtime.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/Animation/Model.h>
#include <Glacier/Animation/Header.h>
#include <Glacier/Animation/ZAnimVariationHandle.h>
#include <Glacier/Com/CCom.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Data/ZGameData.h>
#include <Glacier/Geom/ZParticleTemplate.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZLIST.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/IK/ZBoneModifyBase.h>
#include <Glacier/IK/ZLnkAction.h>
#include <Glacier/Render/Prim/SBoneDefinition.h>
#include <Glacier/Physics/ZCollisionBase.h>
#include <Glacier/Physics/SExtendedImpactInfo.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniAssert.h>
#include <cstring>


namespace Glacier
{
    namespace
    {
        struct ZGeomRefConverter final : ISerializerStream::IREFConverter
        {
            explicit ZGeomRefConverter(ZGEOM*& pGeom) : m_pGeom(pGeom) {}

            ZREF GetRef() override { return m_pGeom ? m_pGeom->GetRef() : 0; }
            void SetRef(ZREF ref) override { m_pGeom = ZGEOM::RefToPtr(ref); }

            ZGEOM*& m_pGeom;
        };
    }

    // ZIKLNKOBJ
    ZIKLNKOBJ::ZIKLNKOBJ(const char* psName, ZBaseGeom* pBaseGeom)
        : ZLNKOBJ(psName, pBaseGeom)
        , m_Active(0)
        , m_bRunning(false)
        , m_bFacingDisabled(false)
        , m_pFootDustTemplate(nullptr)
        , m_pFootPrints(nullptr)
        , m_BodyFacingTarget()
        , m_fHeadTargetPrc(0.0f)
        , m_fHeadHeight(160.0f)
        , m_StdActionDisplatcher(this)
    {
        ZIKLNKOBJ::ContactDepris = BS_Runtime::ZMaterialDescriptionDB::Instance().GetScenePropertyId("ContactDust");
        m_vHeadTarget = {};
    }

    ZIKLNKOBJ::~ZIKLNKOBJ()
    {
        if (m_pActionDispatcher != &m_StdActionDisplatcher)
        {
            ZUniMemory::Delete(m_pActionDispatcher);
        }

        m_pActionDispatcher = nullptr;
    }

    void ZIKLNKOBJ::LoadSave(ISerializerStream& stream, bool bSaving)
    {
        ZLNKOBJ::LoadSave(stream, bSaving);

        stream.Exchange("m_Active", reinterpret_cast<int32_t&>(m_Active));
        stream.Exchange("m_bRunning", m_bRunning);
        stream.Exchange("m_bFacingDisabled", m_bFacingDisabled);
        stream.Exchange("m_lPushedActive", m_lPushedActive);

        ZGeomRefConverter footDust(m_pFootDustTemplate);
        stream.ExchangeREF(stream.GetToken("m_pFootDustTemplate"), &footDust);
        ZGeomRefConverter footPrints(m_pFootPrints);
        stream.ExchangeREF(stream.GetToken("m_pFootPrints"), &footPrints);

        ZASSERT(!m_BodyFacingTarget.m_CallBack.m_pCallback);
        m_BodyFacingTarget.LoadSave(this, stream, bSaving);
        stream.ExchangeArray("m_vHeadTarget", m_vHeadTarget, 3);
        stream.Exchange("m_fHeadTargetPrc", m_fHeadTargetPrc);
        stream.Exchange("m_fHeadHeight", m_fHeadHeight);
        m_pActionDispatcher->LoadSave(stream, bSaving);
    }

    const RTP::ZPropertyInfo& ZIKLNKOBJ::GetProperties() const
    {
        return ZIKLNKOBJ::Info;
    }

    uint32_t ZIKLNKOBJ::GetObjectId() const
    {
        return ZIKLNKOBJ::m_Id;
    }

    void ZIKLNKOBJ::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZIKLNKOBJ::m_Id;
        mask = ZIKLNKOBJ::m_Mask;
    }

    ZGEOMCLASSINFO* ZIKLNKOBJ::GetOldClassInfo() const
    {
        return ZIKLNKOBJ::m_OldClassInfo;
    }

    int ZIKLNKOBJ::AnimCallBackToId(ActiveAnimCB pCallback)
    {
        const auto callback = reinterpret_cast<ActiveAnimCB>(&ZIKLNKOBJ::CallBackLnkActionBone);
        return pCallback == callback ? 1 : ZLNKOBJ::AnimCallBackToId(pCallback);
    }

    ZIKLNKOBJ::ActiveAnimCB ZIKLNKOBJ::AnimCallBackFromId(int lCallbackId)
    {
        if (lCallbackId == 1)
        {
            return reinterpret_cast<ActiveAnimCB>(&ZIKLNKOBJ::CallBackLnkActionBone);
        }
        return ZLNKOBJ::AnimCallBackFromId(lCallbackId);
    }

    void ZIKLNKOBJ::ClassInit()
    {
        ZGROUP* parent = BaseGeom()->ParentGroup();
        ZBaseGeom* geom = parent->BaseGeom();
        while (geom)
        {
            geom->SetControl(0, 0x2001Du);
            parent->RecurGetNext(&geom);
        }

        const ZREF self = GetRef();
        ZLNKOBJ::ClassInit();
        if (!ZGEOM::RefToPtr(self))
            return;

        ZMat3x3 objectMat, parentMat;
        ZVector3 objectPos, parentPos;
        GetMatPos(objectMat, objectPos);
        parent = BaseGeom()->ParentGroup();
        parent->GetMatPos(parentMat, parentPos);
        TransformRootVector(objectPos, parentMat);
        parentMat *= objectMat;
        parentPos += objectPos;
        objectMat.Reset();
        objectPos.Reset();
        SetMatPos(objectMat, objectPos);
        parent->SetMatPos(parentMat, parentPos);

        ZLIST* trackList = ZLIST::m_TrackLinkObjectsInstance;
        if (!trackList)
        {
            trackList = static_cast<ZLIST*>(g_pEngineData->m_pRoot->CreateGeom("TrackLinkObjects", ZLIST::m_TypeId, true));
            trackList->AddEvent("ZLIST_RangeNotify");
            trackList->AddEvent("ZLIST_SelectNotify");
            trackList->AddEvent("ZLIST_CheckVisible");
            trackList->AddEvent("ZLIST_ActionController");
            trackList->DoInit();
        }
        g_pGameData->SetTrackLinkObjectList(trackList);
        trackList->AddGeom(this);

        ZVector3 headPos;
        GetIKBonePos(HeadBoneIndex(), headPos);
        m_fHeadHeight = headPos.y + 40.0f;
        CreateActionDispatcher();
    }

    void ZIKLNKOBJ::ClassInit2()
    {
        ZLNKOBJ::ClassInit2();
        CalcAnimRemapNames();
    }

    void ZIKLNKOBJ::ClassFrameUpdate()
    {
        UpdateTargets();
        ZLNKOBJ::ClassFrameUpdate();
    }

    void ZIKLNKOBJ::Invisible()
    {
        m_lPushedActive = m_Active;
        m_Active &= ~0x20000u;
        ZLNKOBJ::Invisible();
    }

    void ZIKLNKOBJ::PushState()
    {
        if ((m_lPushedActive & 0x20000) != 0)
            EnableIK();
        else
            DisableIK();
        ZLNKOBJ::PushState();
    }

    void ZIKLNKOBJ::GetDefaultBones(ZBone *pBones, uint32_t lFirstBoneNum) const
    {
        const int32_t pelvis = PelvisBoneIndex();
        if (pelvis == -1 || lFirstBoneNum != 0)
        {
            ZLNKOBJ::GetDefaultBones(pBones, lFirstBoneNum);
            return;
        }
        pBones[0]._Mat.Reset();
        pBones[0]._Pos.Reset();
        ZASSERT(pelvis == 1);
        const auto* translation = reinterpret_cast<const int8_t*>(&m_pMotions2[0].m_PackedQuat);
        pBones[pelvis]._Pos = { translation[0] * 2.0f, translation[1] * 2.0f, translation[2] * 2.0f };
        const auto* packed = reinterpret_cast<const int8_t*>(&m_pMotions2[pelvis].m_PackedQuat);
        ZQuat quat { packed[0] / 112.0f, packed[1] / 112.0f, packed[2] / 112.0f, packed[3] / 112.0f };
        qnorm(quat);
        quattomat(pBones[pelvis]._Mat, quat);
        if (m_Model->m_BlendBones)
        {
            m_Model->m_BlendBones[pelvis].m_Quat = quat;
            m_Model->m_BlendBones[pelvis].m_Pos = pBones[pelvis]._Pos;
        }
        ZLNKOBJ::GetDefaultBones(pBones, 2);
    }

    void ZIKLNKOBJ::SetDefaultBones(const ZBone* pBones, const SBoneDefinition* pDef)
    {
        const int32_t pelvis = PelvisBoneIndex();
        ZLNKOBJ::SetDefaultBones(pBones, pDef);
        if (pelvis != -1)
        {
            auto* translation = reinterpret_cast<int8_t*>(&m_pMotions2[0].m_PackedQuat);
            translation[0] = static_cast<int8_t>(pBones[pelvis]._Pos.x * 0.5f);
            translation[1] = static_cast<int8_t>(pBones[pelvis]._Pos.y * 0.5f);
            translation[2] = static_cast<int8_t>(pBones[pelvis]._Pos.z * 0.5f);
        }
    }

    void ZIKLNKOBJ::CopyGeometryFrom(ZREF rGeom)
    {
        ZLNKOBJ::CopyGeometryFrom(rGeom);
        UpdateGeometry(true);
    }

    void ZIKLNKOBJ::CalcShadowProjectPlane(float* vTans, const float* mObjectToLight, const float* pObjectToLight) const
    {
        if (!m_pBoneModify->CalcShadowProjectPlane(this, vTans, mObjectToLight, pObjectToLight))
            ZLNKOBJ::CalcShadowProjectPlane(vTans, mObjectToLight, pObjectToLight);
    }

    void ZIKLNKOBJ::AnimEnd(Animation::ActiveAnimation* pAnim, int lControl)
    {
        ZLNKOBJ::AnimEnd(pAnim, lControl);
        if (m_pActionDispatcher)
            m_pActionDispatcher->AnimEnd(pAnim);
    }

    bool ZIKLNKOBJ::ActivateRagdoll(bool bActive, bool bEnableTimeout, bool bUseDamping)
    {
        const bool result = m_pBoneModify->ActivateRagdoll(this, bActive, bEnableTimeout, bUseDamping);
        if (result)
            StopAllAnims(false);
        return result;
    }

    void ZIKLNKOBJ::CalcAnimRemapNames()
    {
        // Intentionally empty in the PC vtable.
    }

    void ZIKLNKOBJ::EnableIK()
    {
        m_Active |= 0x20000u;
    }

    void ZIKLNKOBJ::DisableIK()
    {
        m_Active &= ~0x20000u;
    }

    void ZIKLNKOBJ::DisableControls()
    {
        // Intentionally empty in the PC vtable.
    }

    void ZIKLNKOBJ::EnableControls()
    {
        // Intentionally empty in the PC vtable.
    }

    void ZIKLNKOBJ::ForceFacing(const ZVector3& vFacing)
    {
        ZVector3 root;
        GetRootPoint(root);

        ZVector3 direction = root - vFacing;
        direction.y = 0.0f;

        ZMat3x3 facing;
        createmat(facing, direction, nullptr);
        MoveToMatPos(facing, root);
    }

    void ZIKLNKOBJ::GetFocalPoint(ZVector3& vPoint)
    {
        vPoint = { 0.0f, m_fHeadHeight, 0.0f };
        GetRootPoint(vPoint);
    }

    void ZIKLNKOBJ::Reset()
    {
        RemoveCurrentLnkAction();
        ResetTargets();
        m_BodyFacingTarget.Clear();
        m_Active = 0;
    }

    void ZIKLNKOBJ::GetRootCenter(ZMat3x3& mMat, ZVector3& vPos) const
    {
        ZVector3 bonePos;
        GetIKBoneMatPos(0, mMat, bonePos);
        vPos = bonePos + ZVector3(0.0f, 115.0f, 0.0f);

        mMat.YAxis() = { 0.0f, 1.0f, 0.0f };
        vcross(mMat.XAxis(), mMat.ZAxis(), mMat.YAxis());
        vnorm(mMat.XAxis());
        vcross(mMat.ZAxis(), mMat.YAxis(), mMat.XAxis());
        vnorm(mMat.ZAxis());
        GetRootMatPos(mMat, vPos);
    }

    ZIKLNKOBJ::SIKBoneCollision ZIKLNKOBJ::GetBoneCollision(const ZVector3& vPoint, const ZVector3& vDirection)
    {
        ZVector3 localPoint = vPoint;
        GetLocalPoint(localPoint);

        SIKBoneCollision collision{};
        const int32_t boneNr = GetBoneFromPoint(localPoint);
        if (boneNr != -1)
            collision.cBodyPart = GetBoneDefinitions()[boneNr].BodyPart;

        ZVector3 forward = Mat().ZAxis();
        GetRootVect(forward);
        collision.bFront = vdot(forward, vDirection) >= 0.0f;
        return collision;
    }

    float ZIKLNKOBJ::GetOwnerMoveSpeed() const
    {
        CCom com;
        ZGROUP* owner = const_cast<ZIKLNKOBJ*>(this)->GetOwner(false);
        const ZMSGID msg = g_pEngineData->RegisterZMsg("MSG_CANMOVE", 0, __FILE__, __LINE__);
        owner->SendCommand(msg, &com, nullptr);

        float speed = 0.0f;
        com.GetVal("fcmpersecond", &speed);
        return speed;
    }

    bool ZIKLNKOBJ::GetIKBoneMatPos(int lBoneNr, ZMat3x3& mMat, ZVector3& vPos) const
    {
        return m_pBoneModify->GetIKBoneMatPos(mMat, vPos, static_cast<uint8_t>(lBoneNr), this, nullptr);
    }

    bool ZIKLNKOBJ::GetIKBoneMat(int lBoneNr, ZMat3x3& mMat) const
    {
        ZVector3 pos;
        return m_pBoneModify->GetIKBoneMatPos(mMat, pos, static_cast<uint8_t>(lBoneNr), this, nullptr);
    }

    bool ZIKLNKOBJ::GetIKBonePos(int lBoneNr, ZVector3& vPos) const
    {
        ZMat3x3 mat;
        return m_pBoneModify->GetIKBoneMatPos(mat, vPos, static_cast<uint8_t>(lBoneNr), this, nullptr);
    }

    void ZIKLNKOBJ::SetHeadTarget(const ZVector3& vPos, float fSpeedScaleFactor)
    {
        m_vHeadTarget = vPos;
        m_fHeadTargetPrc += g_pSysInterface->DeltaFrameTime * 8.0f;
        if (m_fHeadTargetPrc > 1.0f)
            m_fHeadTargetPrc = 1.0f;

        ZVector3 rootTarget = vPos;
        GetRootPoint(rootTarget);

        Animation::IKTarget& target = m_Model->m_Targets[0];
        target.m_Pos2 = rootTarget;
        target.m_LookAt2.m_Mode = 34;
        target.m_LookAt2.m_Speed = fSpeedScaleFactor * 5.0f;
        target.m_Weight2 += g_pSysInterface->DeltaFrameTime * 2.0f;
        if (target.m_Weight2 > 1.0f)
            target.m_Weight2 = 1.0f;
    }

    void ZIKLNKOBJ::ResetHeadTarget()
    {
        m_Model->m_Targets[0].m_Weight2 = 0.0f;
    }

    int32_t ZIKLNKOBJ::HeadBoneIndex() const
    {
        return GetBoneNrFromId(7);
    }

    int32_t ZIKLNKOBJ::NeckBoneIndex() const
    {
        return GetBoneNrFromId(6);
    }

    int32_t ZIKLNKOBJ::PelvisBoneIndex() const
    {
        return GetBoneNrFromId(2);
    }

    int32_t ZIKLNKOBJ::LHandBoneIndex() const
    {
        return GetBoneNrFromId(20);
    }

    int32_t ZIKLNKOBJ::RHandBoneIndex() const
    {
        return GetBoneNrFromId(25);
    }

    int32_t ZIKLNKOBJ::LToeBoneIndex() const
    {
        return GetBoneNrFromId(12);
    }

    int32_t ZIKLNKOBJ::RToeBoneIndex() const
    {
        return GetBoneNrFromId(16);
    }

    int32_t ZIKLNKOBJ::IKCallBackToId(ZIKCALLBACK* pCallBack)
    {
        const ZIKCALLBACK* expected = IKCallBackFromId(1);
        return pCallBack && std::memcmp(pCallBack, expected, sizeof(*pCallBack)) == 0 ? 1 : -1;
    }

    ZIKCALLBACK* ZIKLNKOBJ::IKCallBackFromId(int lCallBack)
    {
        static ZIKCALLBACK callback = [] {
            ZIKCALLBACK value{};
            const auto member = &ZIKLNKOBJ::CallBackLnkActionTarget;
            std::memcpy(&value, &member, sizeof(member));
            return value;
        }();
        static ZIKCALLBACK invalid = [] { ZIKCALLBACK value{}; value.lCallBackId = -1; return value; }();
        return lCallBack == 1 ? &callback : &invalid;
    }

    void ZIKLNKOBJ::SetFacingTarget(ZREF rGeom, float fTime, ZIKCALLBACK cb)
    {
        if (m_bFacingDisabled || (!m_BodyFacingTarget.Removing() && m_BodyFacingTarget.m_rGeom == rGeom))
            return;

        m_BodyFacingTarget.SetMatPosGeom(nullptr, nullptr, rGeom);
        m_BodyFacingTarget.SetCallBack(cb);
        if ((m_Active & 0x200000u) == 0 || m_BodyFacingTarget.Removing())
            m_BodyFacingTarget.SetTime(fTime, false);
        m_Active |= 0x200000u;
    }

    void ZIKLNKOBJ::RemoveFacingTarget(float fTime, ZIKCALLBACK callback)
    {
        if (m_bFacingDisabled)
            return;

        m_BodyFacingTarget.CallBackAndRemove(this);
        m_BodyFacingTarget.SetCallBack(callback);
        m_Active &= ~0x200000u;
        m_BodyFacingTarget.SetTime(0.0f, true);
        UpdateFacing();
    }

    void ZIKLNKOBJ::DisableFacing()
    {
        RemoveFacingTarget(0.0f, {});
        m_bFacingDisabled = true;
    }

    void ZIKLNKOBJ::EnableFacing()
    {
        m_bFacingDisabled = false;
    }

    void ZIKLNKOBJ::CreateActionDispatcher()
    {
        m_pActionDispatcher = &m_StdActionDisplatcher;
    }

    bool ZIKLNKOBJ::RunLnkAction(ZLnkAction* pAction)
    {
        ZASSERT(pAction);
        ZASSERT(!m_pActionDispatcher->m_pCurrentAction);
        return m_pActionDispatcher->Execute(pAction);
    }

    void ZIKLNKOBJ::RemoveCurrentLnkAction()
    {
        m_pActionDispatcher->DropAction();
    }

    void ZIKLNKOBJ::UpdateCurrentLnkAction()
    {
        m_pActionDispatcher->Update();
    }

    void ZIKLNKOBJ::CallBackLnkActionTarget()
    {
        m_pActionDispatcher->CallBack();
    }

    bool ZIKLNKOBJ::CallBackLnkActionBone(Animation::ActiveAnimation*, float, float, ZREF)
    {
        m_pActionDispatcher->CallBack();
        return true;
    }

    ZLnkAction* ZIKLNKOBJ::CreateLnkAction(uint32_t lActionId)
    {
        // Intentionally returns null in the PC vtable.
        return nullptr;
    }

    uint32_t ZIKLNKOBJ::CurrentLnkActionId() const
    {
        const ZLnkAction* action = GetCurrentLnkAction();
        return action ? action->GetActionId() : 0;
    }

    ZLnkAction* ZIKLNKOBJ::GetCurrentLnkAction() const
    {
        return m_pActionDispatcher ? m_pActionDispatcher->m_pCurrentAction : nullptr;
    }

    bool ZIKLNKOBJ::DisplayGround(bool bDisp)
    {
        return DisplayBone(0, bDisp);
    }

    void ZIKLNKOBJ::EmitFootDustParticle(float fTime, int lBoneIndex)
    {
        if (g_pGameData->m_bDisableDust || (BaseGeom()->Control() & 0x1000) == 0 || m_pBoneModify->m_fVisibleDistanceFromCamera > 2000.0f)
            return;

        auto* particleTemplate = geom_cast<ZParticleTemplate>(m_pFootDustTemplate);
        if (!particleTemplate)
            return;

        ZREF controllerRef = 0;
        uint32_t controllerIndex = 0;
        particleTemplate->GetController(&controllerRef, &controllerIndex);
        ZGEOM* controller = ZGEOM::RefToPtr(controllerRef);
        if (!controller)
            return;

        ZVector3 position;
        if (lBoneIndex < 0)
            position.y = 0.0f;
        else
            GetIKBonePos(lBoneIndex, position);
        position.x = 0.0f;
        position.z = 0.0f;
        GetRootPoint(position);

        ZVector3 velocity {
            (g_pSysInterface->FRand(const_cast<char*>(__FILE__), __LINE__) - 0.5f) * 200.0f,
            5.0f,
            (g_pSysInterface->FRand(const_cast<char*>(__FILE__), __LINE__) - 0.5f) * 200.0f
        };

        using CreateParticle = void(__thiscall*)(ZGEOM*, uint32_t, const ZVector3*, const ZVector3*, int32_t);
        auto function = reinterpret_cast<CreateParticle>((*reinterpret_cast<void***>(controller))[119]);
        function(controller, controllerIndex, &position, &velocity, static_cast<int32_t>(fTime * 1024.0f));
    }

    void ZIKLNKOBJ::MakeFootPrint(bool bMake)
    {
        // Intentionally empty in the PC vtable.
    }

    void ZIKLNKOBJ::UpdateHead()
    {
        if (!m_bCutSequence)
        {
            m_pBoneModify->m_fHeadTimePrc = m_fHeadTargetPrc;
            m_pBoneModify->m_vHeadTarget = m_vHeadTarget;
        }

        m_fHeadTargetPrc -= g_pSysInterface->DeltaFrameTime * 4.0f;
        if (m_fHeadTargetPrc < 0.0f)
            m_fHeadTargetPrc = 0.0f;
    }

    void ZIKLNKOBJ::UpdateFacing()
    {
        if ((m_Active & 0x200000u) == 0)
            return;

        ZGEOM* target = ZGEOM::RefToPtr(m_BodyFacingTarget.m_rGeom);
        if (!target || GetGroundAnimation())
            return;

        ZVector3 targetPos;
        target->GetRootPoint(targetPos);
        GetLocalPoint(targetPos);
        targetPos.y = 0.0f;
        vnorm(targetPos);

        const float facing = -targetPos.z;
        if (facing >= 0.5f)
            return;

        const char* animationName = facing < -0.5f
            ? "/Turn/Left_180_Quick"
            : "/Turn/Left_90_Quick";
        const int control = -targetPos.x >= 0.0f ? 0x8001 : 1;
        const ZAnimVariationHandle handle = GetAnimVariationHandle(animationName);
        Animation::Header* header = GetAnimHeaderFromVariation(
            handle,
            m_iAnimVariationFlags,
            g_pSysInterface->FRand(const_cast<char*>(__FILE__), __LINE__));
        if (header)
            ActivateAnim(header, control);
    }

    void ZIKLNKOBJ::UpdateTargets()
    {
        UpdateFacing();
        UpdateHead();
    }

    void ZIKLNKOBJ::ResetTargets()
    {
        // Intentionally empty in the PC vtable.
    }

    void ZIKLNKOBJ::RemoveTargets()
    {
        // Intentionally empty in the PC vtable.
    }

    bool ZIKLNKOBJ::CanPlayAnimSegment(Animation::Header* pAnimHeader, float fFrom, float fTo, bool bMirror)
    {
        ZMat3x3 rootMat;
        ZVector3 rootPos;
        rootMat.Reset();
        rootPos.Reset();
        GetRootMatPos(rootMat, rootPos);
        return CanPlayAnimSegment(pAnimHeader, fFrom, fTo, rootMat, rootPos, bMirror, 50.0f, -130.0f);
    }

    bool ZIKLNKOBJ::CanPlayAnimSegment(Animation::Header* pAnimHeader, float fFrom, float fTo, const float* pRootMat, const float* pRootPos, bool bMirror, float, float)
    {
        ZCollisionBase* collision = ZCollisionBase::GetCollisionInterface();
        if (collision->IsCollisionBoxLocked())
            return false;

        ZVector3 movement(pAnimHeader->m_CycleDist);
        if (bMirror)
            movement.x = -movement.x;
        float groundQuat[4];
        ZVector3 groundMovement;
        pAnimHeader->GetGround(g_pEngineData->m_AnimationManager, (fTo - fFrom) * 0.5f, groundQuat, groundMovement, bMirror);
        if (vlen2(movement) < vlen2(groundMovement))
            movement = groundMovement;
        TransformRootVector(movement, *reinterpret_cast<const ZMat3x3*>(pRootMat));
        movement.y = 0.0f;
        const float movementLength = vlen(movement);
        if (movementLength < 0.01f)
            return true;

        const float pathLength = movementLength + 30.0f;
        vsetlen(movement, pathLength);
        const int stepCount = static_cast<int>(pathLength * Animation::Header::TIME_SCALE + 1.0f);
        ZVector3 step = movement * (1.0f / static_cast<float>(stepCount));
        ZMat3x3 boxMat;
        createmat(boxMat, movement, nullptr);
        ZVector3 boxPos = ZVector3(pRootPos) + movement * 0.5f;
        boxPos.y += 90.0f;
        ZVector3 boxSize(30.0f, static_cast<float>(25 * stepCount + 90), pathLength * 0.5f);

        char stripsBuffer[98304];
        ZCollisionBox* box = collision->LockCollisionBox(stripsBuffer, sizeof(stripsBuffer));
        box->SetBox(boxMat, boxPos, boxSize);
        box->GetStrips(0x20u);
        const ZVector3 down(0.0f, -50.0f, 0.0f);
        ZVector3 current(pRootPos);
        current.y += 25.0f;
        const ZVector3 start(current);
        float maxY = current.y;
        for (int i = 0; i <= stepCount; ++i)
        {
            SExtendedImpactInfo impact;
            if ((std::fabs(current.y - start.y) > 25.0f || i == stepCount)
                && box->CalcLineCollision(&impact, start, current - start, false))
            {
                collision->UnlockCollisionBox(box);
                return false;
            }
            if (!box->CalcLineCollision(&impact, current, down, false))
            {
                collision->UnlockCollisionBox(box);
                return false;
            }
            current.y = impact.vPosition.y + 25.0f;
            maxY = std::max(maxY, current.y);
            current += step;
        }
        boxPos.y = maxY + 115.0f;
        boxSize.y = 65.0f;
        const bool blocked = box->CheckBoxCollision(boxMat, boxPos, boxSize);
        collision->UnlockCollisionBox(box);
        return !blocked;
    }

#   pragma region " --- RTTI --- "
    DECLARE_GEOM_CLASS_IMPL(
        ZIKLNKOBJ,
        ZLNKOBJ,
        0x0097BF00,
        "ZIKLNKOBJ",
        0x007719C0,
        nullptr,
        0x00806C94,
        0x0097BEB0,
        0x0097BEB4
    );
#   pragma endregion

    STATIC_CLASS_VAR_IMPL(ZIKLNKOBJ, TScenePropertyID, ContactDepris, 0x0097BF04, {});
}
