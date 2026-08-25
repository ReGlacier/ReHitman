#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/Prim/ZPoseBoneHeader.h>
#include <Glacier/Render/Prim/ZPoseBone.h>
#include <Glacier/Animation/Manager.h>
#include <Glacier/Animation/ZBone.h>
#include <Glacier/Animation/Model.h>
#include <Glacier/Animation/Header.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdio>


namespace Glacier::Animation
{
    namespace
    {
        void RestrictToCone(float* dir, float r1, float r2)
        {
            const float xx = dir[0] * dir[0];
            const float yy = dir[1] * dir[1];
            const float zz = dir[2] * dir[2];
            const float fMaxSqrZ = xx * r1 + yy * r2;

            if (dir[2] <= 0.0f || fMaxSqrZ > zz)
            {
                const float fOldLenSq = xx + yy + zz;
                dir[2] = std::sqrt(fMaxSqrZ);
                const float fNewLenSq = xx + yy + fMaxSqrZ;
                const float fScale = std::sqrt(fOldLenSq / fNewLenSq);
                vscalar(dir, fScale);
            }
        }

        // Swaps the left/right hand-attacher bones (58/59) and mirrors them.
        void MirrorHandBones(Model* pModel)
        {
            const uint8_t i58 = pModel->m_BoneIdToIndexLookup[eLHandAttacher];
            const uint8_t i59 = pModel->m_BoneIdToIndexLookup[eRHandAttacher];

            if (i58 == 0xFF || i59 == 0xFF)
                return;

            ZBone lTemp = pModel->m_Bones[i58];
            pModel->m_Bones[i58] = pModel->m_Bones[i59];
            pModel->m_Bones[i59] = lTemp;

            const ZQuat lMirrorQuat{ 1.0f, 0.0f, 0.0f, 0.0f };

            pModel->m_Bones[i58]._Quat.j = -pModel->m_Bones[i58]._Quat.j;
            pModel->m_Bones[i58]._Quat.w = -pModel->m_Bones[i58]._Quat.w;
            pModel->m_Bones[i58]._Pos.y = -pModel->m_Bones[i58]._Pos.y;
            qmul(pModel->m_Bones[i58]._Quat, pModel->m_Bones[i58]._Quat, lMirrorQuat);

            pModel->m_Bones[i59]._Quat.j = -pModel->m_Bones[i59]._Quat.j;
            pModel->m_Bones[i59]._Quat.w = -pModel->m_Bones[i59]._Quat.w;
            pModel->m_Bones[i59]._Pos.y = -pModel->m_Bones[i59]._Pos.y;
            qmul(pModel->m_Bones[i59]._Quat, pModel->m_Bones[i59]._Quat, lMirrorQuat);
        }
    }

    Model::Model()
    {
        for (int i = 3; i != -1; --i)
        {
            m_ActiveAnims[i] = ActiveAnimation();
        }

        for (int i = 6; i != -1; --i)
        {
            m_Targets[i] = IKTarget();
        }

        m_BlendAnim = ActiveAnimation();

        m_Animated = false;
        m_Valid = false;
        m_LastFullBody.m_Raw |= 0x7FFFu;
        m_LastUpperBody.m_Raw |= 0x7FFFu;
        m_BlendBones = nullptr;
        m_BoneIdToIndexLookup = nullptr;
        m_PoseWeights = 0;
        m_PoseIdToPosLookup = 0;
        m_Banking[0] = 0.0;
        m_Banking[1] = 0.0;
        m_BlendAnim.header = 0;
        m_LastAnim.header = 0;
        m_State = 0;
        m_StateStore = 0;
        m_BoneIdToPosLookup = 0;
        m_State = 0;
        m_StateBlending = 0;
    }

    void Model::Init(ZLNKOBJ* pLnkObj, ZBone* pBones, uint32_t poseIdx, uint32_t id2IndexIdx, uint32_t index2IdIdx, uint32_t id2PosIdx, uint32_t parentIdx, bool stateModel, char* buffer, int boneCount)
    {
        // TODO: Finish me
    }

