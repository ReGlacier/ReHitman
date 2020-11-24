#pragma once

namespace Glacier
{
    class ZRenderMaterialClassD3D
    {
    public:
        //vftable
        virtual void Release(bool);

        //data
        char* m_name;
        char* m_name2;
        int m_fieldC;
    };
}