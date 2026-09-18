#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Prim/ZPrimAccessMesh.h>


namespace Glacier
{
    class ZPrimAccessMeshWeighted : public ZPrimAccessMesh
    {
    public:
        // vtbl
        virtual void CloneVertex(uint32_t lVertex, const ZPrimAccessMeshWeighted* pSource, uint32_t lSourceVertex) = 0; // PC 0x004A2FC0 (StandardWeightedMeshD3D)
    };
}
