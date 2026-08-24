#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/Prim/ZPoseBoneHeader.h>
#include <Glacier/Render/Prim/ZPoseBone.h>
#include <Glacier/Animation/ZBone.h>
#include <Glacier/Animation/Model.h>


namespace Glacier::Animation
{
    namespace
    {
        void Animation_Transform(ZBone* pDest, const ZBone* pSrc)
        {
            ZASSERT((((uint32_t)&pSrc->_Mat)&15)==0); // memalign for SIMD
            ZASSERT((((uint32_t)&pDest->_Mat)&15)==0);; // memalign for SIMD

            // TODO: Finish me
            // DronCode: It's really huge SIMD method to convert bone during animation
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
        m_Banking.x = 0.0;
        m_Banking.y = 0.0;
        m_BlendAnim.header = 0;
        m_LastAnim.header = 0;
        m_State = 0;
        m_StateStore = 0;
        m_BoneIdToPosLookup = 0;
        m_State = 0;
        m_StateBlending = 0;
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
        // TODO: Finish me
        return 0;
    }

    void Model::PostAnim(float fUnused)
    {
        // TODO: Finish me
    }

    void Model::PrepareAnim()
    {
        // TODO: Finish me
    }

    void Model::PoseRotationAndTranslation()
    {
        // TODO: Finish me
    }

    void Model::AnimateState(Manager* pManager, float)
    {
        // TODO: Finish me
    }

    void Model::PrintDebugInfo()
    {
        // TODO: Finish me
    }

    void Model::StateFit(ZAngelBone* pAngelBone)
    {
        // TODO: Finish me
    }

    void Model::BlendOutPoseWeights()
    {
        // TODO: Finish me
    }

    void Model::BlendQuats()
    {
        // TODO: Finish me
    }

    void Model::ResetBones()
    {
        // TODO: Finish me
    }

    void Model::AnimateQuats(Manager* pManager)
    {
        // TODO: Finish me
    }

    void Model::ModelSpaceBones()
    {
        // Convert quat of bone #1 into rot matrix (in-place)
        quattomat(m_Bones[1]._Mat, m_Bones[1]._Mat);

        // Iterate over all other bones (starting from 2)
        for (int i = 2; i < m_BoneCount; ++i)
        {
            ZBone& bone = m_Bones[i];

            // Take parent bone index for i-bone
            uint8_t parentIndex = m_Parent[i];
            ZBone& parentBone = m_Bones[parentIndex];

            // Convert quat from current bone to matrix
            quattomat(bone._Mat, bone._Mat);

            // Transform current bone to parent bone (local space -> model space)
            Animation_Transform(&bone, &parentBone);
        }
    }

    void Model::LookAt(ZAngelBone* pAngelBone, Manager* pManager, float)
    {
        // TODO: Finish me
    }

    void Model::Bank(float fDt)
    {
        // TODO: Finish me
    }
}
