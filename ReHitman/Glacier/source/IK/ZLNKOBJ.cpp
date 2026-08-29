#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/IK/ZBoneModifyBase.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Animation/ZAnimVariationHandle.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/Prim/SBoneDefinition.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/Runtime/Macro.h>
#include <Glacier/Physics/ZCommonAlgorithms.h>
#include <Glacier/ZSTL/CHUNKFILE.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/ZSTL/ZFilePath.h>
#include <cstdint>
#include <cstring>


namespace Glacier
{
    namespace
    {
        constexpr int kAnimVariationsChunk = 6;

        int stricmpend(const char* str, const char* suffix)
        {
            const size_t strLen = std::strlen(str);
            const size_t suffixLen = std::strlen(suffix);

            if (strLen < suffixLen)
                return 1;

            return _stricmp(str + (strLen - suffixLen), suffix);
        }
    }

    ZLNKOBJ::ZLNKOBJ(const char* psName, ZBaseGeom* pBaseGeom)
        : ZSTDOBJ(psName, pBaseGeom)
    {
        // TODO: Finish me
    }

    ZLNKOBJ::~ZLNKOBJ()
    {
        // TODO: Finish me
    }

    const RTP::ZPropertyInfo& ZLNKOBJ::GetProperties() const
    {
        return ZLNKOBJ::Info;
    }

    uint32_t ZLNKOBJ::GetObjectId() const
    {
        return ZLNKOBJ::m_Id;
    }

