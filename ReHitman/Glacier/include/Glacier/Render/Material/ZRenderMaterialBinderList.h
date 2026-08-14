#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/ZRenderBinderList.h>
#include <cstdint>


namespace Glacier
{
    class ZRenderMaterialBinderList
    {
    public:
        // methods
        ZRenderMaterialBinderList() = default;
        
        // members
        ZRenderBinderList m_BindersMaterial{};
        ZRenderBinderList m_BindersObject{};
    };
    RE_VERIFY_SIZE(ZRenderMaterialBinderList, 0x18);
}