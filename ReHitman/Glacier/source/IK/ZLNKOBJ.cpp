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
#include <Glacier/Animation/Model.h>
#include <Glacier/Animation/Header.h>
#include <Glacier/Animation/ActiveAnimation.h>
#include <Glacier/Animation/Manager.h>
#include <Glacier/Audio/ZSoundObject.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/Items/ZItem.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZEngineGeomControl.h>
#include <Glacier/ZSTL/ZFilePath.h>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <cstring>


namespace Glacier
{
    namespace
    {
        constexpr int kAnimVariationsChunk = 6;

        ZQuat InverseQuat(const ZQuat& quat)
        {
            return { -quat.i, -quat.j, -quat.k, quat.w };
        }

        ZQuat MultiplyQuat(const ZQuat& lhs, const ZQuat& rhs)
        {
            return {
                lhs.w * rhs.i + lhs.i * rhs.w + lhs.j * rhs.k - lhs.k * rhs.j,
                lhs.w * rhs.j - lhs.i * rhs.k + lhs.j * rhs.w + lhs.k * rhs.i,
                lhs.w * rhs.k + lhs.i * rhs.j - lhs.j * rhs.i + lhs.k * rhs.w,
                lhs.w * rhs.w - lhs.i * rhs.i - lhs.j * rhs.j - lhs.k * rhs.k
            };
        }

        ZDeltaBone ComposeTransform(const ZDeltaBone& lhs, const ZDeltaBone& rhs)
        {
            ZDeltaBone result;
            const auto& lhsQuat = *reinterpret_cast<const ZQuat*>(lhs.m_OffsetQuat);
            const auto& rhsQuat = *reinterpret_cast<const ZQuat*>(rhs.m_OffsetQuat);
            *reinterpret_cast<ZQuat*>(result.m_OffsetQuat) = MultiplyQuat(lhsQuat, rhsQuat);
            qtran(result.m_OffsetPos, lhs.m_OffsetQuat, rhs.m_OffsetPos);
            vadd(result.m_OffsetPos, lhs.m_OffsetPos);
            return result;
        }

        ZDeltaBone InverseTransform(const ZDeltaBone& transform)
        {
            ZDeltaBone result;
            *reinterpret_cast<ZQuat*>(result.m_OffsetQuat) = InverseQuat(*reinterpret_cast<const ZQuat*>(transform.m_OffsetQuat));
            ZVector3 negated { -transform.m_OffsetPos[0], -transform.m_OffsetPos[1], -transform.m_OffsetPos[2] };
            qtran(result.m_OffsetPos, result.m_OffsetQuat, &negated.x);
            return result;
        }

        ZDeltaBone RelativeTransform(const ZDeltaBone& from, const ZDeltaBone& to)
        {
            return ComposeTransform(InverseTransform(from), to);
        }

        Animation::ActiveAnimation::CallBack_t ToAnimationCallback(
            bool (ZLNKOBJ::*callback)(Animation::ActiveAnimation*, float, float, uint32_t))
        {
            Animation::ActiveAnimation::CallBack_t result{};
            static_assert(sizeof(result) == sizeof(callback));
            std::memcpy(&result, &callback, sizeof(result));
            return result;
        }

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
        , m_pPoseAnim(nullptr)
        , m_pMotions2(nullptr)
        , m_pMotions2BoneCount(0)
        , m_lVariantId(0)
        , m_pGameEntity(nullptr)
        , m_AnimSound {}
        , m_SoundMappingMaterial(0)
        , m_iVisionID(0xFF)
        , m_Model(ZUniMemory::New<Animation::Model>())
        , m_iAnimVariationFlags(0)
        , m_pBoneModify(nullptr)
        , m_bCutSequence(false)
        , m_bHideInThisView(false)
        , m_GroundAnimDestStartFrame(-1.0f)
        , m_GroundAnimDestEndFrame(-1.0f)
        , m_fPelvisBoneOffset(0.0f)
        , m_pAttachedTo(0)
        , m_LODMask(0)
        , m_bUseLODMASK(false)
        , m_FramesReset(false)
        , m_bMetaKeyCallBacks(false)
    {
        m_Ground._Mat.Reset();
        m_Ground._Pos.Reset();
        for (auto& animation : m_Model->m_ActiveAnims)
            animation.Create(this);
    }