    void ZLNKOBJ::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZLNKOBJ::m_Id;
        mask = ZLNKOBJ::m_Mask;
    }

    ZGEOMCLASSINFO* ZLNKOBJ::GetOldClassInfo() const
    {
        return ZLNKOBJ::m_OldClassInfo;
    }

    const ZBone* ZLNKOBJ::GetBones() const
    {
        return m_pBoneModify->GetBones(this);
    }

    const ZBone* ZLNKOBJ::GetGlobalPrimBones() const
    {
        // it's ok due ZBone is POD type contains only 'float' entries.
        return reinterpret_cast<const ZBone*>(ZPrimControlBase::Instance()->GetGlobalPrimBones(Prim()));
    }

    const char* ZLNKOBJ::LoadAnimVariationsBuffer(const char* pszFileName)
    {
        auto sConvertedAnimBuffName = g_pSysInterface->ConvertFileName(pszFileName);
        const char* pszAnimBuffFileName = sConvertedAnimBuffName;

        CHUNKFILE* pPackedAnims = g_pEngineData->m_pPackedAnims;
        if (!pPackedAnims)
            return nullptr;

        int nrChunks = 0;
        CHUNKFILE* pChunk = nullptr;

        if (pPackedAnims->m_lTotalSize < 0)
        {
            nrChunks = pPackedAnims->type.Type2.NrChunks;
            pChunk = pPackedAnims->FirstChild();
        }

        if (!nrChunks)
            return nullptr;

        const char* pAnimVariationName = nullptr;

        while (true)
        {
            if (pChunk->Name == kAnimVariationsChunk)
            {
                const char* pName = static_cast<const char*>(pChunk->Data());

                ZFilePath sAnimFileName(pName);
                ZFilePath sRequestedFileName(pszAnimBuffFileName);

                if (stricmpend(sRequestedFileName.AsChar(), sAnimFileName.AsChar()) == 0)
                {
                    pAnimVariationName = pName;
                    break;
                }
            }

            pChunk = pChunk->GetNextSibling();
            if (--nrChunks == 0)
                return nullptr;
        }

        size_t offset = std::strlen(pAnimVariationName) + 1;
        while (offset % 4 != 0)
            ++offset;

        return pAnimVariationName + offset;
    }

    Animation::Model* ZLNKOBJ::Model()
    {
        return m_Model;
    }

    const Animation::Model* ZLNKOBJ::Model() const
    {
        return m_Model;
    }

    void ZLNKOBJ::InitObjMatBone()
    {
        const auto lNrBones = ZPrimControlBase::Instance()->GetNrBones(Prim());
        m_pBoneModify = g_pRenderDll->CreateBoneModifier(lNrBones);
        m_pMotions2 = ZUniMemory::NewArray<ActBoneMotion2>(lNrBones);

        auto* pBones = m_pBoneModify->GetBones(this);
        const auto* pBoneDefs = GetBoneDefinitions();
        SetDefaultBones(pBones, pBoneDefs);
    }

    void ZLNKOBJ::CloseObjMatBone()
    {
        if (m_pMotions2)
        {
            ZUniMemory::Delete(m_pMotions2);
            m_pMotions2 = nullptr;
        }

        if (m_pBoneModify)
        {
            ZUniMemory::Delete(m_pBoneModify);
            m_pBoneModify = nullptr;
        }
    }

    // ZSerializable
    bool ZLNKOBJ::PostLoad(ISerializerStream& stream)
    {
        m_TemplateNames.Init();
        return true;
    }

    void ZLNKOBJ::LoadSave(ISerializerStream& stream, bool bSaving)
    {
        // TODO: Finish me
    }

    // ZGEOM
    void ZLNKOBJ::CalcCenSize()
    {
        // TODO: Finish me
    }

    int ZLNKOBJ::AnimCallBackToId(ZGEOM::ActiveAnimCB pCallback)
    {
        // TODO: Finish me
        return 0;
    }

    ZGEOM::ActiveAnimCB ZLNKOBJ::AnimCallBackFromId(int)
    {
        // TODO: Finish me
        return nullptr;
    }

    bool ZLNKOBJ::WantViewPrimHideUnhideRequest() const
    {
        // TODO: Finish me
        return false;
    }

    bool ZLNKOBJ::WantViewPrimHide(uint8_t lLODControl) const
    {
        // TODO: Finish me
        return false;
    }

    bool ZLNKOBJ::WantViewPrimHideMirrors(uint8_t lLODControl) const
    {
        // TODO: Finish me
        return false;
    }

    uint8_t ZLNKOBJ::GetLODMaskOverride() const
    {
        return m_bUseLODMASK ? m_LODMask : 0u;
    }

    void ZLNKOBJ::ClassInit()
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::ClassFrameUpdate()
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::Invisible()
    {
        SetCutSequence(false);
        ResetAllAnimBones();
    }

    void ZLNKOBJ::PushState()
    {
        SetCutSequence(false);
        ResetAllAnimBones();
    }

    void ZLNKOBJ::CopyData(const ZGEOM* Source)
    {
        ZSTDOBJ::CopyData(Source);

        if (Source->IsDerivedFrom<ZLNKOBJ>())
        {
            auto* pSource = geom_cast<ZLNKOBJ>(Source);

            m_lVariantId = pSource->m_lVariantId;
            m_TemplateNames.Init();
            m_TemplateNames.SetBuffer(pSource->m_AnimVariations.GetBuffer());
            animCollectionName = pSource->animCollectionName;
        }
    }

    bool ZLNKOBJ::IsAttached() const
    {
        return m_pAttachedTo != nullptr;
    }

    void ZLNKOBJ::SetAttachedTo(ZLNKOBJ* pAttachedTo)
    {
        m_pAttachedTo = pAttachedTo;
    }

    ZLNKOBJ* ZLNKOBJ::GetAttachedTo() const
    {
        return m_pAttachedTo;
    }

    // ZLNKOBJ
    void* ZLNKOBJ::GetAnim(const char*)
    {
        // TODO: Finish me
        return nullptr;
    }

    void ZLNKOBJ::StopAllAnims(bool)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::StopUBAnims()
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::StopAnim(Animation::ActiveAnimation*)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::ActivateAnimSegment(Animation::Header*, int, float, float, float)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::ActivateAnimSegment(ZAnimVariationHandle, int, float, float, float)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::ActivateAnim(Animation::Header*, int)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::SetActiveAnimDestination(float const*, float const*, float)
    {
        // TODO: Finish me
    }

    bool ZLNKOBJ::CheckActiveAnim(Animation::Header*, int)
    {
        // TODO: Finish me
        return false;
    }

    void ZLNKOBJ::SetBoneFrameBlend(Animation::Header*, float, float, bool, unsigned int)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::SetDualFrame(Animation::Header*, float, Animation::Header*, float, float, float)
    {
        // TODO: Finish me
    }

    bool ZLNKOBJ::GroundAnimated()
    {
        // TODO: Finish me
        return false;
    }

    void ZLNKOBJ::OnMetaKey(Animation::ActiveAnimation*, Animation::ZMetaKey*, char const*)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::ActivatePoseAnim(char*, float, unsigned int, float)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::StopPoseAnim(unsigned int, bool)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::StopAudio()
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::StopAnimSound(bool, int, bool)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::StopAllAnimSounds(void)
    {
        // TODO: Finish me
    }

    int ZLNKOBJ::CheckLineCollision(float* pResult, const float* pLineStart, const float* pLineDirection) const
    {
        if (!m_pBoneModify || m_pBoneModify->m_lNumActiveBones <= 1)
            return 0;

        const auto* pBones = GetBones();
        const auto* pBoneDefinitions = GetBoneDefinitions();
        if (!pBones || !pBoneDefinitions)
            return 0;

        int bestBoneId = 0;
        float bestT = *pResult;
        for (uint32_t i = 1; i < m_pBoneModify->m_lNumActiveBones; ++i)
        {
            const auto& definition = pBoneDefinitions[i];
            if (definition.Size.x == 0.0f && definition.Size.y == 0.0f && definition.Size.z == 0.0f)
                continue;

            ZVector3 boneCenter;
            vmmul(boneCenter, definition.Center, pBones[i]._Mat);
            boneCenter += pBones[i]._Pos;

            ZVector3 localStart;
            vsub(localStart, pLineStart, boneCenter);
            vmtmul(localStart, pBones[i]._Mat.data);

            ZVector3 localDirection { pLineDirection };
            vmtmul(localDirection, pBones[i]._Mat.data);

            float hitT = bestT;
            if (!ZCommonAlgorithms::LineVS_AABB(
                    -definition.Size.x, -definition.Size.y, -definition.Size.z,
                    definition.Size.x, definition.Size.y, definition.Size.z,
                    localStart.x, localStart.y, localStart.z,
                    localDirection.x, localDirection.y, localDirection.z,
                    &hitT) || hitT >= bestT)
            {
                continue;
            }

            bestT = hitT;
            bestBoneId = definition.Id;
        }

        if (!bestBoneId)
            return 0;

        *pResult = bestT;
        return bestBoneId;
    }

    int ZLNKOBJ::CheckLineCollision(float* pResult, const float* pLineStart, const float* pLineDirection, float* pUnused) const
    {
        (void)pUnused;
        return CheckLineCollision(pResult, pLineStart, pLineDirection);
    }

    void ZLNKOBJ::CheckBoxCollision(float const*, float const*, float const*)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::Use2Skeletons(void)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::LocalStateIK(void)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::GetDefaultBones(ZBone* pBones, uint32_t lFirstBoneNum) const
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::SetDefaultBones(ZBone const*, SBoneDefinition const*)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::GetAnimDeltaBones(Animation::Header*)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::GetBoneMatPos(Glacier::ZMat3x3*, Glacier::ZVector3*, unsigned int)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::AttachBaseGeomToBone(ZBaseGeom const*, unsigned int, float const*, float const*)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::DetachBaseGeomFromBone(ZBaseGeom const*, unsigned int)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::GetAttachedBaseGeomBoneId(ZBaseGeom const*)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::GetGroundBoneAnimMatPos(ZMat3x3*, ZVector3*, Animation::Header*, float, bool)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::GetGroundBoneDeltaMatPos(ZMat3x3*, ZVector3*, Animation::Header*, float, float)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::GetAttachedGeomMatPos(ZBaseGeom const*, float*, float*)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::DisplayBone(unsigned int, bool)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::GetFocusMatPos(float*, float*)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::DisplayAllBones(bool)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::GetRootFocusMatPos(float*, float*)
    {
        // TODO: Finish me
    }

    uint16_t ZLNKOBJ::GetBoneNrFromId(uint8_t lBoneId) const
    {
        // TODO: Finish me
        return 0;
    }

    void ZLNKOBJ::GetBoneFromPoint(float const*)
    {
        // TODO: Finish me
    }

    uint16_t ZLNKOBJ::GetBoneNrFromName(char const* pszName) const
    {
        // TODO: Finish me
        return 0;
    }

    const char* ZLNKOBJ::GetBoneName(int)
    {
        // TODO: Finish me
        return nullptr;
    }

    void ZLNKOBJ::GetOrigLocalBones(void)
    {
        // TODO: Finish me
    }

    const SBoneDefinition* ZLNKOBJ::GetBoneDefinitions() const
    {
        // TODO: Finish me
        return nullptr;
    }

    void ZLNKOBJ::CopyGeometryFrom(ZGEOM*)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::CopyGeometryFrom(unsigned int)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::CopyPoseFrom(ZLNKOBJ*)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::ChangeMesh(ZGROUP*)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::PrintAllBoneNames(void)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::SetCutSequence(bool)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::MoveToMatPos(float const*, float const*)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::SetRootTMParent(float*, float*)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::EventCallBack(Animation::ActiveAnimation*, float, float, void*)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::OnMoving(void)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::OnMoved(void)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::CalcTightCenSize(float*, float*)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::CalcShadowProjectPlane(float*, float const*, float const*)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::GetBoneControl(int)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::UpdateGeometry(bool)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::ResetAllAnimBones(void)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::ResetInactiveBones(void)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::ExecuteCallBack(char const*)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::GetBoneVolume(int)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::GetBoneSize(int, Glacier::Vector3*)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::GetBoneCenter(int, Glacier::Vector3*)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::AnimEnd(Animation::ActiveAnimation*, int)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::UpdateAnimationsAndGroundLink(float)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::UpdatePoseAnimation()
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::StartAnim(Animation::ActiveAnimation*, int)
    {
        // TODO: Finish me
    }

    bool ZLNKOBJ::IsInElevator() const
    {
        // TODO: Finish me
        return false;
    }

    float ZLNKOBJ::GetElevatorDeltaY() const
    {
        // TODO: Finish me
        return 0.0f;
    }

    bool ZLNKOBJ::WantBloodOnHit()
    {
        // TODO: Finish me
        return false;
    }

    void ZLNKOBJ::LoadSaveAnimations(ISerializerStream& stream, bool bSaving)
    {
        // TODO: Finish me
    }

    void ZLNKOBJ::GetAnimCollectionProperty(ZRTString& anim_collection_name)
    {
        // Do nothing
    }

    void ZLNKOBJ::SetAnimCollectionProperty(const ZRTString& anim_collection_name)
    {
        animCollectionName = anim_collection_name;

        const char* pszAnimCollectionName = anim_collection_name;
        if (pszAnimCollectionName && *pszAnimCollectionName)
        {
            const auto* pBuffer = LoadAnimVariationsBuffer(pszAnimCollectionName);
            m_AnimVariations.SetBuffer(pBuffer);
        }
    }

