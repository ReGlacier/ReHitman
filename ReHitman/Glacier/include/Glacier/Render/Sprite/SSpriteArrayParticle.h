#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Sprite/SSpriteArray.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    /**
     * @brief Particle sprite-array parameters copied from a ZParticleTemplate
     *        into a ZParticleController::SParticleBlock (PC size 0x4C).
     */
    struct SSpriteArrayParticle : public SSpriteArray
    {
        float fMaxAge;                // 0x0C
        float fScale;                 // 0x10
        float fScaleVel;              // 0x14
        float fScaleAcc;              // 0x18
        float fAngleSpeed;            // 0x1C
        float fAngleSpeedVel;         // 0x20
        float fAngleSpeedAcc;         // 0x24
        float fFriction;              // 0x28
        float fMotionStretch;         // 0x2C
        bool bAlignWithDir;           // 0x30
        RE_ADD_PADDING(3);
        int32_t lColorRepeat;         // 0x34
        ZVector3 vScaledGravity;      // 0x38
        const uint32_t* piColorTable; // 0x44
        const float* pfColorTable;    // 0x48
    };
    RE_VERIFY_SIZE(SSpriteArrayParticle, 0x4C);
}