    ZLNKOBJ::~ZLNKOBJ()
    {
        CloseObjMatBone();
        ZUniMemory::Delete(m_Model);
        m_Model = nullptr;
        ZUniMemory::Delete(m_pPoseAnim);
        m_pPoseAnim = nullptr;
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

    uint32_t ZLNKOBJ::NumActiveBones() const
    {
        return m_pBoneModify ? m_pBoneModify->m_lNumActiveBones : 0;
    }

    Animation::ActiveAnimation* ZLNKOBJ::GetGroundAnimation() const
    {
        if (!m_Model)
            return nullptr;
        for (auto& animation : m_Model->m_ActiveAnims)
        {
            if ((animation.mode & 0x8005) == 0x8001 && animation.header && (animation.header->m_Mask & 0x40))
                return &animation;
        }
        return nullptr;
    }

    Animation::Header* ZLNKOBJ::GetAnimHeaderFromVariation(ZAnimVariationHandle handle, int flags, float random) const
    {
        ZAnimVariation variation;
        if (!m_AnimVariations.GetAnimVariation(variation, handle))
            return nullptr;
        const uint32_t offset = variation.GetAnimOffset(flags, random);
        return offset == static_cast<uint32_t>(-1) ? nullptr : Animation::instance->FromIndex(static_cast<int>(offset));
    }

    bool ZLNKOBJ::MetaKeyCallBack(Animation::ActiveAnimation* pAnimation, float, float, uint32_t metaKeyOffset)
    {
        if (!pAnimation || !pAnimation->header)
            return true;
        auto* pMetaKey = reinterpret_cast<Animation::ZMetaKey*>(
            reinterpret_cast<char*>(Animation::instance->GetMetaKeyData(pAnimation->header->m_MetaDataOffset)) + metaKeyOffset);
        OnMetaKey(pAnimation, pMetaKey, pMetaKey->GetString());
        return true;
    }

    bool ZLNKOBJ::AnimSoundCallback(Animation::ActiveAnimation*, float, float, uint32_t soundRef)
    {
        if (auto* pSound = g_pEngineData->SRefToPtr(soundRef))
        {
            pSound->m_lSoundFlags |= 0x20000u;
            return true;
        }

        return false;
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

    ZAnimVariationHandle ZLNKOBJ::GetAnimVariationHandle(const char* pszName)
    {
        ZAnimVariationHandle hAnim;
        m_TemplateNames.FindAnimVariationHandle(hAnim, pszName);
        return hAnim;
    }

    Animation::ActiveAnimation* ZLNKOBJ::IsAnimationRunning(int hAnim)
    {
        if (!m_Model)
            return nullptr;

        for (auto& animation : m_Model->m_ActiveAnims)
        {
            const int mode = animation.mode & 7;
            if (mode != 0 && mode != 2 && animation.sequenceId == hAnim)
                return &animation;
        }
        return nullptr;
    }

    int ZLNKOBJ::PlayAnimSegment(Animation::Header* pAnimHeader, int32_t dwMode, float fFrom, float fTo, float fSpeed)
    {
        if (pAnimHeader)
        {
            auto* pActiveAnim = ActivateAnimSegment(pAnimHeader, dwMode, fFrom, fTo, fSpeed);
            if (pActiveAnim)
            {
                return pActiveAnim->sequenceId;
            }
        }
        else
        {
            SendCommand(0x803u, &pAnimHeader, 0);
        }

        return 0;
    }

    void ZLNKOBJ::InitObjMatBone()
    {
        const auto lNrBones = ZPrimControlBase::Instance()->GetNrBones(Prim());
        m_pBoneModify = g_pRenderDll->CreateBoneModifier(lNrBones);
        m_pMotions2 = ZUniMemory::NewArray<ActBoneMotion2>(lNrBones);
        std::memset(m_pMotions2, 0, sizeof(ActBoneMotion2) * lNrBones);

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
        ZGEOM::LoadSave(stream, bSaving);
        stream.Exchange("LastUpperBody", reinterpret_cast<int16_t&>(m_Model->m_LastUpperBody));
        stream.Exchange("LastFullBody", reinterpret_cast<int16_t&>(m_Model->m_LastFullBody));
        stream.Exchange("Animated", m_Model->m_Animated);

        if (bSaving && m_pBoneModify)
        {
            const auto* pBones = GetBones();
            if (pBones && pBones != GetGlobalPrimBones())
                SetDefaultBones(pBones, GetBoneDefinitions());
        }

        bool motionsNull = m_pMotions2 == nullptr;
        stream.Exchange("isNull", motionsNull);
        uint32_t boneCount = ZPrimControlBase::Instance()->GetNrBones(Prim());
        stream.Exchange("NumberOfBones", boneCount);
        if (!bSaving && !motionsNull)
        {
            ZASSERT(boneCount == ZPrimControlBase::Instance()->GetNrBones(Prim()));
            ZUniMemory::Delete(m_pMotions2);
            m_pMotions2 = ZUniMemory::NewArray<ActBoneMotion2>(boneCount);
        }
        if (!motionsNull)
            stream.ExchangeRaw("Bones", m_pMotions2, sizeof(ActBoneMotion2) * boneCount);
        m_pMotions2BoneCount = motionsNull ? 0 : static_cast<int>(boneCount);

        LoadSaveAnimations(stream, bSaving);
        stream.Exchange("m_iAnimVariationFlags", reinterpret_cast<uint8_t&>(m_iAnimVariationFlags));
        m_QGroundAnimDestOld.LoadSave(stream, bSaving);
        m_QGroundAnimDest.LoadSave(stream, bSaving);
        m_QGroundAnimCurrent.LoadSave(stream, bSaving);
        stream.Exchange("m_GroundAnimDestStartFrame", m_GroundAnimDestStartFrame);
        stream.Exchange("m_GroundAnimDestEndFrame", m_GroundAnimDestEndFrame);
        stream.Exchange("m_fPelvisBoneOffset", m_fPelvisBoneOffset);
        stream.Exchange("m_LODMask", m_LODMask);
        stream.Exchange("m_bUseLODMASK", m_bUseLODMASK);
        stream.Exchange("m_FramesReset", m_FramesReset);
        stream.Exchange("m_bMetaKeyCallBacks", m_bMetaKeyCallBacks);

        bool boneModifierNull = m_pBoneModify == nullptr;
        stream.Exchange("isNull", boneModifierNull);
        if (!boneModifierNull)
        {
            m_pBoneModify->LoadSave(stream, bSaving);
        }

        bool poseAnimationRunning = m_pPoseAnim != nullptr;
        stream.Exchange("bPoseAnimRunning", poseAnimationRunning);
        if (!bSaving && poseAnimationRunning && !m_pPoseAnim)
            m_pPoseAnim = ZUniMemory::New<ZPoseAnim>();
        if (m_pPoseAnim && poseAnimationRunning)
        {
            stream.Exchange(ZToken::Void, m_pPoseAnim->m_rHost);
            stream.Exchange(ZToken::Void, m_pPoseAnim->m_dwID);
            stream.Exchange(ZToken::Void, m_pPoseAnim->m_lCount);
            stream.Exchange(ZToken::Void, m_pPoseAnim->m_lStartCount);
            stream.Exchange(ZToken::Void, m_pPoseAnim->m_lActivePoses);
            stream.Exchange(ZToken::Void, m_pPoseAnim->m_fStartTime);
            stream.Exchange(ZToken::Void, m_pPoseAnim->m_fTimeMultiplier);
            stream.Exchange(ZToken::Void, m_pPoseAnim->m_fEndTime);
            stream.Exchange(ZToken::Void, m_pPoseAnim->m_lPoseSize);
            stream.Exchange(ZToken::Void, m_pPoseAnim->m_fFrameTime);
            stream.Exchange(ZToken::Void, m_pPoseAnim->m_fFrameStartTime);
            stream.Exchange(ZToken::Void, m_pPoseAnim->m_iUpdate);
            stream.Exchange(ZToken::Void, m_pPoseAnim->m_lEmotionID);
            bool poseListPresent = m_pPoseAnim->m_pPoseList != nullptr;
            stream.Exchange(ZToken::Void, poseListPresent);
            if (!bSaving && poseListPresent && !m_pPoseAnim->m_pPoseList)
                m_pPoseAnim->m_pPoseList = ZUniMemory::NewArray<ZPoseModel>(m_pPoseAnim->m_lCount);
            if (poseListPresent)
            {
                for (uint32_t i = 0; i < m_pPoseAnim->m_lCount; ++i)
                {
                    auto& pose = m_pPoseAnim->m_pPoseList[i];
                    stream.Exchange(ZToken::Void, pose.m_cName);
                    stream.Exchange(ZToken::Void, pose.m_dwIndex);
                    stream.Exchange(ZToken::Void, pose.m_dwSize);
                    stream.Exchange(ZToken::Void, pose.m_bActive);
                    bool keysPresent = pose.m_pPoseKeys != nullptr;
                    stream.Exchange(ZToken::Void, keysPresent);
                    if (!bSaving && keysPresent)
                        pose.m_pPoseKeys = static_cast<SPoseKey*>(ZUniMemory::Allocate(sizeof(SPoseKey) * pose.m_dwSize));
                    if (keysPresent)
                    {
                        for (uint32_t key = 0; key < pose.m_dwSize; ++key)
                        {
                            stream.Exchange(ZToken::Void, pose.m_pPoseKeys[key].Frame);
                            stream.Exchange(ZToken::Void, pose.m_pPoseKeys[key].Strength);
                        }
                    }
                }
            }
        }

        for (auto& sound : m_AnimSound)
        {
            stream.Exchange(ZToken::Void, sound.m_rAnimationSound);
            stream.Exchange(ZToken::Void, sound.m_lSequenceID);
        }
    }

    // ZGEOM
    void ZLNKOBJ::CalcCenSize()
    {
        ZSTDOBJ::CalcCenSize();
        ZVector3 size;
        GetSize(size);
        const float extent = std::max(size.x, std::max(size.y, size.z));
        size = { extent };
        SetRadius(std::sqrt(3.0f * extent * extent) + 1.0f);
        SetSize(size);
    }

    int ZLNKOBJ::AnimCallBackToId(ZGEOM::ActiveAnimCB pCallback)
    {
        const ActiveAnimCB metaKeyCallback = ToAnimationCallback(&ZLNKOBJ::MetaKeyCallBack);
        if (pCallback == metaKeyCallback)
            return 4;

        const ActiveAnimCB soundCallback = ToAnimationCallback(&ZLNKOBJ::AnimSoundCallback);
        if (pCallback == soundCallback)
            return 3;

        return ZGEOM::AnimCallBackToId(pCallback);
    }

    ZGEOM::ActiveAnimCB ZLNKOBJ::AnimCallBackFromId(int id)
    {
        if (id == 3)
        {
            return ToAnimationCallback(&ZLNKOBJ::AnimSoundCallback);
        }
        if (id == 4)
        {
            return ToAnimationCallback(&ZLNKOBJ::MetaKeyCallBack);
        }
        return ZGEOM::AnimCallBackFromId(id);
    }

    bool ZLNKOBJ::WantViewPrimHideUnhideRequest() const
    {
        // TODO: Finish this place after ZHM3ItemWeapon will be reversed
        // return Parent()->IsDerivedFrom<ZHM3ItemWeapon>() && Parent()->WantViewPrimHideUnhideRequest();
        return false;
    }

    bool ZLNKOBJ::WantViewPrimHide(uint8_t lLODControl) const
    {
        // TODO: Finish this place after ZHM3ItemWeapon will be reversed
        // return Parent()->IsDerivedFrom<ZHM3ItemWeapon>() && Parent()->WantViewPrimHide(lLODControl);
        return false;
    }

    bool ZLNKOBJ::WantViewPrimHideMirrors(uint8_t lLODControl) const
    {
        (void)lLODControl;
        return false;
    }

    uint8_t ZLNKOBJ::GetLODMaskOverride() const
    {
        return m_bUseLODMASK ? m_LODMask : 0u;
    }

    void ZLNKOBJ::ClassInit()
    {
        if (!Prim() || !ZPrimControlBase::Instance()->GetPrimData(Prim()))
        {
            Delete();
            return;
        }
        EnableClassCall(16u);
        InitObjMatBone();
        if (auto* pParent = Parent())
        {
            if (GetObjectId() != ZLNKOBJ::m_Id && !(pParent->Control() & 0x4000) && (pParent->Control() & 0x40000))
                pParent->SetAutoRoomAssign(true);
            ZEngineGeomControl::GetInstance().GeomMoved(pParent->BaseGeom());
        }
        for (auto& sound : m_AnimSound)
            sound = {};
        if (!m_AnimVariations.IsValid())
            SetAnimCollectionProperty(reinterpret_cast<const ZANIM&>(animCollectionName));
    }

    void ZLNKOBJ::ClassFrameUpdate()
    {
        if (!m_Model)
            return;

        bool animationRunning = false;
        for (const auto& animation : m_Model->m_ActiveAnims)
            animationRunning |= (animation.mode & 7) != 0;
        if (!animationRunning)
        {
            if (Parent()->IsDerivedFrom<ZItem>())
            {
                DisableClassCall(16u);
            }
        }

        if (!UpdateAnimationsAndGroundLink(-1.0f))
            return;

        ZGROUP* pParent = Parent();
        if (!pParent)
            return;
        ZMat3x3 parentMat;
        ZVector3 parentPos;
        pParent->GetMatPos(parentMat, parentPos);
        ZMat3x3 resultMat;
        ZVector3 resultPos;
        mmmul(resultMat, m_Ground._Mat, parentMat);
        vmmul(resultPos, m_Ground._Pos, parentMat);
        resultPos += parentPos;
        vcross(resultMat.YAxis(), resultMat.XAxis(), resultMat.ZAxis());
        vcross(resultMat.XAxis(), resultMat.ZAxis(), resultMat.YAxis());
        vnorm(resultMat.ZAxis());
        vnorm(resultMat.YAxis());
        vnorm(resultMat.XAxis());

        // TODO: Finish this place after ZActor will be reversed
        // Actors set their grandparent root transform; other links set the parent local transform.
        pParent->SetMatPos(resultMat, resultPos);
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
        return m_pAttachedTo != 0;
    }

    void ZLNKOBJ::SetAttachedTo(ZLNKOBJ* pAttachedTo)
    {
        m_pAttachedTo = pAttachedTo ? pAttachedTo->GetRef() : 0;
    }

    ZLNKOBJ* ZLNKOBJ::GetAttachedTo() const
    {
        return ZGEOM::RefCast<ZLNKOBJ>(m_pAttachedTo);
    }

    // ZLNKOBJ
    Animation::Header* ZLNKOBJ::GetAnim(const char*)
    {
        return nullptr;
    }

    void ZLNKOBJ::StopAllAnims(bool bUpdateAlways)
    {
        if (!m_Model)
            return;
        for (auto& animation : m_Model->m_ActiveAnims)
        {
            const int mode = animation.mode & 7;
            if ((mode == 1 || mode == 3) && ((animation.mode & 0x8000) || !bUpdateAlways))
            {
                StopAnimSound((animation.header->m_Mask & 8) == 0, animation.sequenceId, false);
                AnimEnd(&animation, Control());
                animation.Destroy(true);
                animation.mode = 0;
            }
        }
    }

    void ZLNKOBJ::StopUBAnims()
    {
        if (!m_Model)
            return;
        for (auto& animation : m_Model->m_ActiveAnims)
        {
            const int mode = animation.mode & 7;
            if ((mode == 1 || mode == 3) && !(animation.mode & 0x8000))
            {
                StopAnimSound((animation.header->m_Mask & 8) == 0, animation.sequenceId, false);
                animation.Destroy(true);
                animation.mode = 0;
            }
        }
    }

    void ZLNKOBJ::StopAnim(Animation::ActiveAnimation* pAnimation)
    {
        if (!pAnimation)
            return;
        const int mode = pAnimation->mode & 7;
        if (mode == 1 || mode == 3)
        {
            StopAnimSound((pAnimation->header->m_Mask & 8) == 0, pAnimation->sequenceId, false);
            pAnimation->Destroy(true);
            pAnimation->mode = 0;
        }
    }

    Animation::ActiveAnimation* ZLNKOBJ::ActivateAnimSegment(Animation::Header* pHeader, int control, float startFrame, float endFrame, float speed)
    {
        if (!pHeader || !m_Model || startFrame < 0.0f)
            return nullptr;
        if (endFrame == -1.0f)
            endFrame = static_cast<float>(pHeader->m_Frames - 1);
        if (startFrame > endFrame || endFrame > pHeader->m_Frames - 1)
            return nullptr;

        if (control & 1)
        {
            const bool ground = (pHeader->m_OldControl & 0x10) == 0;
            for (auto& animation : m_Model->m_ActiveAnims)
            {
                const int mode = animation.mode & 7;
                if ((mode == 1 || mode == 3) && ((animation.mode & 0x8000) != 0) == ground)
                    StopAnim(&animation);
            }
        }

        Animation::ActiveAnimation* pAnimation = nullptr;
        for (auto& animation : m_Model->m_ActiveAnims)
        {
            if ((animation.mode & 7) == 0)
            {
                pAnimation = &animation;
                break;
            }
        }
        if (!pAnimation)
            return nullptr;

        m_Model->m_Animated = true;
        pAnimation->header = pHeader;
        pAnimation->frame = startFrame;
        pAnimation->startFrame = startFrame;
        pAnimation->endFrame = endFrame;
        pAnimation->deltaframe = speed * 25.0f;
        pAnimation->blend = std::min(endFrame - startFrame,
            (control & 0x80) ? pHeader->m_BlendFrames : std::max(8.0f, pHeader->m_BlendFrames));

        pAnimation->sequenceId = g_pEngineData->GetNextAnimId();
        pAnimation->mode = (control & 4) ? 1 : 3;
        if (control & 2)
            pAnimation->blend = 0.0f;
        if (control & 0x1000)
            pAnimation->mode |= 0x8000000;
        if (control & 0x20)
            pAnimation->mode |= 0x1000;
        if (control & 0x8000)
            pAnimation->mode |= 0x100;
        if (control & 0x80)
            pAnimation->mode |= 0x2000;
        pAnimation->mode |= (control & 0x100) << 9;
        if ((pHeader->m_OldControl & 0x10) == 0)
        {
            pAnimation->mode |= 0x8000;
            m_Model->m_LastFullBody.m_Id = static_cast<uint16_t>(Animation::instance->ToIndex(pHeader));
            m_Model->m_LastFullBody.m_Mirrored = (control & 0x8000) != 0;
            m_Model->m_LastUpperBody.m_Id = 0x7FFF;
            pHeader->GetGround(Animation::instance, startFrame,
                m_QGroundAnimCurrent.m_OffsetQuat, m_QGroundAnimCurrent.m_OffsetPos, (control & 0x8000) != 0);
            m_GroundAnimDestStartFrame = -1.0f;
            m_GroundAnimDestEndFrame = -1.0f;
        }
        else
        {
            m_Model->m_LastUpperBody.m_Id = static_cast<uint16_t>(Animation::instance->ToIndex(pHeader));
            m_Model->m_LastUpperBody.m_Mirrored = (control & 0x8000) != 0;
        }
        if (pHeader->HasBone(Animation::instance, Ground))
            pAnimation->mode |= 0x400;

        if (m_bMetaKeyCallBacks)
        {
            const uint32_t count = Animation::instance->GetMetaKeyDataLength(pHeader->m_MetaDataOffset);
            for (uint32_t i = 0; i < count; ++i)
                pAnimation->AddCallBack(static_cast<float>(Animation::instance->GetMetaKeyData(pHeader->m_MetaDataOffset)[i].lFrame),
                    ToAnimationCallback(&ZLNKOBJ::MetaKeyCallBack), i * sizeof(Animation::ZMetaKey), 0, 0);
        }
        return pAnimation;
    }

    Animation::ActiveAnimation* ZLNKOBJ::ActivateAnimSegment(ZAnimVariationHandle handle, int control, float startFrame, float endFrame, float speed)
    {
        const float random = g_pSysInterface->FRand(const_cast<char*>(__FILE__), __LINE__);
        return ActivateAnimSegment(GetAnimHeaderFromVariation(handle, m_iAnimVariationFlags, random), control, startFrame, endFrame, speed);
    }

    Animation::ActiveAnimation* ZLNKOBJ::ActivateAnim(Animation::Header* pHeader, int lControl)
    {
        return ActivateAnimSegment(pHeader, lControl, 0.0f, -1.0f, 1.0f);
    }

    bool ZLNKOBJ::SetActiveAnimDestination(const float* pMat, const float* pPos, float duration)
    {
        auto* pAnimation = GetGroundAnimation();
        if (!pAnimation)
            return false;

        m_GroundAnimDestStartFrame = pAnimation->frame;
        m_GroundAnimDestEndFrame = std::min(pAnimation->frame + duration, pAnimation->endFrame);
        ZQuat animationQuat;
        ZVector3 animationPos;
        pAnimation->header->GetGround(Animation::instance, m_GroundAnimDestEndFrame,
            &animationQuat.i, &animationPos.x, (pAnimation->mode & 0x100) != 0);
        ZQuat destinationQuat;
        mattoquat(destinationQuat, *reinterpret_cast<const ZMat3x3*>(pMat));
        ZMat3x3 currentRootMat;
        ZVector3 currentRootPos;
        GetRootTM(currentRootMat, currentRootPos);
        ZQuat currentRootQuat;
        mattoquat(currentRootQuat, currentRootMat);

        ZDeltaBone animationTransform;
        *reinterpret_cast<ZQuat*>(animationTransform.m_OffsetQuat) = animationQuat;
        *reinterpret_cast<ZVector3*>(animationTransform.m_OffsetPos) = animationPos;
        ZDeltaBone destinationTransform;
        *reinterpret_cast<ZQuat*>(destinationTransform.m_OffsetQuat) = destinationQuat;
        *reinterpret_cast<ZVector3*>(destinationTransform.m_OffsetPos) = *reinterpret_cast<const ZVector3*>(pPos);
        ZDeltaBone rootTransform;
        *reinterpret_cast<ZQuat*>(rootTransform.m_OffsetQuat) = currentRootQuat;
        *reinterpret_cast<ZVector3*>(rootTransform.m_OffsetPos) = currentRootPos;
        const ZDeltaBone currentGroundWorld = ComposeTransform(rootTransform, m_QGroundAnimCurrent);
        m_QGroundAnimDest = RelativeTransform(
            RelativeTransform(currentGroundWorld, animationTransform),
            RelativeTransform(rootTransform, destinationTransform));
        m_QGroundAnimDestOld = {};
        m_QGroundAnimCurrent = {};
        pAnimation->mode |= 0x200;
        return true;
    }

    bool ZLNKOBJ::CheckActiveAnim(Animation::Header* pHeader, int)
    {
        if (!m_Model)
            return false;
        for (const auto& animation : m_Model->m_ActiveAnims)
            if ((animation.mode & 7) && animation.header == pHeader)
                return true;
        return false;
    }

    Animation::ActiveAnimation* ZLNKOBJ::SetBoneFrameBlend(Animation::Header* pHeader, float frame, float blend, bool absoluteFrame, unsigned int control)
    {
        if (!pHeader || !m_Model || !pHeader->m_Frames)
            return nullptr;
        if (!absoluteFrame)
            frame = (frame == 1.0f ? frame : std::fmod(frame, 1.0f)) * (pHeader->m_Frames - 1);
        if (pHeader->m_RealFPS != 25)
            frame = std::min(frame * (25.0f / pHeader->m_RealFPS), static_cast<float>(pHeader->m_Frames - 1));
        ZASSERT(frame >= 0.0f && frame <= pHeader->m_Frames - 1);
        frame = std::clamp(frame, 0.0f, static_cast<float>(pHeader->m_Frames - 1));

        Animation::ActiveAnimation* pAnimation = nullptr;
        for (auto& animation : m_Model->m_ActiveAnims)
        {
            if ((animation.mode & 7) == 0 || ((animation.mode & 7) == 2 && animation.header == pHeader))
            {
                pAnimation = &animation;
                break;
            }
        }
        if (!pAnimation)
            return nullptr;
        m_Model->m_Animated = true;
        pAnimation->header = pHeader;
        pAnimation->frame = frame;
        pAnimation->startFrame = 0.0f;
        pAnimation->endFrame = static_cast<float>(pHeader->m_Frames - 1);
        pAnimation->deltaframe = 25.0f;
        pAnimation->mode = 2 | ((control & 0x100) << 9);
        if ((pHeader->m_OldControl & 0x10) == 0)
            pAnimation->mode |= 0x8000;
        if (control & 0x8000)
            pAnimation->mode |= 0x100;
        if (control & 0x80)
            pAnimation->mode |= 0x2000;
        pAnimation->blend = (control & 0x4000) ? blend * 25.0f
            : ((control & 0x80) ? pHeader->m_BlendFrames : std::max(8.0f, pHeader->m_BlendFrames));
        if (control & 2)
            pAnimation->blend = 0.0f;
        pAnimation->sequenceId = -1;
        return pAnimation;
    }

    void ZLNKOBJ::SetDualFrame(Animation::Header* pHeader1, float frame1, Animation::Header* pHeader2, float frame2, float blend2, float blend1)
    {
        ZASSERT(pHeader1 && pHeader2);
        if (!pHeader1 || !pHeader2 || !m_Model)
            return;
        Animation::ActiveAnimation* first = nullptr;
        for (int i = 0; i < 3; ++i)
        {
            if ((m_Model->m_ActiveAnims[i].mode & 7) == 0 && (m_Model->m_ActiveAnims[i + 1].mode & 7) == 0)
            {
                first = &m_Model->m_ActiveAnims[i];
                break;
            }
        }
        if (!first)
            return;
        m_Model->m_Animated = true;
        first->header = pHeader1;
        first->frame = frame1 * (pHeader1->m_Frames - 1);
        first->startFrame = 0.0f;
        first->endFrame = static_cast<float>(pHeader1->m_Frames - 1);
        first->blend = blend1;
        first->deltaframe = 25.0f;
        first->mode = 0x2000A | (((pHeader1->m_OldControl & 0x10) == 0) ? 0x8000 : 0);
        first->sequenceId = -1;
        auto* second = first + 1;
        second->header = pHeader2;
        second->frame = frame2 * (pHeader2->m_Frames - 1);
        second->startFrame = 0.0f;
        second->endFrame = static_cast<float>(pHeader2->m_Frames - 1);
        second->blend = blend2;
        second->deltaframe = 25.0f;
        second->mode = 0x20002 | (((pHeader1->m_OldControl & 0x10) == 0) ? 0x8000 : 0);
        second->sequenceId = -1;
    }

    bool ZLNKOBJ::GroundAnimated()
    {
        auto* pAnimation = GetGroundAnimation();
        return pAnimation && ((pAnimation->mode & 0x200)
            || (!(pAnimation->header->m_Mask & 8) && pAnimation->header->HasBone(Animation::instance, Ground)));
    }

    void ZLNKOBJ::OnMetaKey(Animation::ActiveAnimation*, Animation::ZMetaKey*, const char*)
    {
    }

    ZPoseAnim* ZLNKOBJ::ActivatePoseAnim(char* pBuffer, float fTime, unsigned int lId, float fMultiplier)
    {
        if (!pBuffer)
            return nullptr;
        StopPoseAnim(lId, true);
        m_pPoseAnim = ZUniMemory::New<ZPoseAnim>(lId);
        if (!m_pPoseAnim->Create(pBuffer))
        {
            ZUniMemory::Delete(m_pPoseAnim);
            m_pPoseAnim = nullptr;
            return nullptr;
        }
        const float realTime = static_cast<float>(g_pSysInterface->GetRealTime());
        m_pPoseAnim->SetStartTime(realTime);
        m_pPoseAnim->SetFrameStartTime(realTime);
        m_pPoseAnim->SetTimeMultiplier(fMultiplier);
        m_pPoseAnim->SetEndTime(realTime + fTime * fMultiplier);
        return m_pPoseAnim;
    }

    bool ZLNKOBJ::StopPoseAnim(unsigned int host, bool bForce)
    {
        if (m_pPoseAnim && !bForce && m_pPoseAnim->GetHost() != host)
            return false;
        ZUniMemory::Delete(m_pPoseAnim);
        m_pPoseAnim = nullptr;
        return true;
    }

    void ZLNKOBJ::StopAudio()
    {
        StopAllAnimSounds();
    }

    bool ZLNKOBJ::StopAnimSound(bool upperBody, int sequenceId, bool force)
    {
        if (force || m_AnimSound[upperBody].m_lSequenceID == sequenceId)
        {
            auto* pSound = g_pEngineData->SRefToPtr(m_AnimSound[upperBody].m_rAnimationSound);
            if (pSound)
            {
                pSound->m_lSoundFlags |= 2;

                if ((pSound->m_lSoundFlags & 0x800) != 0)
                {
                    ZUniMemory::Delete(pSound);

                    m_AnimSound[upperBody].m_rAnimationSound = 0u;
                    m_AnimSound[upperBody].m_lSequenceID = 0;
                    return false;
                }

                pSound->SetFadeOut(0.2f, 0.0f, 0.0f);
            }

            m_AnimSound[upperBody].m_rAnimationSound = 0;
            m_AnimSound[upperBody].m_lSequenceID = 0;

            return false;
        }

        return force; // ???
    }

    void ZLNKOBJ::StopAllAnimSounds()
    {
        for (int i = 0; i < 2; ++i)
        {
            auto* pActiveSound = &m_AnimSound[i];
            auto* pSound = g_pEngineData->SRefToPtr(pActiveSound->m_rAnimationSound);
            if (pSound)
            {
                pSound->m_lSoundFlags |= 0x2u;
                if ((pSound->m_lSoundFlags & 0x800) == 0)
                {
                    pSound->SetFadeOut(0.2f, 0.0f, 0.0f);
                }
                else
                {
                    ZUniMemory::Delete(pSound);
                }
            }

            pActiveSound->m_rAnimationSound = 0;
            pActiveSound->m_lSequenceID = 0;
        }
    }

    uint32_t ZLNKOBJ::CheckLineCollision(float* pResult, const float* pLineStart, const float* pLineDirection) const
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

    uint32_t ZLNKOBJ::CheckLineCollision(float* pResult, const float* pLineStart, const float* pLineDirection, float* pUnused) const
    {
        return CheckLineCollision(pResult, pLineStart, pLineDirection);
    }

    uint32_t ZLNKOBJ::CheckBoxCollision(float const* pMat, float const* pPos, float const* pSize) const
    {
        if (!m_pBoneModify || m_pBoneModify->m_lNumActiveBones <= 1)
        {
            return 0;
        }

        const auto* pBones = GetBones();
        const auto* pDefinitions = GetBoneDefinitions();
        uint32_t bestId = 1000;
        bool collided = false;
        ZMat3x3 localMat { pMat };
        ZVector3 localPos { pPos };
        GetLocalMatPos(localMat, localPos);
        for (uint32_t i = 1; i < m_pBoneModify->m_lNumActiveBones; ++i)
        {
            if (rectBoxColi(localMat.data, &localPos.x, pSize, pBones[i]._Mat.data, &pBones[i]._Pos.x, &pDefinitions[i].Size.x))
            {
                collided = true;
                bestId = std::min(bestId, static_cast<uint32_t>(pDefinitions[i].BodyPart));
            }
        }
        return collided ? bestId : 0;
    }

    bool ZLNKOBJ::Use2Skeletons(void)
    {
        return false;
    }

    void ZLNKOBJ::LocalStateIK(void)
    {
        ZASSERT(m_Model);
        m_Model->m_PelvisPlacement.m_Quat.i = -2.0f;
    }

    void ZLNKOBJ::GetDefaultBones(ZBone* pBones, uint32_t lFirstBoneNum) const
    {
        const auto count = NumActiveBones();
        const auto* pDefinitions = GetBoneDefinitions();
        const auto* pLocalBones = GetOrigLocalBones();
        if (!pDefinitions || !pLocalBones || lFirstBoneNum >= count)
            return;

        uint32_t first = lFirstBoneNum;
        if (!first)
        {
            pBones[0]._Mat.Reset();
            pBones[0]._Pos.Reset();
            first = 1;
        }
        const uint32_t packedCount = std::min<uint32_t>(count, m_pMotions2BoneCount);
        for (uint32_t i = first; i < packedCount; ++i)
        {
            const auto* packed = reinterpret_cast<const int8_t*>(&m_pMotions2[i].m_PackedQuat);
            ZQuat quat { packed[1] / 112.0f, packed[2] / 112.0f, packed[3] / 112.0f, packed[0] / 112.0f };
            if (!m_pMotions2[i].m_PackedQuat)
                quat = {};
            quattomat(pBones[i]._Mat, quat);
            pBones[i]._Pos = pLocalBones[i]._Pos;
            const uint32_t parent = pDefinitions[i].lPrevBoneNr;
            if (parent)
            {
                pBones[i]._Mat *= pBones[parent]._Mat;
                vmmul(pBones[i]._Pos, pBones[i]._Pos, pBones[parent]._Mat);
                pBones[i]._Pos += pBones[parent]._Pos;
            }
        }
        const auto* pLocalQuats = reinterpret_cast<const Animation::ZAngelBone*>(
            ZPrimControlBase::Instance()->GetLocalPrimBonesQuats(Prim()));
        for (uint32_t i = std::max(first, packedCount); i < count; ++i)
        {
            pBones[i]._Quat = pLocalQuats[i].m_Quat;
            pBones[i]._Pos = pLocalQuats[i].m_Pos;
            const uint32_t parent = pDefinitions[i].lPrevBoneNr;
            pBones[i]._Mat *= pBones[parent]._Mat;
            vmmul(pBones[i]._Pos, pBones[i]._Pos, pBones[parent]._Mat);
            pBones[i]._Pos += pBones[parent]._Pos;
        }
    }

    void ZLNKOBJ::SetDefaultBones(const ZBone* pBones, const SBoneDefinition* pDefinitions)
    {
        if (!m_pMotions2 || !m_pBoneModify || !pBones || !pDefinitions)
            return;
        for (uint32_t i = 1; i < m_pBoneModify->m_lNumActiveBones; ++i)
        {
            ZMat3x3 local = pBones[i]._Mat;
            const uint32_t parent = pDefinitions[i].lPrevBoneNr;
            if (parent)
            {
                ZMat3x3 parentTranspose = pBones[parent]._Mat.TransposedAntidiagonal();
                local *= parentTranspose;
            }
            ZQuat quat;
            mattoquat(quat, local);
            auto* packed = reinterpret_cast<int8_t*>(&m_pMotions2[i].m_PackedQuat);
            packed[0] = static_cast<int8_t>(quat.w * 112.0f);
            packed[1] = static_cast<int8_t>(quat.i * 112.0f);
            packed[2] = static_cast<int8_t>(quat.j * 112.0f);
            packed[3] = static_cast<int8_t>(quat.k * 112.0f);
            if (!m_pMotions2[i].m_PackedQuat)
                packed[3] = 112;
        }
        m_pMotions2BoneCount = m_pBoneModify->m_lNumActiveBones;
    }

    ZDeltaBone* ZLNKOBJ::GetAnimDeltaBones(Animation::Header*)
    {
        ZASSERT(false);
        return nullptr;
    }

    void ZLNKOBJ::GetBoneMatPos(ZMat3x3* pMat, ZVector3* pPos, unsigned int lBoneId) const
    {
        m_pBoneModify->GetBoneMatPos(*pMat, *pPos, lBoneId, this);
    }

    bool ZLNKOBJ::AttachBaseGeomToBone(const ZBaseGeom* pBaseGeom, unsigned int lBoneId, const float* pMat, const float* pPos)
    {
        return m_pBoneModify->AttachBaseGeomToBone(pBaseGeom, lBoneId, pMat, pPos);
    }

    void ZLNKOBJ::DetachBaseGeomFromBone(const ZBaseGeom* pBaseGeom, unsigned int lBoneId)
    {
        m_pBoneModify->DetachBaseGeomFromBone(pBaseGeom, lBoneId);
    }

    uint32_t ZLNKOBJ::GetAttachedBaseGeomBoneId(const ZBaseGeom* pBaseGeom)
    {
        return m_pBoneModify->GetAttachedBaseGeomBoneId(pBaseGeom);
    }

    void ZLNKOBJ::GetGroundBoneAnimMatPos(ZMat3x3* pMat, ZVector3* pPos, Animation::Header* pHeader, float fFrame, bool bMirror)
    {
        ZQuat quat;
        pHeader->GetGround(g_pEngineData->m_AnimationManager, fFrame, &quat.i, &pPos->x, bMirror);
        quattomat(*pMat, quat);
    }

    void ZLNKOBJ::GetGroundBoneDeltaMatPos(ZMat3x3* pMat, ZVector3* pPos, Animation::Header* pHeader, float fStartFrame, float fEndFrame)
    {
        ZQuat startQuat;
        ZQuat endQuat;
        ZVector3 startPos;
        ZVector3 endPos;
        pHeader->GetGround(g_pEngineData->m_AnimationManager, fStartFrame, &startQuat.i, &startPos.x, false);
        pHeader->GetGround(g_pEngineData->m_AnimationManager, fEndFrame, &endQuat.i, &endPos.x, false);

        ZMat3x3 startMat;
        ZMat3x3 endMat;
        quattomat(startMat, startQuat);
        quattomat(endMat, endQuat);
        const ZMat3x3 startInverse = startMat.TransposedAntidiagonal();
        mmmul(*pMat, endMat, startInverse);
        vsub(*pPos, endPos, startPos);
        vmtmul(*pPos, startInverse.data);
    }

    bool ZLNKOBJ::GetAttachedGeomMatPos(const ZBaseGeom* pBaseGeom, float* pMat, float* pPos) const
    {
        return m_pBoneModify->FindAttachedGeomMatPos(*reinterpret_cast<ZMat3x3*>(pMat), *reinterpret_cast<ZVector3*>(pPos), pBaseGeom, this);
    }

    bool ZLNKOBJ::DisplayBone(unsigned int, bool)
    {
        return false;
    }

    void ZLNKOBJ::GetFocusMatPos(float* pMat, float* pPos) const
    {
        GetMatPos(*reinterpret_cast<ZMat3x3*>(pMat), *reinterpret_cast<ZVector3*>(pPos));
    }

    void ZLNKOBJ::DisplayAllBones(bool)
    {
    }

    void ZLNKOBJ::GetRootFocusMatPos(float* pMat, float* pPos)
    {
        GetFocusMatPos(pMat, pPos);
        GetRootMatPos(*reinterpret_cast<ZMat3x3*>(pMat), *reinterpret_cast<ZVector3*>(pPos));
    }

    int32_t ZLNKOBJ::GetBoneNrFromId(uint8_t lBoneId) const
    {
        if (!lBoneId)
            return -1;
        const uint8_t index = ZPrimControlBase::Instance()->GetBoneIdToIndexLookup(Prim())[lBoneId];
        return index == 0xFF ? -1 : index;
    }

    int32_t ZLNKOBJ::GetBoneFromPoint(float const* pPoint) const
    {
        const auto* pBones = GetBones();
        const auto* pDefinitions = GetBoneDefinitions();
        float bestDistance = 1.0e38f;
        int32_t bestBone = 0;
        for (uint32_t i = 1; i < NumActiveBones(); ++i)
        {
            ZVector3 center;
            vmmul(center, pDefinitions[i].Center, pBones[i]._Mat);
            center += pBones[i]._Pos;
            ZVector3 delta;
            vsub(delta, pPoint, center);
            vmtmul(delta, pBones[i]._Mat.data);
            const float distance = vlen2(delta);
            const bool contains = std::fabs(delta.x) < pDefinitions[i].Size.x
                && std::fabs(delta.y) < pDefinitions[i].Size.y
                && std::fabs(delta.z) < pDefinitions[i].Size.z;
            if (contains)
            {
                bestDistance = 0.0f;
                bestBone = static_cast<int32_t>(i);
                continue;
            }
            if (distance < bestDistance)
            {
                bestDistance = distance;
                bestBone = static_cast<int32_t>(i);
            }
        }
        return bestBone;
    }

    int32_t ZLNKOBJ::GetBoneNrFromName(char const* pszName) const
    {
        const auto* pDefinitions = GetBoneDefinitions();
        for (uint32_t i = 1; i < NumActiveBones(); ++i)
            if (!_stricmp(pDefinitions[i].Name, pszName))
                return static_cast<int32_t>(i);
        return 0;
    }

    const char* ZLNKOBJ::GetBoneName(int boneIndex) const
    {
        return boneIndex < 0 ? nullptr : GetBoneDefinitions()[boneIndex].Name;
    }

    const ZBone* ZLNKOBJ::GetOrigLocalBones(void) const
    {
        return reinterpret_cast<const ZBone*>(ZPrimControlBase::Instance()->GetLocalPrimBones(Prim()));
    }

    const SBoneDefinition* ZLNKOBJ::GetBoneDefinitions() const
    {
        return ZPrimControlBase::Instance()->GetBoneDefinitions(Prim());
    }

    void ZLNKOBJ::CopyGeometryFrom(ZGEOM* pSource)
    {
        CopyGeometryFrom(pSource->Prim());
    }

    void ZLNKOBJ::CopyGeometryFrom(unsigned int lPrim)
    {
        if (!m_pBoneModify)
            return;
        const uint32_t oldPrim = Prim();
        const uint8_t* pOldLookup = ZPrimControlBase::Instance()->GetBoneIdToIndexLookup(oldPrim);
        ActBoneMotion2* pOldMotions = m_pMotions2;
        const uint32_t boneCount = ZPrimControlBase::Instance()->GetNrBones(lPrim);
        auto* pMotions = ZUniMemory::NewArray<ActBoneMotion2>(boneCount);
        std::memset(pMotions, 0, sizeof(ActBoneMotion2) * boneCount);
        m_pMotions2 = pMotions;
        m_pMotions2BoneCount = 0;
        m_pBoneModify->m_lNumActiveBones = static_cast<uint16_t>(boneCount);
        const uint8_t* pNewLookup = ZPrimControlBase::Instance()->GetBoneIdToIndexLookup(lPrim);
        const SBoneDefinition* pNewDefinitions = ZPrimControlBase::Instance()->GetBoneDefinitions(lPrim);
        SetDefaultBones(reinterpret_cast<const ZBone*>(ZPrimControlBase::Instance()->GetGlobalPrimBones(lPrim)), pNewDefinitions);
        for (uint32_t i = 0; i < boneCount; ++i)
        {
            const uint8_t boneId = pNewDefinitions[i].Id;
            const uint8_t oldIndex = pOldLookup[boneId];
            const uint8_t newIndex = pNewLookup[boneId];
            ZASSERT(newIndex != 0xFF);
            if (oldIndex != 0xFF)
                m_pMotions2[newIndex] = pOldMotions[oldIndex];
        }
        BaseGeom()->SetPrim(static_cast<int>(lPrim));
        m_pBoneModify->PrimChanged(lPrim);
        m_Model->m_Valid = false;
        m_Model->m_Bones = nullptr;
        m_Model->m_PoseWeights = nullptr;
        m_Model->m_StateBlending = nullptr;
        m_Model->m_BlendBones = nullptr;
        m_Model->m_State = nullptr;
        ZUniMemory::Delete(pOldMotions);
    }

    void ZLNKOBJ::CopyPoseFrom(ZLNKOBJ*)
    {
    }

    bool ZLNKOBJ::ChangeMesh(ZGROUP* pGroup)
    {
        if (!pGroup || !pGroup->Is<ZGROUP>())
            return false;
        for (ZBaseGeom* pGeom = pGroup->m_pGroupLast; pGeom && !ForGroupsCheck(pGeom); pGeom = pGeom->GetPrev())
        {
            const char* pName = pGeom->Name();
            if (pName && !_strnicmp(pName, "ground", 6))
            {
                if (auto* pExtraGeom = pGeom->GetGeom())
                {
                    CopyGeometryFrom(pExtraGeom->Prim());
                    pExtraGeom->CalcCenSize();
                }
                else
                {
                    CopyGeometryFrom(pGeom->Prim());
                    pGeom->ForceCalcMaxMin();
                }
                BaseGeom()->ForceCalcMaxMin();
                break;
            }
        }
        return true;
    }

    void ZLNKOBJ::PrintAllBoneNames(void) const
    {
        const auto* pDefinitions = GetBoneDefinitions();
        for (uint32_t i = 0; i < NumActiveBones(); ++i)
        {
            printf("lBoneNr %u Name \"%s\"\n", i, pDefinitions[i].Name);
            const uint32_t parent = pDefinitions[i].lPrevBoneNr;
            if (parent != static_cast<uint32_t>(-1))
                printf("        Parent %u Name \"%s\"\n", parent, pDefinitions[parent].Name);
        }
    }

    void ZLNKOBJ::SetCutSequence(bool bCutSequence)
    {
        m_bCutSequence = bCutSequence;
        if (!bCutSequence)
        {
            const ZMSGID msg = g_pEngineData->RegisterZMsg("MSG_LNKOBJOUTOFCUTSEQ", 0, __FILE__, __LINE__);
            const ZREF camera = g_pEngineData->GetSceneVar("MainCamera");
            SendCommand(camera, msg, reinterpret_cast<void*>(GetRef()));
        }
    }

    void ZLNKOBJ::MoveToMatPos(float const* pMat, float const* pPos)
    {
        SetRootTM(*reinterpret_cast<const ZMat3x3*>(pMat), *reinterpret_cast<const ZVector3*>(pPos));
    }

    void ZLNKOBJ::SetRootTMParent(float* pMat, float* pPos)
    {
        Parent()->SetRootTM(*reinterpret_cast<ZMat3x3*>(pMat), *reinterpret_cast<ZVector3*>(pPos));
    }

    bool ZLNKOBJ::EventCallBack(Animation::ActiveAnimation*, float, float, void* pData)
    {
        if (!pData)
            return false;
        const auto* pChunk = static_cast<const CHUNKFILE*>(pData);
        const char* pName = static_cast<const char*>(const_cast<CHUNKFILE*>(pChunk)->GetData('NAME', 0));
        if (!pName)
            return false;
        const ZMSGID msg = g_pEngineData->RegisterZMsg(pName, 0, __FILE__, __LINE__);
        ClassCommand(msg, nullptr);
        return false;
    }

    void ZLNKOBJ::OnMoving(void)
    {
        // Intentionally empty in the PC vtable.
    }

    void ZLNKOBJ::OnMoved(void)
    {
        // Intentionally empty in the PC vtable.
    }

    void ZLNKOBJ::CalcTightCenSize(float* pCen, float* pSize) const
    {
        std::memcpy(pCen, BaseGeom()->Cen(), sizeof(ZVector3));
        std::memcpy(pSize, BaseGeom()->Size(), sizeof(ZVector3));
    }

    void ZLNKOBJ::CalcShadowProjectPlane(float* pPlane, float const*, float const*) const
    {
        pPlane[0] = pPlane[1] = pPlane[2] = pPlane[3] = 1.0e38f;
    }

    int32_t ZLNKOBJ::GetBoneControl(int) const
    {
        const auto* pAnimation = GetGroundAnimation();
        return pAnimation ? (pAnimation->mode >> 7) & 0x20 : 0;
    }

    void ZLNKOBJ::UpdateGeometry(bool)
    {
        // Intentionally empty in the PC vtable.
    }

    void ZLNKOBJ::ResetAllAnimBones(void)
    {
        // Intentionally empty in PC, iOS, and PS2.
    }

    void ZLNKOBJ::ResetInactiveBones(void)
    {
        // Intentionally empty in PC, iOS, and PS2.
    }

    void ZLNKOBJ::ExecuteCallBack(char const*)
    {
        ZASSERT(false);
    }

    float ZLNKOBJ::GetBoneVolume(int boneIndex)
    {
        const auto& size = GetBoneDefinitions()[boneIndex].Size;
        return size.x * size.y * size.z;
    }

    void ZLNKOBJ::GetBoneSize(int boneIndex, Glacier::Vector3* pSize)
    {
        *pSize = GetBoneDefinitions()[boneIndex].Size;
    }

    void ZLNKOBJ::GetBoneCenter(int boneIndex, Glacier::Vector3* pCenter)
    {
        *pCenter = GetBoneDefinitions()[boneIndex].Center;
    }

    void ZLNKOBJ::AnimEnd(Animation::ActiveAnimation*, int)
    {
        // Intentionally empty in the PC vtable.
    }

    bool ZLNKOBJ::UpdateAnimationsAndGroundLink(float fDeltaTime)
    {
        if (fDeltaTime < 0.0f)
            fDeltaTime = g_pSysInterface->DeltaFrameTime;
        if (m_Model && !m_FramesReset)
            m_Model->PostAnim(fDeltaTime);
        m_FramesReset = false;

        Animation::ActiveAnimation* pFinishedGroundAnimation = nullptr;
        if (m_Model)
        {
            for (auto& animation : m_Model->m_ActiveAnims)
            {
                const int mode = animation.mode & 7;
                if (!mode || mode == 2 || (animation.mode & 0x100000))
                    continue;

                animation.frame += fDeltaTime * animation.deltaframe;
                animation.UpdateCallBacks();
                if (animation.frame <= animation.endFrame)
                    continue;

                StopAnimSound((animation.header->m_Mask & 8) == 0, animation.sequenceId, false);
                if (mode == 3)
                {
                    if ((animation.mode & 0x8400) == 0x8400)
                        pFinishedGroundAnimation = &animation;
                    animation.mode &= ~3;
                    if (m_eStatus == STATUS_PostInit2)
                        ClassCommand(2051, &animation.sequenceId);
                    AnimEnd(&animation, Control());
                    animation.Destroy(false);
                }
                else
                {
                    const float length = animation.endFrame - animation.startFrame;
                    if (length > 0.0f)
                        animation.frame = std::fmod(animation.frame - animation.startFrame, length) + animation.startFrame;
                    m_QGroundAnimCurrent = {};
                }
            }
        }

        UpdatePoseAnimation();

        Animation::ActiveAnimation* pGroundAnimation = GetGroundAnimation();
        if (!pGroundAnimation)
            pGroundAnimation = pFinishedGroundAnimation;
        if (!pGroundAnimation || !(pGroundAnimation->mode & 0x600))
        {
            m_Ground._Mat.Reset();
            m_Ground._Pos.Reset();
            if (m_pBoneModify)
                m_pBoneModify->Update(this, m_Ground._Mat, m_Ground._Pos);
            return false;
        }

        const float frame = std::min(pGroundAnimation->frame, pGroundAnimation->endFrame);
        ZDeltaBone ground;
        pGroundAnimation->header->GetGround(Animation::instance, frame,
            ground.m_OffsetQuat, ground.m_OffsetPos, (pGroundAnimation->mode & 0x100) != 0);
        ZDeltaBone delta;
        if (pGroundAnimation->mode & 0x200)
        {
            float t = 1.0f;
            if (frame < m_GroundAnimDestEndFrame && m_GroundAnimDestEndFrame != m_GroundAnimDestStartFrame)
                t = (frame - m_GroundAnimDestStartFrame) / (m_GroundAnimDestEndFrame - m_GroundAnimDestStartFrame);
            ZDeltaBone destinationPart;
            const ZQuat identity;
            qpul(destinationPart.m_OffsetQuat, &identity.i, m_QGroundAnimDest.m_OffsetQuat, t);
            vscalar(destinationPart.m_OffsetPos, m_QGroundAnimDest.m_OffsetPos, t);
            const ZDeltaBone correctedOld = ComposeTransform(m_QGroundAnimDestOld, destinationPart);
            delta = RelativeTransform(correctedOld, ground);
            m_QGroundAnimDestOld = destinationPart;
            if (t == 1.0f)
                m_QGroundAnimDest = {};
        }
        else
        {
            delta = RelativeTransform(m_QGroundAnimCurrent, ground);
        }
        m_QGroundAnimCurrent = ground;
        quattomat(m_Ground._Mat, *reinterpret_cast<ZQuat*>(delta.m_OffsetQuat));
        m_Ground._Pos = *reinterpret_cast<ZVector3*>(delta.m_OffsetPos);
        if (m_pBoneModify)
            m_pBoneModify->Update(this, m_Ground._Mat, m_Ground._Pos);
        return true;
    }

    bool ZLNKOBJ::UpdatePoseAnimation()
    {
        if (!m_pPoseAnim)
            return false;
        m_pPoseAnim->UpdateFrame();
        return true;
    }

    bool ZLNKOBJ::StartAnim(Animation::ActiveAnimation* pAnimation, int sequenceId)
    {
        (void)pAnimation;
        (void)sequenceId;
        return false;
    }

    bool ZLNKOBJ::IsInElevator() const
    {
        return 0;
    }

    float ZLNKOBJ::GetElevatorDeltaY() const
    {
        return 0.0f;
    }

    bool ZLNKOBJ::WantBloodOnHit() const
    {
        return true;
    }

    void ZLNKOBJ::LoadSaveAnimations(ISerializerStream& stream, bool bSaving)
    {
        for (auto& animation : m_Model->m_ActiveAnims)
            animation.LoadSave(stream, bSaving);
    }

    void ZLNKOBJ::GetAnimCollectionProperty(ZANIM& anim_collection_name)
    {
        static_cast<ZRTString&>(anim_collection_name) = animCollectionName;
    }

    void ZLNKOBJ::SetAnimCollectionProperty(const ZANIM& anim_collection_name)
    {
        animCollectionName = anim_collection_name;

        const char* pszAnimCollectionName = anim_collection_name;
        if (pszAnimCollectionName && *pszAnimCollectionName)
        {
            const auto* pBuffer = LoadAnimVariationsBuffer(pszAnimCollectionName);
            m_AnimVariations.SetBuffer(pBuffer);
        }
    }

#   pragma region "RTTI"
    namespace cProperties
    {
        static RTP::ZVirtualProperty<ZANIM> NamespaceItem_3349 {
            .m_Node = { nullptr, "anmCollection", 1 },
            .m_VirtualTable = VirtualTable_VP__188,
            .m_Get = &ZLNKOBJ::GetAnimCollectionProperty,
            .m_Set = &ZLNKOBJ::SetAnimCollectionProperty
        };

        static RTP::ZDataProperty<ZGEOMREF> NamespaceItem_3348 {
            .m_Node = { NamespaceItem_3349, "m_pAttachedTo", 2 },
            .m_VirtualTable = VirtualTable_DP__5,
            .m_Offset = reinterpret_cast<ZGEOMREF*>(CLASS_PROPERTY(ZLNKOBJ, m_pAttachedTo))
        };

        static RTP::ZDataProperty<bool> NamespaceItem_3347 {
            .m_Node = { NamespaceItem_3348, "m_bHideInThisView", 2 },
            .m_VirtualTable = VirtualTable_DP__1,
            .m_Offset = CLASS_PROPERTY(ZLNKOBJ, m_bHideInThisView)
        };

        static RTP::ZDataProperty<bool> NamespaceItem_3346 {
            .m_Node = { NamespaceItem_3347, "m_bCutSequence", 2 },
            .m_VirtualTable = VirtualTable_DP__1,
            .m_Offset = CLASS_PROPERTY(ZLNKOBJ, m_bCutSequence)
        };

        static RTP::ZDataProperty<ZANIM> NamespaceItem_3345 {
            .m_Node = { NamespaceItem_3346, "animCollectionName", 2 },
            .m_VirtualTable = VirtualTable_DP__188,
            .m_Offset = reinterpret_cast<ZANIM*>(CLASS_PROPERTY(ZLNKOBJ, animCollectionName))
        };

        static RTP::ZDataProperty<uint8_t> NamespaceItem_3344 {
            .m_Node = { NamespaceItem_3345, "m_iVisionID", 2 },
            .m_VirtualTable = VirtualTable_DP__32,
            .m_Offset = CLASS_PROPERTY(ZLNKOBJ, m_iVisionID)
        };

        static RTP::ZDataProperty<ZAUDIOREF> NamespaceItem_3343 {
            .m_Node = { NamespaceItem_3344, "m_SoundMappingMaterial", 1 },
            .m_VirtualTable = VirtualTable_DP__21,
            .m_Offset = CLASS_PROPERTY(ZLNKOBJ, m_SoundMappingMaterial)
        };

        static RTP::ZDataProperty<uint32_t> NamespaceItem_3341 {
            .m_Node = { NamespaceItem_3343, "m_lVariantId", 3 },
            .m_VirtualTable = VirtualTable_DP__6,
            .m_Offset = CLASS_PROPERTY(ZLNKOBJ, m_lVariantId)
        };


        static RTP::ZDataProperty<int> NamespaceItem_3340 {
            .m_Node = {
                .m_Next = NamespaceItem_3341,
                .m_Name = "m_pMotions2BoneCount",
                .m_Filter = 2
            },
            .m_VirtualTable = VirtualTable_DP__7,
            .m_Offset = CLASS_PROPERTY(ZLNKOBJ, m_pMotions2BoneCount)
        };
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZLNKOBJ,
        ZSTDOBJ,
        0x0099C4F8,
        "ZLNKOBJ",
        0x0077037C,
        cProperties::NamespaceItem_3340,
        0x00806A54,
        0x0099C488,
        0x0099C48C
    );
#   pragma endregion
}
