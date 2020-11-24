#pragma once

namespace Glacier
{
    class ZRenderMaterialClassD3D;
    class ZRenderMaterialLayerD3DFX;

    class ZRenderMaterialSubClassD3D
    {
    public:
        // vftable
        virtual void Release(bool);

        // data
        char* name;
        ZRenderMaterialClassD3D* m_materialClass;


    };
}