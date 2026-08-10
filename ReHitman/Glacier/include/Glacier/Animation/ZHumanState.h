#pragma once

#include <cstdint>

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>

namespace Glacier
{
    namespace Animation
    {
        struct ZState {};
        struct ZStateBlending;
    }

    /** Serialized channel kind used by Glacier animation stream packing. */
    enum DataType
    {
        eAngle = 0,
        eFloat = 1,
        eGenericFloat = 2,
        eQuat = 3,
        eBone = 4,
    };
    
    struct DataInfo
    {
        /** Packing type used to encode this channel. */
        DataType m_Type {};

        /** Quantization precision used by the stream packer for this channel. */
        float    m_Prec { 0.f };
    };

    /** Runtime pose state for the original human animation rig. */
    struct ZHumanState : Animation::ZState
    {
        /** Initializes the shared human-state channel names, counts, offsets, and compression metadata. */
        static void InitStaticData();

        /** Updates the static channel packing metadata for the requested animation compression ratio. */
        static void SetCompressionRatio(float fRatio);

        /** Writes bone IDs used by a logical human-state group and returns the number of IDs written. */
        static int BoneIds(int state, uint16_t* ids);

        /** Marks static metadata as uninitialized so the next use rebuilds it lazily. */
        static void NukeStaticData();

        /** Mirror bit for torso/root channels. */
        static constexpr int kMirrorTorso = 0x001;

        /** Mirror bit for spine channels. */
        static constexpr int kMirrorSpine = 0x002;

        /** Mirror bit for head channels. */
        static constexpr int kMirrorHead = 0x004;

        /** Mirror bit for left leg channels. */
        static constexpr int kMirrorLeftLeg = 0x008;

        /** Mirror bit for right leg channels. */
        static constexpr int kMirrorRightLeg = 0x010;

        /** Mirror bit for left arm channels. */
        static constexpr int kMirrorLeftArm = 0x020;

        /** Mirror bit for right arm channels. */
        static constexpr int kMirrorRightArm = 0x040;

        /** Mirror bit for left hand/finger channels. */
        static constexpr int kMirrorLeftHand = 0x080;

        /** Mirror bit for right hand/finger channels. */
        static constexpr int kMirrorRightHand = 0x100;

        /** Convenience mask for both leg channel groups. */
        static constexpr int kMirrorLegs = kMirrorLeftLeg | kMirrorRightLeg;

        /** Convenience mask for both arm channel groups. */
        static constexpr int kMirrorArms = kMirrorLeftArm | kMirrorRightArm;

        /** Convenience mask for both hand/finger channel groups. */
        static constexpr int kMirrorHands = kMirrorLeftHand | kMirrorRightHand;

        /** Convenience mask for every known mirrorable human-state group. */
        static constexpr int kMirrorAll = kMirrorTorso | kMirrorSpine | kMirrorHead | kMirrorLegs | kMirrorArms | kMirrorHands;
        
        // methods
        /** Initializes the instance to the original default human pose. */
        ZHumanState();

        /** Trivial destructor; ZHumanState owns no dynamic resources. */
        ~ZHumanState();

        /** Resets quaternion and float channels to the default human pose. */
        void Reset();

        /** Blends selected channel groups from another human state into this state. */
        void Blend(ZHumanState* state, float blend, int mask);

        /** Blends selected channel groups using per-group blend timing state. */
        void Blend(int mask, ZHumanState* state, Animation::ZStateBlending* current, Animation::ZStateBlending* next, float seconds);

        /** Mirrors selected channel groups in-place and returns the mask with left/right groups swapped. */
        int Mirror(int mask);

        // members
        /** Quaternion channels stored before float channels in serialized state order. */
        ZQuat m_Quats[10];

        /** Scalar animation channels stored after quaternion channels in serialized state order. */
        float m_Floats[64];

        /** Display names for the nine logical human-state groups. */
        static const char* m_StateName[9];

        /** Display names for individual quaternion and float channels used by tools and debug output. */
        static const char* m_Names[74];

        /** Number of float channels stored by each logical human-state group. */
        static int m_FloatCount[9];

        /** First float-channel index for each logical human-state group. */
        static int m_FloatStart[9];

        /** Number of quaternion channels stored by each logical human-state group. */
        static int m_QuatCount[9];

        /** First quaternion-channel index for each logical human-state group. */
        static int m_QuatStart[9];

        /** Stream-packer type and precision information for every serialized human-state channel. */
        static DataInfo m_DataInfo[74];

        /** Maps serialized state channel IDs to byte positions inside ZHumanState. */
        static int16_t m_StateId2Pos[74];

        /** One-shot guard that lazily initializes the static channel tables before compression is used. */
        static int firstRun;
    };
    RE_VERIFY_SIZE(ZHumanState, 0x1A0);
}