    int Model::DynamicSize(ZLNKOBJ* pLnkObj, uint32_t poseIdx, uint32_t id2IndexIdx, uint32_t index2IdIdx, uint32_t id2PosIdx, uint32_t parentIdx, bool stateModel, int boneCount)
    {
        int32_t lBufferSize = 0;
        m_Poses.m_PoseIdx = poseIdx;
        m_BoneIndexToIdLookup = ZPrimControlBase::GetPrimitive<BoneID>(index2IdIdx);

        if (poseIdx)
        {
            auto* hdr = ZPrimControlBase::GetPrimitive<ZPoseBoneHeader>(poseIdx);
            if (hdr && hdr->m_PoseCount)
            {
                lBufferSize = sizeof(uint32_t) * m_Poses.poseCount();
            }
        }

        if (m_BoneIndexToIdLookup && stateModel)
        {
            lBufferSize += 0x268; // TODO: Understand what it means (I guess size of some struct?)
        }

        return lBufferSize + 16 * (3 * boneCount + 9);
    }

    int Model::DepackOrder(uint8_t* order)
    {
        int lPriority[4];
        int lCount = 0;

        for (int i = 0; i < 4; ++i)
        {
            const int32_t lMode = m_ActiveAnims[i].mode;

            if ((lMode & 7) != 0)
            {
                order[lCount] = static_cast<uint8_t>(i);
                lPriority[lCount] = (lMode & 0x70000) + ((lMode & 0x8000) ? 0 : 4);
                ++lCount;

                // A dual animation occupies two consecutive slots; skip its second half.
                if ((lMode & 8) != 0)
                    ++i;
            }
        }

        // Bubble sort the active slots by ascending blend priority.
        int lEnd = lCount;
        bool lSwapped;
        do
        {
            lSwapped = false;
            --lEnd;

            for (int j = 0; j < lEnd; ++j)
            {
                if (lPriority[j] > lPriority[j + 1])
                {
                    const uint8_t lTmpOrder = order[j];
                    order[j] = order[j + 1];
                    order[j + 1] = lTmpOrder;

                    const int lTmpPriority = lPriority[j];
                    lPriority[j] = lPriority[j + 1];
                    lPriority[j + 1] = lTmpPriority;

                    lSwapped = true;
                }
            }
        }
        while (lSwapped);

        return lCount;
    }

    void Model::PostAnim(float _)
    {
        for (int i = 0; i < 4; ++i)
        {
            if ((m_ActiveAnims[i].mode & 0x400000u) == 0 && (m_ActiveAnims[i].mode & 7) == 2)
            {
                m_ActiveAnims[i].mode ^= 2;
            }
        }
    }

    void Model::PrepareAnim()
    {
        m_OrderSize = DepackOrder(m_DepackOrder);
    }

    void Model::PoseRotationAndTranslation()
    {
        auto* aIndexLUT = m_Poses.indexToOffsetLookup();
        auto lPoseCount = m_Poses.poseCount();
        auto pPoseData = m_Poses.poseData();

        float* lPoseWeights = m_PoseWeights;
        float* const lPoseWeightsEnd = m_PoseWeights + lPoseCount;

        for (; lPoseWeights != lPoseWeightsEnd; ++lPoseWeights, ++aIndexLUT)
        {
            const float lWeight = *lPoseWeights;
            if (lWeight == 0.0f)
                continue;

            const float lScale = lWeight + lWeight;

            const ZPoseBone* lPoseBone = &pPoseData[aIndexLUT[0]];
            const ZPoseBone* const lPoseBoneEnd = &lPoseBone[aIndexLUT[1] - aIndexLUT[0]];

            for (; lPoseBone < lPoseBoneEnd; ++lPoseBone)
            {
                const int32_t lBoneIndex = lPoseBone->m_BoneIndex;
                ZASSERT(lBoneIndex >= 0);

                if (lBoneIndex >= m_BoneCount)
                    continue;

                vaddscalar(m_Bones[lBoneIndex]._Pos, m_Bones[lBoneIndex]._Pos, lPoseBone->m_Pos, lScale);

                if (lPoseBone->m_Quat.w != 0.0f)
                {
                    const float lAngle = lScale * lPoseBone->m_Quat.w;
                    const float lSin = std::sin(lAngle);

                    ZQuat lQuat;
                    lQuat.i = lSin * lPoseBone->m_Quat.i;
                    lQuat.j = lSin * lPoseBone->m_Quat.j;
                    lQuat.k = lSin * lPoseBone->m_Quat.k;
                    lQuat.w = std::cos(lAngle);

                    qmul(m_Bones[lBoneIndex]._Quat, m_Bones[lBoneIndex]._Quat, lQuat);
                }
            }
        }
    }

