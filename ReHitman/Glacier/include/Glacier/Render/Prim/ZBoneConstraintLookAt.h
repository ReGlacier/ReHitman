#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Prim/ZBoneConstraint.h>


namespace Glacier
{
    class ZBoneConstraintLookAt : public ZBoneConstraint
    {
    public:
        // constants
        static constexpr int32_t Type = 0;

        // methods
        ZBoneConstraintLookAt() = default;

        // members
        uint8_t m_lNrTargets;
        uint8_t m_lLookAtAxis;
        uint8_t m_lUpBoneAlignmentAxis;
        uint8_t m_lLookAtFlip;
        uint8_t m_lUpFlip;
        uint8_t m_UpnodeControl;
        uint8_t m_AlignedToUpnodeAxis;
        uint8_t m_UpNodeParentIdx;
        uint8_t m_TargetParentIdx[2];
        uint8_t _align[2];
        float m_lBoneTargetsWeights[2];
        float m_TargetPos[2][3];
        float m_UpPos[3];
    };
    RE_VERIFY_SIZE(ZBoneConstraintLookAt, 0x3C);
}
