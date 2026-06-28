#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZOctree.h>

namespace Glacier
{
    struct ZOctreeCompiled : public ZOctree
    {
        // data
        void* m_pxTree;
    };
    RE_VERIFY_SIZE(ZOctreeCompiled, 0x18);
}