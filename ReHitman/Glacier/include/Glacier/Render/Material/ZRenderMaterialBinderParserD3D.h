#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Material/ZRenderMaterialBinderParser.h>


namespace Glacier
{
    class ZRenderMaterialBinderParserD3D : public ZRenderMaterialBinderParser
    {
    public:
        // vtbl
        ~ZRenderMaterialBinderParserD3D() override;

        // methods
        ZRenderMaterialBinderParserD3D();
        
        // members
    };
}