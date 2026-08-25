#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Animation/EBoneID.h>
#include <cstdint>


namespace Glacier
{
    class ZBoneQuat;
    class ZHumanState;
}

namespace Glacier::Animation
{
    // fwds
    class Manager;

    struct Header
    {
        // constants
        static constexpr int32_t ZHM_UNCOMPRESSED = 0x10; // Related to m_Mask
        static constexpr int32_t ZHM_HAS_QUATS = 0x2;
        static constexpr int32_t ZHM_HAS_STATE = 0x4;
        static constexpr int32_t ZHM_HAS_BONES = 0x10;
        static constexpr int32_t ZHM_HAS_POSES = 0x20;
        static constexpr float TIME_SCALE =  0.039999999f;

        // methods
        static Header* RefToPtr(int rRef);

        int GetRef();
        int Length();
        int32_t GetSoundIndex() const;
        float GetBlendTime() const;
        bool HasBone(Manager* manager, EBoneID boneId) const;
        void DePackState(ZHumanState& state, float frame, Manager* manager, int bailAt);
        void DePackQuats(int16_t*, ZBoneQuat* quat, float frame, Manager* manager);
        void DePackPose(int16_t* pPoseToPosLookup, float* poseBuf, float frame, Manager* manager);
        void DePackUncompressed(uint8_t* pBoneIdToIndexLookup, ZBoneQuat* pBones, float frame, Manager* manager);
        void GetGround(Manager* manager, float frame, float* q, float* v, bool bMirror);

        // members
        int16_t m_States;
        int16_t m_OrgStartFrame;
        int16_t m_Frames;
        int16_t m_RealFPS;
        int32_t m_Mask;
        int32_t m_Size;
        int32_t m_StateOffset;
        int32_t m_QuatOffset;
        int32_t m_GroundOffset;
        int32_t m_MetaDataOffset;
        int32_t m_PoseDataOffset;
        float m_BlendFrames;
        float m_CycleDist[3];
        int32_t m_OldControl;
        int32_t m_SoundIndex;
        char* m_Name;
    };
    // Need verify later because on PS2 it's 0x40, in Mini Ninjas it's 0x50, but looks like it's 0x40
    RE_VERIFY_SIZE(Header, 0x40);
}
