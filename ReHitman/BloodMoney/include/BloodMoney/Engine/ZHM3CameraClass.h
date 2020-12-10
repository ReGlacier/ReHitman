#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZMath.h>

namespace Hitman::BloodMoney
{
    class ZHM3CameraClass
    {
    public:
        int m_matrix4x3; //0x0004
        int m_field8; //0x0008
        int m_memcheck; //0x000C
        int m_baseGeom; //0x0010
        int m_field14; //0x0014
        int m_field18; //0x0018
        int m_field1C; //0x001C
        float m_field20; //0x0020
        float m_field24; //0x0024
        float m_field28; //0x0028
        int m_field2C; //0x002C
        int m_field30; //0x0030
        Glacier::Vector2 m_viewport; //0x0034
        float m_field3C; //0x003C
        float m_field40; //0x0040
        float m_field44; //0x0044
        float m_field48; //0x0048
        float m_field4C; //0x004C
        float m_renderDistance; //0x0050
        int m_field54; //0x0054
        int m_field58; //0x0058
        float m_field5C; //0x005C
        float m_field60; //0x0060
        int field_0064;// +0x64
        int field_0068;// +0x68
        int field_006C;// +0x6C
        int field_0070;// +0x70
        int field_0074;// +0x74
        int field_0078;// +0x78
        int field_007C;// +0x7C
        int field_0080;// +0x80
        int field_0084;// +0x84
        int field_0088;// +0x88
        int field_008C;// +0x8C
        int field_0090;// +0x90
        int field_0094;// +0x94
        int field_0098;// +0x98
        int field_009C;// +0x9C
        int field_00A0;// +0xA0
        int field_00A4;// +0xA4
        int field_00A8;// +0xA8
        int field_00AC;// +0xAC
        int field_00B0;// +0xB0
        int field_00B4;// +0xB4
        int field_00B8;// +0xB8
        int field_00BC;// +0xBC
        int field_00C0;// +0xC0
        int field_00C4;// +0xC4
        int field_00C8;// +0xC8
        int field_00CC;// +0xCC
        int field_00D0;// +0xD0
        int field_00D4;// +0xD4
        int field_00D8;// +0xD8
        int field_00DC;// +0xDC
        int field_00E0;// +0xE0
        int field_00E4;// +0xE4
        int field_00E8;// +0xE8
        int field_00EC;// +0xEC
        int field_00F0;// +0xF0
        int field_00F4;// +0xF4
        int field_00F8;// +0xF8
        int field_00FC;// +0xFC
        int field_0100;// +0x100
        int field_0104;// +0x104
        int field_0108;// +0x108
        int field_010C;// +0x10C
        int field_0110;// +0x110
        int field_0114;// +0x114
        int field_0118;// +0x118
        int field_011C;// +0x11C
        int field_0120;// +0x120
        int field_0124;// +0x124
        int field_0128;// +0x128
        int field_012C;// +0x12C
        int field_0130;// +0x130
        int field_0134;// +0x134
        int field_0138;// +0x138
        int field_013C;// +0x13C
        int field_0140;// +0x140
        int field_0144;// +0x144
        int field_0148;// +0x148
        int field_014C;// +0x14C
        int field_0150;// +0x150
        int field_0154;// +0x154
        int field_0158;// +0x158
        int m_room; //0x015C
        int field_0160;// +0x160
        int field_0164;// +0x164
        int field_0168;// +0x168
        int field_016C;// +0x16C
        int field_0170;// +0x170
        int field_0174;// +0x174
        int field_0178;// +0x178
        int field_017C;// +0x17C
        int field_0180;// +0x180
        int field_0184;// +0x184
        int m_renderD3D; //0x0188
        int m_hitman3; //0x018C
        int m_field190; //0x0190
        int m_field194; //0x0194
        Glacier::Vector2 m_viewport2; //0x0198
        int m_field19C; //0x01A0
        int m_field1A0; //0x01A4
        int m_field1A4; //0x01A8

        // vftable
        virtual void Release(bool);
    };
}