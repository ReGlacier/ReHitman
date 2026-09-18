#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/Render/Fwd.h>


namespace Glacier
{
    struct ZRenderEntryGeomCreateInfo
    {
        ZRenderEntry* m_pObserverEntry;
        ZBaseGeom* m_pBaseGeom;
        uint32_t m_lPrim;
    };
    RE_VERIFY_SIZE(ZRenderEntryGeomCreateInfo, 0xC);
}