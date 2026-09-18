#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    struct SRagdollCollisionInfo
    {
        // types
        struct SPartColi
        {
            bool bCollision;
            bool bImpact;
            RE_ADD_PADDING(2);
            uint32_t rGeom;
            ZVector3 vel;
        };
        RE_VERIFY_SIZE(SPartColi, 0x14);

        // members
        bool bCollision;
        bool bInWater;
        unsigned int rWaterBox;
        bool bWaterImpact;
        unsigned __int16 nCollides;
        SRagdollCollisionInfo::SPartColi sPartColi[6];
    };
    RE_VERIFY_SIZE(SRagdollCollisionInfo, 0x84);
}