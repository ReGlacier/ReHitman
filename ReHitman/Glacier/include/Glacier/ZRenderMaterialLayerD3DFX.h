#pragma once

namespace Glacier
{
    class ZRenderMaterialLayerD3DFX
    {
    public:
        // vftable
        virtual void Release(bool);
        virtual void Function_1();
        virtual void Function_2();
        virtual void Function_3();
        virtual void Function_4();
        virtual void Function_5();
        virtual void Function_6();
        virtual void Function_7(); //NULLSTUB
        virtual void Function_8();
        virtual void Function_9();  //NULLSTUB
        virtual void Function_10(); //Return m_field14 if m_fieldC inited
        virtual void Function_11();
        virtual void Function_12(); //NULLSTUB

        // data
        char* m_layerName; //0x0004
        char** m_pShaderName; //0x0008
        int m_fieldC; //0x000C
        char* m_techniqueName; //0x0010
        int m_field14; //0x0014
        int m_field18; //0x0018
        int m_field1C; //0x001C
    };

}