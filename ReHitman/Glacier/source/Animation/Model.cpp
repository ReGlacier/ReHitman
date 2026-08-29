#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/Prim/ZPoseBoneHeader.h>
#include <Glacier/Render/Prim/ZPoseBone.h>
#include <Glacier/Animation/Manager.h>
#include <Glacier/Animation/ZBone.h>
#include <Glacier/Animation/ZBoneQuat.h>
#include <Glacier/Animation/Model.h>
#include <Glacier/Animation/Header.h>
#include <Glacier/Animation/ZHumanState.h>
#include <Glacier/Animation/StreamPacker.h>
#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZSTL/ZMath.h>
#include <numbers>
#include <cstring>
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

    // Computes the state-blending "magic number" that identifies an active animation
    // (matches the magic number used by Model::AnimateQuats).
    int MagicNr(const ActiveAnimation& anim)
    {
        if ((anim.mode & 7) != 2)
            return anim.sequenceId << 16;

        const int lHeaderIndex = anim.header ? Animation::instance->ToIndex(anim.header) : -1;
        return (lHeaderIndex & 0x7FFF) | (((anim.mode >> 8) & 1) << 15);
    }

    // File-scope scratch state buffers used by Model::AnimateState (originally
    // flt_9ACB68 / flt_9AC9C8 / stru_9ACB78).
    ZHumanState g_StateScratch;
    ZHumanState g_StateScratch2;
    ZQuat g_AimQuat;

    // Aim IK arm-base data. Per arm: {pos.x, pos.y, pos.z, factor}; left arm at
    // float offset 16, right arm at float offset 20.
    // TODO: Find where these arm-base positions are populated (the attacker/aim setup).
    float g_AimArmBase[64] = {};
    const int g_AimArmBaseOffsets[2] = { 16, 20 }; // dword_7832E0

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
        ZASSERT((reinterpret_cast<uintptr_t>(buffer) & 0xF) == 0);

        m_Valid = true;
        m_AimDir.x = -2.0f;

        const uint32_t lPrim = pLnkObj->m_baseGeom->m_lPrim;

        m_PelvisPlacement.m_Quat.i = -2.0f;
        m_PelvisPlacement.m_Quat.j = 0.0f;
        m_PelvisPlacement.m_Quat.k = 0.0f;
        m_PelvisPlacement.m_Quat.w = 1.0f;
        m_PelvisPlacement.m_Pos.x = -2.0f;
        m_PelvisPlacementWeight = 0.0f;

        m_Poses.m_PoseIdx = poseIdx;
        m_Bones = pBones;
        m_AngelPose = reinterpret_cast<ZAngelBone*>(const_cast<float*>(ZPrimControlBase::Instance()->GetLocalPrimBonesQuats(lPrim)));
        m_BoneIdToIndexLookup = ZPrimControlBase::GetPrimitive<BoneIndex>(id2IndexIdx);
        m_BoneIndexToIdLookup = ZPrimControlBase::GetPrimitive<BoneID>(index2IdIdx);
        m_BoneCount = boneCount;
        m_Parent = ZPrimControlBase::GetPrimitive<BoneIndex>(parentIdx);
        m_BoneIdToPosLookup = ZPrimControlBase::GetPrimitive<int16_t>(id2PosIdx);

        m_State = nullptr;

        char* lBlendBones = buffer;
        if ((reinterpret_cast<uintptr_t>(lBlendBones) & 0x3F) != 0)
            lBlendBones = lBlendBones - (reinterpret_cast<uintptr_t>(lBlendBones) & 0x3F) + 64;
        m_BlendBones = reinterpret_cast<ZBlendBone*>(lBlendBones);

        float* lCursor = reinterpret_cast<float*>(buffer + 48 * m_BoneCount + 96);

        for (int i = 0; i < m_BoneCount; ++i)
        {
            m_BlendBones[i].m_Quat = m_AngelPose[i].m_Quat;
            m_BlendBones[i].m_Pos = m_AngelPose[i].m_Pos;
            m_BlendBones[i].m_MagicNr = -1;
            m_BlendBones[i].m_Blend = 0.0f;
        }

        m_PoseWeights = nullptr;
        if (m_Poses.m_PoseIdx)
        {
            auto* lPoseHeader = ZPrimControlBase::GetPrimitive<ZPoseBoneHeader>(m_Poses.m_PoseIdx);
            if (lPoseHeader && lPoseHeader->m_PoseCount)
            {
                m_PoseWeights = lCursor;
                lCursor += lPoseHeader->m_PoseCount;
                memset(m_PoseWeights, 0, sizeof(float) * lPoseHeader->m_PoseCount);
            }
        }

        if (m_BoneIndexToIdLookup)
        {
            if (stateModel)
            {
                char* lState = reinterpret_cast<char*>(lCursor);
                if ((reinterpret_cast<uintptr_t>(lState) & 0x3F) != 0)
                    lState = lState - (reinterpret_cast<uintptr_t>(lState) & 0x3F) + 64;
                m_State = znew_placement<ZHumanState>(reinterpret_cast<ZHumanState*>(lState));

                m_StateBlending = reinterpret_cast<ZStateBlending*>(lCursor + 136);
                for (int i = 0; i < 9; ++i)
                    m_StateBlending[i].m_MagicNumber = 0xFFFFFFFFu;

                m_ScaleInfo.m_ArmScale = vlen(m_AngelPose[m_BoneIdToIndexLookup[LClavicle]].m_Pos);
                m_ScaleInfo.m_ArmScale += vlen(m_AngelPose[m_BoneIdToIndexLookup[LUArm]].m_Pos);
                m_ScaleInfo.m_ArmScale += vlen(m_AngelPose[m_BoneIdToIndexLookup[LLArm]].m_Pos);
                m_ScaleInfo.m_ArmScale += vlen(m_AngelPose[m_BoneIdToIndexLookup[LHand]].m_Pos);
                m_ScaleInfo.m_ArmScale += vlen(m_AngelPose[m_BoneIdToIndexLookup[RClavicle]].m_Pos);
                m_ScaleInfo.m_ArmScale += vlen(m_AngelPose[m_BoneIdToIndexLookup[RUArm]].m_Pos);
                m_ScaleInfo.m_ArmScale += vlen(m_AngelPose[m_BoneIdToIndexLookup[RLArm]].m_Pos);
                m_ScaleInfo.m_ArmScale = (vlen(m_AngelPose[m_BoneIdToIndexLookup[RHand]].m_Pos) + m_ScaleInfo.m_ArmScale) * 0.5f;

                m_ScaleInfo.m_HipScale = vlen(m_AngelPose[m_BoneIdToIndexLookup[LLLeg]].m_Pos);
                m_ScaleInfo.m_HipScale += vlen(m_AngelPose[m_BoneIdToIndexLookup[LAnkle]].m_Pos);
                m_ScaleInfo.m_HipScale += vlen(m_AngelPose[m_BoneIdToIndexLookup[RLLeg]].m_Pos);
                m_ScaleInfo.m_HipScale = (vlen(m_AngelPose[m_BoneIdToIndexLookup[RAnkle]].m_Pos) + m_ScaleInfo.m_HipScale) * 0.5f;

                int lIndex = 0;
                for (; lIndex < 4; ++lIndex)
                {
                    if ((m_ActiveAnims[lIndex].mode & 7) != 0 && (m_ActiveAnims[lIndex].mode & 0x8000) != 0)
                        break;
                }

                if (lIndex == 4)
                {
                    const int lBailAt = (m_BoneCount < 30) ? 34 : 74;

                    const int lFullBodyIndex = static_cast<int>(m_LastFullBody.m_Raw << 17) >> 17;
                    if (lFullBodyIndex != -1)
                    {
                        ZASSERT(lFullBodyIndex >= 0 && lFullBodyIndex < Animation::instance->m_Animcount);
                        Header* lHeader = &Animation::instance->m_Headers[lFullBodyIndex];
                        lHeader->DePackState(*m_State, static_cast<float>(lHeader->m_Frames - 1), Animation::instance, lBailAt);
                        if (m_LastFullBody.m_Raw & 0x8000u)
                            m_State->Mirror(lHeader->m_States);
                    }

                    const int lUpperBodyIndex = static_cast<int>(m_LastUpperBody.m_Raw << 17) >> 17;
                    if (lUpperBodyIndex != -1)
                    {
                        ZASSERT(lUpperBodyIndex >= 0 && lUpperBodyIndex < Animation::instance->m_Animcount);
                        Header* lHeader = &Animation::instance->m_Headers[lUpperBodyIndex];
                        lHeader->DePackState(*m_State, static_cast<float>(lHeader->m_Frames - 1), Animation::instance, lBailAt);
                        if (m_LastUpperBody.m_Raw & 0x8000u)
                            m_State->Mirror(lHeader->m_States);
                    }
                }
            }
            else
            {
                m_State = nullptr;
                m_StateBlending = nullptr;
                m_Animated = true;

                int lIndex = 0;
                for (; lIndex < 4; ++lIndex)
                {
                    if ((m_ActiveAnims[lIndex].mode & 7) != 0 && (m_ActiveAnims[lIndex].mode & 0x8000) != 0)
                        break;
                }

                if (lIndex == 4)
                {
                    const int lFullBodyIndex = static_cast<int>(m_LastFullBody.m_Raw << 17) >> 17;
                    m_Animated = false;

                    if (lFullBodyIndex != -1)
                    {
                        ZASSERT(lFullBodyIndex >= 0 && lFullBodyIndex < Animation::instance->m_Animcount);
                        Header* lHeader = &Animation::instance->m_Headers[lFullBodyIndex];

                        mreset(m_Bones[0]._Mat.data);
                        m_Bones[0]._Pos.Reset();

                        for (int i = 1; i < m_BoneCount; ++i)
                        {
                            m_Bones[i]._Quat = m_AngelPose[i].m_Quat;
                            m_Bones[i]._Pos = m_AngelPose[i].m_Pos;
                        }

                        lHeader->DePackQuats(m_BoneIdToPosLookup, reinterpret_cast<ZBoneQuat*>(m_Bones), static_cast<float>(lHeader->m_Frames - 1), Animation::instance);

                        EBoneID lCount;
                        uint8_t* pVectorIds = StreamPacker::GetVectorIds(&Animation::instance->m_Data[lHeader->m_QuatOffset], lCount);

                        for (int i = 0; i < lCount; ++i)
                        {
                            const uint32_t lBoneId = static_cast<uint16_t>(pVectorIds[2 * i]) | (static_cast<uint16_t>(pVectorIds[2 * i + 1]) << 8);
                            const uint8_t lBoneIndex = m_BoneIdToIndexLookup[lBoneId];

                            if (lBoneIndex != 0xFF && lBoneIndex < m_BoneCount)
                            {
                                m_BlendBones[lBoneIndex].m_Quat = m_Bones[lBoneIndex]._Quat;
                                m_BlendBones[lBoneIndex].m_Pos = m_Bones[lBoneIndex]._Pos;
                            }
                        }

                        ModelSpaceBones();
                    }
                }
            }
        }
        else
        {
            m_State = nullptr;
            m_StateBlending = nullptr;
        }
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
            lBufferSize += 0x268; // NOTE: Understand what it means (I guess size of some struct?)
        }

        return lBufferSize + 16 * (3 * boneCount + 6);
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
        if (manager->GetPlayUncompressed())
            return;

        int lMask = 0;

        if (!m_State)
            return;

        memcpy(&g_StateScratch, m_State, sizeof(ZHumanState));

        m_FaceAnimated = false;
        bool lHasAim = false;

        const int lBailAt = (m_BoneCount < 50) ? 34 : 74;

        ZStateBlending lNext[9];
        for (int i = 0; i < 9; ++i)
            lNext[i].m_MagicNumber = 0xFFFFFFFFu;

        for (int i = 0; i < m_OrderSize; ++i)
        {
            const int lSlot = m_DepackOrder[i];
            ActiveAnimation& lAnim = m_ActiveAnims[lSlot];

            if ((lAnim.mode & 7) == 0)
                continue;

            m_Animated = true;

            Header* lHeader = lAnim.header;
            const float lFrame = lAnim.frame;

            if (!lHeader || (lHeader->m_Mask & Header::ZHM_HAS_STATE) == 0)
                continue;

            const int lMode = lAnim.mode;

            if ((lMode & 0xF0) != 0)
            {
                const float lWeight = lFrame <= 1.0f ? lFrame : 1.0f;
                if (lMode & 0x10)
                    m_Targets[3].m_Weight2 = lWeight;
                if (lMode & 0x20)
                    m_Targets[4].m_Weight2 = lWeight;
                if (lMode & 0x40)
                    m_Targets[5].m_Weight2 = lWeight;
                if (lMode & 0x80)
                    m_Targets[6].m_Weight2 = lWeight;
            }

            if ((lMode & 0x800) == 0)
            {
                lHeader->DePackState(g_StateScratch, lFrame, manager, lBailAt);

                if (lBailAt != 34 && lHeader->m_PoseDataOffset != -1 && m_Poses.m_PoseIdx)
                {
                    m_FaceAnimated = true;
                    lHeader->DePackPose(m_Poses.idToPosLookup(), m_PoseWeights, lFrame, manager);
                }

                int lStates = lHeader->m_States;
                if (lMode & 0x100)
                    lStates = g_StateScratch.Mirror(lHeader->m_States);

                lMask |= lStates;

                const int lMagic = MagicNr(lAnim);
                for (int j = 0; j < 9; ++j)
                {
                    if ((1 << j) & lStates)
                    {
                        lNext[j].m_MagicNumber = lMagic;
                        lNext[j].m_BlendTime = lAnim.blend;
                    }
                }
            }
            else
            {
                lHasAim = true;
                lMask |= lHeader->m_States;

                const uint8_t lBone = m_BoneIdToIndexLookup[eStateBoneCount];

                ZQuat lBaseQuat{ 0.0f, 0.0f, 0.0f, 1.0f };

                auto lDepackAim = [&](float lAngleH, float lAngleV)
                {
                    float lCircle1, lCircle2, lBlendPrc;
                    GetAimFrames(lCircle1, lCircle2, lBlendPrc, lAngleH, lAngleV);
                    lHeader->DePackState(g_StateScratch, lCircle2, manager, lBailAt);
                    lHeader->DePackState(g_StateScratch2, lCircle1, manager, lBailAt);
                    g_StateScratch.Blend(&g_StateScratch2, lBlendPrc, lHeader->m_States);
                };

                if (m_Targets[1].m_Data[3] == 2.0f)
                {
                    if (lHeader->m_Frames > 50)
                        lDepackAim(3.1415927f, 1.5707964f);
                    else
                        lHeader->DePackState(g_StateScratch, lFrame, manager, lBailAt);
                }
                else
                {
                    float lSlice = 0.0f;
                    float lAngle = 0.0f;
                    float lDist = 500.0f;

                    ZVector3 lToTarget = m_Targets[1].m_Pos2 - m_Bones[lBone]._Pos;
                    vmtmul(&lToTarget.x, m_Bones[lBone]._Mat.data);

                    if (lHeader->m_Mask & 8)
                    {
                        if (lMask & 1)
                        {
                            lMask &= ~1u;
                            m_State->Blend(1, &g_StateScratch, m_StateBlending, lNext, fDt);
                        }

                        const ZQuat lStateQuat = m_State->m_Quats[0];
                        ZVector3 lUp{ 0.0f, 1.0f, 0.0f };

                        ZQuat lInv = lStateQuat;
                        lInv.i = -lInv.i;
                        lInv.j = -lInv.j;
                        lInv.k = -lInv.k;
                        qtran(&lUp.x, &lInv.i, &lUp.x);

                        if (g_AimBasePelvis)
                            minTransformQuat(&lBaseQuat.i, &lUp.x);

                        ZQuat lAimQuat;
                        qmul(lAimQuat, lStateQuat, lBaseQuat);

                        ZMat3x3 lMat;
                        quattomat(lMat, lAimQuat);

                        ZVector3 lPelvis{ m_State->m_Floats[0], m_State->m_Floats[1], m_State->m_Floats[2] };
                        vscalar(&lPelvis.x, m_ScaleInfo.m_HipScale);

                        ZVector3 lDir;
                        if (m_AimDir.x == -2.0f)
                        {
                            vsub(&lDir.x, &lToTarget.x, &lPelvis.x);
                            lDir.y -= 60.0f;
                            vmtmul(&lDir.x, lMat.data);
                            lDist = vnorm(&lDir.x);
                        }
                        else
                        {
                            lDir.x = -m_AimDir.x;
                            lDir.y = -m_AimDir.y;
                            lDir.z = -m_AimDir.z;
                            vmtmul(&lDir.x, m_Bones[lBone]._Mat.data);
                            vmtmul(&lDir.x, lMat.data);
                            lDist = 1.0f;
                        }

                        lSlice = GetAngle(-lDir.z, -lDir.y);
                        lAngle = std::acos(lDir.x);
                    }
                    else
                    {
                        ZVector3 lDir;
                        lDir.x = -lToTarget.z;
                        lDir.y = -lToTarget.x;
                        lDir.z = lToTarget.y - 180.0f;
                        lDist = vnorm(&lDir.x);
                        lSlice = GetAngle(-lDir.x, lDir.y);
                        lAngle = std::acos(lDir.z);
                    }

                    if (m_Targets[1].m_Weight2 == 0.0f)
                    {
                        m_Targets[1].m_Data[0] = lSlice;
                        m_Targets[1].m_Data[1] = lAngle;
                        m_Targets[1].m_Data[2] = lDist;
                    }
                    else
                    {
                        const float lRate = fDt * 400.0f;
                        PullToValue(m_Targets[1].m_Data[0], lSlice, lRate);
                        PullToValue(m_Targets[1].m_Data[1], lAngle, lRate);
                        PullToValue(m_Targets[1].m_Data[2], lDist, fDt * 5000.1001f);
                        lSlice = m_Targets[1].m_Data[0];
                        lAngle = m_Targets[1].m_Data[1];
                    }

                    m_Targets[1].m_Weight2 = 1.0f;

                    float lVert = lAngle + m_Targets[1].m_Data[4];
                    float lHorz = lSlice + m_Targets[1].m_Data[5];

                    if (lVert < 0.0f)
                        lVert = 0.0f;
                    else if (lVert > 3.1415927f)
                        lVert = 3.1415927f;

                    if (lHorz < 0.0f)
                        lHorz = 0.0f;
                    else if (lHorz > 6.2831855f)
                        lHorz = 6.2831855f;

                    if (!g_UseNewAim)
                    {
                        lDepackAim(lHorz, lVert);
                    }
                    else
                    {
                        if (lVert < 0.62831855f)
                            lVert = 0.62831855f;
                        else if (lVert > 2.8274333f)
                            lVert = 2.8274333f;

                        if (lHorz >= 1.5707964f && lHorz <= 4.712389f)
                        {
                            if (lHeader->m_Frames > 50)
                                lDepackAim(3.1415927f, 1.5707964f);
                            else
                                lHeader->DePackState(g_StateScratch, lFrame, manager, lBailAt);

                            const float lVertDelta = lVert - 1.5707964f;
                            const float lArmFac = g_AimIkFac * lVertDelta;

                            ZQuat lRotX;
                            {
                                const float h = (lHorz - 3.1415927f) * 0.5f;
                                lRotX.i = -std::sin(h);
                                lRotX.j = 0.0f;
                                lRotX.k = 0.0f;
                                lRotX.w = std::cos(h);
                            }

                            ZQuat lRotY1;
                            {
                                const float h = lVertDelta * (1.0f - g_AimIkFac) * 0.5f;
                                lRotY1.i = 0.0f;
                                lRotY1.j = -std::sin(h);
                                lRotY1.k = 0.0f;
                                lRotY1.w = std::cos(h);
                            }

                            const ZQuat lOldAimQuat = g_AimQuat;

                            ZQuat lCombined;
                            qmul(lCombined, lRotX, lRotY1);

                            ZQuat lAimQuat;
                            qmul(lAimQuat, lCombined, lOldAimQuat);
                            qmul(g_AimQuat, lBaseQuat, lAimQuat);

                            ZQuat lRotY2;
                            {
                                const float h = lArmFac * 0.5f;
                                lRotY2.i = 0.0f;
                                lRotY2.j = -std::sin(h);
                                lRotY2.k = 0.0f;
                                lRotY2.w = std::cos(h);
                            }

                            // pQuat1 = inverse(lOldAimQuat) * lRotY2 * lOldAimQuat
                            ZQuat lInvAim{ -lOldAimQuat.i, -lOldAimQuat.j, -lOldAimQuat.k, lOldAimQuat.w };
                            ZQuat lPivotQuat;
                            qmul(lPivotQuat, lInvAim, lRotY2);
                            qmul(lPivotQuat, lPivotQuat, lOldAimQuat);

                            const ZVector3 lPivot{ 0.2f, -0.2f, 0.05f };

                            const bool lLeftArm = (lMode & 0x800000) == 0 ? (lHeader->m_States & 0x20) != 0 : false;
                            const bool lRightArm = (lMode & 0x1000000) == 0 ? (lHeader->m_States & 0x40) != 0 : false;

                            for (int j = 0; j < 2; ++j)
                            {
                                if (!(j == 0 ? lLeftArm : lRightArm))
                                    continue;

                                const int lOff = g_AimArmBaseOffsets[j];

                                ZVector3 lPos;
                                vcpy(&lPos.x, &g_AimArmBase[lOff]);
                                vsub(&lPos.x, &lPivot.x);

                                ZVector3 lRotated;
                                qtran(&lRotated.x, &lPivotQuat.i, &lPos.x);
                                vadd(&lRotated.x, &lPivot.x);

                                vcpy(&g_AimArmBase[lOff], &lRotated.x);
                                g_AimArmBase[lOff + 3] -= lArmFac * _g_AimArmFac;
                            }
                        }
                    }
                }

                const int lMagic = MagicNr(lAnim);
                for (int j = 0; j < 9; ++j)
                {
                    if ((1 << j) & lHeader->m_States)
                    {
                        lNext[j].m_MagicNumber = lMagic;
                        lNext[j].m_BlendTime = lAnim.blend;
                    }
                }
            }

            if (lMode & 8)
            {
                lHeader->DePackState(g_StateScratch, lFrame, manager, lBailAt);

                Header* lDualHeader = m_ActiveAnims[lSlot + 1].header;
                lDualHeader->DePackState(g_StateScratch2, m_ActiveAnims[lSlot + 1].frame, manager, lBailAt);
                g_StateScratch.Blend(&g_StateScratch2, 1.0f - m_ActiveAnims[lSlot + 1].blend, lDualHeader->m_States);

                lMask |= lDualHeader->m_States;

                for (int j = 0; j < 9; ++j)
                {
                    if ((1 << j) & lDualHeader->m_States)
                        lNext[j].m_MagicNumber += 4999 * Animation::instance->ToIndex(lDualHeader);
                }
            }
        }

        if (!lHasAim)
            m_Targets[1].m_Weight2 = 0.0f;

        m_State->Blend(lMask, &g_StateScratch, m_StateBlending, lNext, fDt);
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

            // Convert local quat of current bone to its matrix
            quattomat(bone._Mat, bone._Quat);

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

    void Model::ResolveStaticResourceRefs()
    {
        m_EyePoseId[0] = Animation::instance->GetPoseID("E_AimL_Left");
        m_EyePoseId[1] = Animation::instance->GetPoseID("E_AimL_Right");
        m_EyePoseId[2] = Animation::instance->GetPoseID("E_AimL_Down");
        m_EyePoseId[3] = Animation::instance->GetPoseID("E_AimL_Up");
        m_EyePoseId[4] = Animation::instance->GetPoseID("E_AimR_Left");
        m_EyePoseId[5] = Animation::instance->GetPoseID("E_AimR_Right");
        m_EyePoseId[6] = Animation::instance->GetPoseID("E_AimR_Down");
        m_EyePoseId[7] = Animation::instance->GetPoseID("E_AimR_Up");

        m_EyePoseIdOk = 1;
        for (int i = 0; i < 8; ++i)
        {
            if (m_EyePoseId[i] == ePoseIDNA)
                m_EyePoseIdOk = 0;
        }
    }

    void Model::GetAimFrames(float& fCircle1Prc, float& fCircle2Prc, float& fBlendPrc, float fAngleHorz, float fAngleVert)
    {
        const float fHorz = std::clamp(fAngleHorz, 0.f, std::numbers::pi_v<float> * 2.0f);
        float fVert = std::clamp(fAngleVert, 0.f, std::numbers::pi_v<float>);

        const float fCircle = fHorz * 0.15915494f * 64.0f;
        fCircle1Prc = fCircle;
        fCircle2Prc = fCircle;

        if (fVert > 0.78539819f)
        {
            if (fVert > 1.5707964f)
            {
                if (fVert > 2.3561945f)
                {
                    if (fVert > 2.8797934f)
                        fVert = 2.8797934f;
                    fCircle1Prc += 65.0f;
                    fCircle2Prc += 260.0f;
                    fBlendPrc = 1.0f - (fVert - 2.3561945f) * 1.9098593f;
                }
                else
                {
                    fCircle2Prc += 65.0f;
                    fBlendPrc = 1.0f - (fVert - 1.5707964f) * 1.2732395f;
                }
            }
            else
            {
                fCircle2Prc += 130.0f;
                fBlendPrc = (fVert - 0.78539819f) * 1.2732395f;
            }
        }
        else
        {
            if (fVert < 0.2617994f)
                fVert = 0.2617994f;
            fCircle1Prc += 130.0f;
            fCircle2Prc += 195.0f;
            fBlendPrc = (fVert - 0.2617994f) * 1.9098593f;
        }

        if (fBlendPrc >= 0.0f)
        {
            if (fBlendPrc > 1.0f)
                fBlendPrc = 1.0f;
        }
        else
        {
            fBlendPrc = 0.0f;
        }
    }

    STATIC_CLASS_VAR_IMPL(Model, int, m_EyePoseIdOk, 0x009AC9A8, 0);
    STATIC_CLASS_VAR_ARRAY_IMPL(Model, int, m_EyePoseId, 8, 0x009A3DB4);
    STATIC_CLASS_VAR_IMPL(Model, float, g_YCEN, 0x007FEB5C, 20.0f);
    STATIC_CLASS_VAR_IMPL(Model, int, g_UseNewAim, 0x007FEB60, 1);
    STATIC_CLASS_VAR_IMPL(Model, float, g_EyeLookAtHor, 0x007FEB64, 60.0f);
    STATIC_CLASS_VAR_IMPL(Model, float, g_EyeLookAtVer, 0x007FEB68, 17.0f);
    STATIC_CLASS_VAR_IMPL(Model, float, g_AimIkFac, 0x007FEB70, 0.5f);
    STATIC_CLASS_VAR_IMPL(Model, int, g_AimBasePelvis, 0x007FEB74, 1);
    STATIC_CLASS_VAR_IMPL(Model, float, _g_AimArmFac, 0x007FEB78, 1.0f);
}