#   pragma region " --- ZGEOM RTTI --- "
    STATIC_CLASS_VAR_IMPL(ZLNKOBJ, const char*, FactoryName, 0x0077037C, "ZLNKOBJ");
    DECLARE_ID_AND_MASK_IMPL(ZLNKOBJ, 0x0099C488, 0x0099C48C);
    REGISTER_GLACIER_GEOM_CLASS(ZLNKOBJ, ZSTDOBJ, 0x200006u, 0x0099C4F8);
#   pragma endregion

#   pragma region "RTTI"
    namespace cProperties
    {
        // TODO: Finish me
        static RTP::ZDataProperty<int> NamespaceItem_3340 {
            .m_Node = {
                .m_Next = nullptr,
                .m_Name = "m_pMotions2BoneCount",
                .m_Filter = 2
            },
            .m_VirtualTable = VirtualTable_DP__7,
            .m_Offset = CLASS_PROPERTY(ZLNKOBJ, m_pMotions2BoneCount)
        };
    }


    // Entry
    STATIC_CLASS_VAR_IMPL(ZLNKOBJ, RTP::ZPropertyInfo, Info, 0x00806A54, (RTP::ZPropertyInfo {
        .First = cProperties::NamespaceItem_3340,
        .Super = &ZSTDOBJ::Info,
        .Name = ZLNKOBJ::FactoryName
    }));
#   pragma endregion
}