    void Model::AnimateState(Manager* manager, float fDt)
    {
        // TODO: Finish me
    }

    void Model::PrintDebugInfo()
    {
        if (!Animation::printDebugInfo)
            return;

        printf("------------------------\n");

        if (m_BlendAnim.header)
        {
            printf("blend: mode=%Xh blend=%.2f, %6.2f/%.1f-%.1f/%d%s%s%s (%s)\n",
                m_BlendAnim.mode,
                m_BlendAnim.blend,
                m_BlendAnim.frame,
                m_BlendAnim.startFrame,
                m_BlendAnim.endFrame,
                m_BlendAnim.header->m_Frames - 1,
                (m_BlendAnim.mode & 7) == 1 ? " LOOP" : "",
                (m_BlendAnim.mode & 7) == 2 ? " FRM" : "",
                (m_BlendAnim.mode & 0x100) ? " MIRROR" : "",
                m_BlendAnim.header->m_Name);
        }
        else
        {
            printf("blend: none\n");
        }

        bool lHasActive = false;
        for (int i = 0; i < 4; ++i)
        {
            if ((m_ActiveAnims[i].mode & 7) != 0)
            {
                lHasActive = true;
                break;
            }
        }

        if (!lHasActive)
        {
            printf("NO ACTIVE ANIMATIONS!\n");
        }

        for (int i = 0; i < m_OrderSize; ++i)
        {
            const int lSlot = m_DepackOrder[i];
            const ActiveAnimation& lAnim = m_ActiveAnims[lSlot];

            if ((lAnim.mode & 7) != 0)
            {
                printf("anim slot=%i: mode=%Xh blend=%.2f, %6.2f/%.1f-%.1f/%d%s%s%s%s (%s)\n",
                    lSlot,
                    lAnim.mode,
                    lAnim.blend,
                    lAnim.frame,
                    lAnim.startFrame,
                    lAnim.endFrame,
                    lAnim.header->m_Frames - 1,
                    (lAnim.mode & 7) == 1 ? " LOOP" : "",
                    (lAnim.mode & 7) == 2 ? " FRM" : "",
                    (lAnim.mode & 0x100) ? " MIRROR" : "",
                    (lAnim.mode & 0x200) ? " ANIMDEST" : "",
                    lAnim.header->m_Name);
            }
        }
    }

    void Model::StateFit(ZAngelBone* pAngelBone)
    {
        // TODO: Finish me
    }

    void Model::BlendOutPoseWeights()
    {
        float* lPoseWeights = m_PoseWeights;
        float* const lEnd = m_PoseWeights + m_Poses.poseCount();
        const float lFade = g_pSysInterface->DeltaFrameTime * 5.0f;

        for (; lPoseWeights != lEnd; ++lPoseWeights)
        {
            if (*lPoseWeights > 0.0f)
            {
                *lPoseWeights -= lFade;
                if (*lPoseWeights < 0.0f)
                    *lPoseWeights = 0.0f;
            }
            else if (*lPoseWeights < 0.0f)
            {
                *lPoseWeights += lFade;
                if (*lPoseWeights > 0.0f)
                    *lPoseWeights = 0.0f;
            }
        }
    }

