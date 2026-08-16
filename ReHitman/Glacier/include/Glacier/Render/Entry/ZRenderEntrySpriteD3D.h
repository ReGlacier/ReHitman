#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/Render/Entry/ZRenderEntryGeomCreateInfo.h>
#include <Glacier/Render/Entry/ZRenderEntrySprite.h>


namespace Glacier
{
    class ZRenderEntrySpriteD3D : public ZRenderEntrySprite
    {
    public:
        // vtbl
        ~ZRenderEntrySpriteD3D() override;

        // methods
        ZRenderEntrySpriteD3D(ZRenderEntryGeomCreateInfo* pInfo);
        
        static ZRenderEntrySpriteD3D* Create(ZRenderEntryGeomCreateInfo* pInfo);
    };
    RE_VERIFY_SIZE(ZRenderEntrySpriteD3D, 0x8C); // PC ALLOC
}