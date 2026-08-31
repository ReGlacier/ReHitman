#include <Glacier/IK/ZIKLNKOBJ.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/Materials/BS_Runtime.h>
#include <Glacier/Serializer/ISerializerStream.h>


namespace Glacier
{
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

        // TODO: Finish me
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
        // TODO: Finish me
        return 0;
    }

    ZIKLNKOBJ::ActiveAnimCB ZIKLNKOBJ::AnimCallBackFromId(int)
    {
        // TODO: Finish me
        return nullptr;
    }

    void ZIKLNKOBJ::ClassInit()
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::ClassInit2()
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::ClassFrameUpdate()
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::Invisible()
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::PushState()
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::GetDefaultBones(ZBone *pBones, uint32_t lFirstBoneNum) const
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::SetDefaultBones(const ZBone* pBones, const SBoneDefinition* pDef)
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::CopyGeometryFrom(ZREF rGeom)
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::CalcShadowProjectPlane(float* vTans, const float* mObjectToLight, const float* pObjectToLight) const
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::AnimEnd(Animation::ActiveAnimation* pAnim, int lControl)
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::ActivateRagdoll(bool bActive, bool bEnableTimeout, bool bUseDamping)
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::CalcAnimRemapNames()
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::EnableIK()
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::DisableIK()
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::DisableControls()
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::EnableControls()
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::ForceFacing(const ZVector3& vFacing)
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::GetFocalPoint(ZVector3& vPoint)
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::Reset()
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::GetRootCenter(ZMat3x3& mMat, ZVector3& vPos)
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::GetBoneCollision(ZMat3x3& mMat, ZVector3& vPos)
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::GetOwnerMoveSpeed()
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::GetIKBoneMatPos(int lBoneNr, ZMat3x3& mMat, ZVector3& vPos)
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::GetIKBoneMat(int lBoneNr, ZMat3x3& mMat)
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::GetIKBonePos(int lBoneNr, ZVector3& vPos)
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::SetHeadTarget(const ZVector3& vPos, float fSpeedScaleFactor)
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::ResetHeadTarget()
    {
        // TODO: Finish me
    }

    uint32_t ZIKLNKOBJ::HeadBoneIndex()
    {
        // TODO: Finish me
        return 0;
    }

    uint32_t ZIKLNKOBJ::NeckBoneIndex()
    {
        // TODO: Finish me
        return 0;
    }

    uint32_t ZIKLNKOBJ::PelvisBoneIndex()
    {
        // TODO: Finish me
        return 0;
    }

    uint32_t ZIKLNKOBJ::LHandBoneIndex()
    {
        // TODO: Finish me
        return 0;
    }

    uint32_t ZIKLNKOBJ::RHandBoneIndex()
    {
        // TODO: Finish me
        return 0;
    }

    uint32_t ZIKLNKOBJ::LToeBoneIndex()
    {
        // TODO: Finish me
        return 0;
    }

    uint32_t ZIKLNKOBJ::RToeBoneIndex()
    {
        // TODO: Finish me
        return 0;
    }

    int32_t ZIKLNKOBJ::IKCallBackToId(ZIKCALLBACK* pCallBack)
    {
        // TODO: Finish me
        return 0;
    }

    ZIKCALLBACK* ZIKLNKOBJ::IKCallBackFromId(int lCallBack)
    {
        // TODO: Finish me
        return nullptr;
    }

    void ZIKLNKOBJ::SetFacingTarget(ZREF rGeom, float fTime, ZIKCALLBACK cb)
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::RemoveFacingTarget(float fTime, ZIKCALLBACK callback)
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::DisableFacing()
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::EnableFacing()
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::CreateActionDispatcher()
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::RunLnkAction(ZLnkAction* pAction)
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::RemoveCurrentLnkAction()
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::UpdateCurrentLnkAction()
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::CallBackLnkActionTarget()
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::CallBackLnkActionBone(Animation::ActiveAnimation* pZBoneAnim, float fCallBackStartFrame, float fFrame, ZREF rGeomRef)
    {
        // TODO: Finish me
    }

    ZLnkAction* ZIKLNKOBJ::CreateLnkAction(uint32_t lActionId)
    {
        return nullptr;
    }

    uint32_t ZIKLNKOBJ::CurrentLnkActionId() const
    {
        // TODO: Finish me
        return 0;
    }

    ZLnkAction* ZIKLNKOBJ::GetCurrentLnkAction() const
    {
        // TODO: Finish me
        return nullptr;
    }

    void ZIKLNKOBJ::DisplayGround(bool bDisp)
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::EmitFootDustParticle(float fTime, int lBoneIndex)
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::MakeFootPrint(bool bMake)
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::UpdateHead()
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::UpdateFacing()
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::UpdateTargets()
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::ResetTargets()
    {
        // TODO: Finish me
    }

    void ZIKLNKOBJ::RemoveTargets()
    {
        // TODO: Finish me
    }

    bool ZIKLNKOBJ::CanPlayAnimSegment(Animation::Header* pAnimHeader, float fFrom, float fTo, bool bMirror) const
    {
        // TODO: Finish me
        return false;
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