    void Model::BlendQuats()
    {
        const float lFade = g_pSysInterface->DeltaFrameTime * 25.0f;

        for (int i = 1; i < m_BoneCount; ++i)
        {
            ZBlendBone& lBlendBone = m_BlendBones[i];
            ZBone& lBone = m_Bones[i];

            ZASSERT(lBlendBone.m_Blend >= 0.0f);

            const int32_t lBoneMagic = *reinterpret_cast<const int32_t*>(&lBone._Mat.data[8]);
            if (lBlendBone.m_MagicNr != lBoneMagic)
            {
                lBlendBone.m_Blend = (lBlendBone.m_MagicNr == -1) ? 0.0f : lBone._Mat.data[7];
                lBlendBone.m_MagicNr = lBoneMagic;
            }

            if (lFade < lBlendBone.m_Blend)
            {
                const float t = lFade / lBlendBone.m_Blend;

                ZQuat lQuat;
                qpul(lQuat, lBlendBone.m_Quat, lBone._Quat, t);

                ZVector3 lPos;
                lPos.x = lBlendBone.m_Pos.x + (lBone._Pos.x - lBlendBone.m_Pos.x) * t;
                lPos.y = lBlendBone.m_Pos.y + (lBone._Pos.y - lBlendBone.m_Pos.y) * t;
                lPos.z = lBlendBone.m_Pos.z + (lBone._Pos.z - lBlendBone.m_Pos.z) * t;

                qnorm(lQuat);

                lBone._Quat = lQuat;
                lBone._Pos = lPos;

                lBlendBone.m_Blend -= lFade;
                if (lBlendBone.m_Blend < 0.0f)
                    lBlendBone.m_Blend = 0.0f;
            }

            lBlendBone.m_Quat = lBone._Quat;
            lBlendBone.m_Pos = lBone._Pos;
        }
    }

    void Model::ResetBones()
    {
        if (m_BoneCount <= 1)
        {
            return;
        }

        for (int i = 1; i < m_BoneCount; ++i)
        {
            const ZBlendBone& srcBone = m_BlendBones[i];
            ZBone& dstBone = m_Bones[i];

            dstBone._Quat = srcBone.m_Quat;
            dstBone._Pos  = srcBone.m_Pos;
        }
    }

