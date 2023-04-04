#pragma once

#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Geom/ZEntityLocator.h>

namespace Glacier {
    class ZRenderEntryGeomD3D {
    public:
        // vftable
        virtual void Release(bool);

        // data (size 0x9C)
        int m_field4; //0x0004
        int m_field8; //0x0008
        int m_fieldC; //0x000C
        int m_field10; //0x0010
        int m_field14; //0x0014
        int m_field18; //0x0018
        int m_field1C; //0x001C
        int m_field20; //0x0020
        int m_field24; //0x0024
        int m_field28; //0x0028 (possible flag)
        unsigned int m_field2C; //0x002C (possible mask)
        int m_unkFlag30; //0x0030 (possible mask)
        int m_field34; //0x0034
        int m_field38; //0x0038
        int m_field3C; //0x003C
        int m_field40; //0x0040
        int m_field44; //0x0044
        Glacier::ZMat3x3 m_transform; //0x0048
        Glacier::ZVector3 m_position; //0x006C
        Glacier::ZEntityLocator* m_pEntityLocator0; //0x0078
        int m_field7C; //0x007C
        int m_field80; //0x0080
        int m_field84; //0x0084
        int m_field88; //0x0088
        Glacier::ZEntityLocator* m_pEntityLocator; //0x008C
        int m_ref; //0x0090
        int m_field94; //0x0094
        Glacier::ZEntityLocator* m_pPlayerEntity; //0x0098 (I'm not sure that this is player entity because sometimes this field is null)
    };
}