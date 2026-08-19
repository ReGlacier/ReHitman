#pragma once

#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ReGlacier.h>

namespace Glacier
{
    struct SRigidBodyVelocity
    {
        ZMat3x3 m_mOldMat;      
        ZMat3x3 m_mMat;
        ZVector3 m_vOldPos;
        ZVector3 m_vPos;
        float m_fTime;
    };
    RE_VERIFY_SIZE(SRigidBodyVelocity, 0x64); // Verified
}