    void Model::AnimateQuats(Manager* manager)
    {
        PrintDebugInfo();

        uint32_t lAnimatedBones = 0;

        for (int i = 0; i < m_OrderSize; ++i)
        {
            const int lSlot = m_DepackOrder[i];
            ActiveAnimation& lAnim = m_ActiveAnims[lSlot];

            if ((lAnim.mode & 7) == 0)
                continue;

            m_Animated = true;

            Header* pHeader = lAnim.header;
            if (manager->m_PlayUncompressed)
                ++pHeader;

            if ((pHeader->m_Mask & Header::ZHM_HAS_QUATS) != 0)
            {
                int32_t lMagicNr;
                if ((lAnim.mode & 7) != 2)
                {
                    lMagicNr = lAnim.sequenceId << 16;
                }
                else
                {
                    const int lHeaderIndex = lAnim.header ? Animation::instance->ToIndex(lAnim.header) : -1;
                    lMagicNr = (lHeaderIndex & 0x7FFF) | (((lAnim.mode >> 8) & 1) << 15);
                }

                if ((lAnim.mode & 0x100) != 0)
                    MirrorHandBones(this);

                pHeader->DePackQuats(m_BoneIdToPosLookup, reinterpret_cast<ZBoneQuat*>(m_Bones), lAnim.frame, manager);

                if ((lAnim.mode & 0x100) != 0)
                    MirrorHandBones(this);

                EBoneID lCount;
                uint8_t* pVectorIds = StreamPacker::GetVectorIds(&manager->m_Data[pHeader->m_QuatOffset], lCount);

                for (int j = 0; j < lCount; ++j)
                {
                    uint32_t lBoneId = static_cast<uint16_t>(pVectorIds[2 * j]) | (static_cast<uint16_t>(pVectorIds[2 * j + 1]) << 8);

                    if ((lAnim.mode & 0x100) != 0)
                    {
                        if (lBoneId == 58)
                            lBoneId = 59;
                        else if (lBoneId == 59)
                            lBoneId = 58;
                    }

                    const uint8_t lIndex = m_BoneIdToIndexLookup[lBoneId];
                    if (lIndex == 0xFF)
                        continue;

                    if (lBoneId == 58)
                        lAnimatedBones |= 1;
                    else if (lBoneId == 59)
                        lAnimatedBones |= 2;

                    ZBone& lBone = m_Bones[lIndex];
                    lBone._Mat.data[8] = *reinterpret_cast<const float*>(&lMagicNr);
                    lBone._Mat.data[7] = lAnim.blend;
                    ZASSERT(lAnim.blend >= 0.0f);
                }
            }
            else if ((pHeader->m_Mask & Header::ZHM_HAS_BONES) != 0)
            {
                pHeader->DePackUncompressed(m_BoneIdToIndexLookup, reinterpret_cast<ZBoneQuat*>(m_Bones), lAnim.frame, manager);
            }
        }

        if (m_OrderSize && m_State)
        {
            if ((lAnimatedBones & 1) == 0)
            {
                const uint8_t i58 = m_BoneIdToIndexLookup[58];
                if (i58 != 0xFF)
                {
                    m_Bones[i58]._Pos = m_AngelPose[i58].m_Pos;
                    m_Bones[i58]._Quat = m_AngelPose[i58].m_Quat;
                }
            }

            if ((lAnimatedBones & 2) == 0)
            {
                const uint8_t i59 = m_BoneIdToIndexLookup[59];
                if (i59 != 0xFF)
                {
                    m_Bones[i59]._Pos = m_AngelPose[i59].m_Pos;
                    m_Bones[i59]._Quat = m_AngelPose[i59].m_Quat;
                }
            }
        }
    }

    void Model::ModelSpaceBones()
    {
        // Root bone must be converted into mat form
        quattomat(m_Bones[1]._Mat, m_Bones[1]._Quat);

        // ^ Bone #0 is ROOT bone without any transform
        // ^ Bone #1 is pelvis/skel/main root bone

        // Other bones
        for (int i = 2; i < m_BoneCount; ++i)
        {
            ZBone& bone = m_Bones[i];

            // Take parent bone index for i-bone
            uint8_t parentIndex = m_Parent[i];
            ZBone& parentBone = m_Bones[parentIndex];

            // Convert local quat of current bone to it's matrix
            quattomat(bone._Mat, parentBone._Quat);

            // Multiply rotations mat by mat
            mmmul(bone._Mat, parentBone._Mat);

            // Translate bone position
            vmmul(bone._Pos, parentBone._Mat);
            vadd(bone._Pos, parentBone._Pos);
        }
    }

    void Model::LookAt(ZAngelBone* pAngelBone, Manager* pManager, float fDt)
    {
        // TODO: Finish me
    }

    void Model::EyeLookAt(ZAngelBone* pAngelBone, Manager* pManager, float fDt)
    {
        // TODO: Finish me
    }

