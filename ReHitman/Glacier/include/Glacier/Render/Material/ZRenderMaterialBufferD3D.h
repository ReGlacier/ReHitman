#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Material/ZRenderMaterialBuffer.h>
#include <cstdint>


namespace Glacier
{
    class ZRenderMaterialBufferD3D : public ZRenderMaterialBuffer
    {
    public:
        // constants
        static constexpr int MAX_MATERIAL_INSTANCES_NR = 2048;

        // vtbl
        ~ZRenderMaterialBufferD3D() override;
        virtual uint32_t NumMaterialInstances() const;

        // methods

        // members
        uint8_t m_D3DSpecificShit[8192]; // TODO: Finish me
        uint32_t m_lNumMaterialClasses { 0u };
        int m_pMaterialClasses[32];
        int m_lNumMaterialInstances{ 0 };
        ZRenderMaterialInstance* m_pMaterialInstances[MAX_MATERIAL_INSTANCES_NR];
        bool m_bResourcesAllocated { false };
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(ZRenderMaterialBufferD3D, 0x40B0);
}