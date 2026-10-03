#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZUniMemory.h>
#include <cstdint>

namespace Glacier
{
    /**
     * Owner for the packed static game-level data buffer loaded from an SGD
     * cache file. The original PC object is a four-byte holder.
     */
    struct ZStaticGameLevelData
    {
        static void Create();
        static ZStaticGameLevelData* Instance();

        void Load(const void* pData);
        void Destroy();

        STATIC_CLASS_VAR(ZStaticGameLevelData, ZStaticGameLevelData*, m_Instance);

        const void* m_pFirstDataBlock;
    };
    RE_VERIFY_SIZE(ZStaticGameLevelData, 0x4);
}