    void Model::Bank(float fDt)
    {
        if (m_Banking[0] == 0.0f && m_Banking[1] == 0.0f)
        {
            return;
        }

        ZVector3 vLean;
        vLean.x = m_Banking[0];
        vLean.y = 0.0f;
        vLean.z = m_Banking[1];

        vmtmul(&vLean.x, m_Bones[0]._Mat.data);
        vLean.y = 0.0f;

        const float lLen = std::sqrt(vLean.z * vLean.z + vLean.x * vLean.x);

        if (lLen >= 0.1f)
        {
            float lClamped = lLen;
            if (lClamped > 30.0f)
            {
                lClamped = 30.0f;
                vsetlen(&vLean.x, 30.0f);
            }

            const float lAngle = lClamped * 0.004f;
            const float lSin = std::sin(lAngle) / lClamped;

            ZQuat vBanking;
            vBanking.i = vLean.z * lSin;
            vBanking.j = 0.0f;
            vBanking.k = -(lSin * vLean.x);
            vBanking.w = std::cos(lAngle);

            ZVector3 v39;
            v39.x = -vLean.x;
            v39.y = 0.0f;
            v39.z = -vLean.z;

            ZQuat lStateQuat = m_State->m_Quats[0];
            qmul(m_State->m_Quats[0], vBanking, lStateQuat);

            const float lInvHipScale = 1.0f / m_ScaleInfo.m_HipScale;
            m_State->m_Floats[1] -= lClamped * 0.0003f;
            qtran(&m_State->m_Floats[0], &vBanking.i, &m_State->m_Floats[0]);
            vaddscalar(&m_State->m_Floats[0], &m_State->m_Floats[0], &v39.x, lInvHipScale);

            if (vLean.x != 0.0f)
            {
                const float l19 = (vLean.x < 0.0f) ? vLean.x - 2.0f : vLean.x + 2.0f;
                const float l20 = 1000.0f / l19;
                const float lYCEN = Model::g_YCEN;

                float l31 = l20;
                float l34 = l20;
                if (l20 < 0.0f)
                    l31 = l20 - 20.0f;
                else
                    l34 = l20 + 20.0f;

                const float l24 = 1.0f / std::fabs(l20);
                const float l25 = (m_State->m_Floats[7] - lYCEN) * l24;

                if (m_State->m_Floats[7] < lYCEN)
                {
                    m_State->m_Floats[6] = l31 - std::cos(l25) * l31 + m_State->m_Floats[6];
                    m_State->m_Floats[7] = std::sin(l25) * std::fabs(l31) + lYCEN;
                }

                const float l26 = l24 * (m_State->m_Floats[12] - lYCEN);
                if (m_State->m_Floats[12] < lYCEN)
                {
                    m_State->m_Floats[11] = l34 - std::cos(l26) * l34 + m_State->m_Floats[11];
                    m_State->m_Floats[12] = std::sin(l26) * std::fabs(l34) + lYCEN;
                }
            }

            const float l27 = (1.0f - m_State->m_Floats[8] * 0.014285714f) * 1.5f;
            m_State->m_Floats[6] = v39.x * l27 + m_State->m_Floats[6];
            m_State->m_Floats[7] = l27 * v39.z + m_State->m_Floats[7];

            const float l28 = (1.0f - m_State->m_Floats[13] * 0.014285714f) * 1.5f;
            m_State->m_Floats[11] = v39.x * l28 + m_State->m_Floats[11];
            m_State->m_Floats[12] = l28 * v39.z + m_State->m_Floats[12];
        }
    }

    STATIC_CLASS_VAR_IMPL(Model, int, m_EyePoseIdOk, 0x009AC9A8, 0);
    STATIC_CLASS_VAR_IMPL(Model, int, m_EyePoseId, 0x009A3DB4, 0);
    STATIC_CLASS_VAR_IMPL(Model, float, g_YCEN, 0x007FEB5C, 20.0f);
    STATIC_CLASS_VAR_IMPL(Model, int, g_UseNewAim, 0x007FEB60, 1);
    STATIC_CLASS_VAR_IMPL(Model, float, g_EyeLookAtHor, 0x007FEB64, 60.0f);
    STATIC_CLASS_VAR_IMPL(Model, float, g_EyeLookAtVer, 0x007FEB68, 17.0f);
    STATIC_CLASS_VAR_IMPL(Model, float, g_AimIkFac, 0x007FEB70, 0.5f);
    STATIC_CLASS_VAR_IMPL(Model, int, g_AimBasePelvis, 0x007FEB74, 1);
    STATIC_CLASS_VAR_IMPL(Model, float, _g_AimArmFac, 0x007FEB78, 1.0f);
}
