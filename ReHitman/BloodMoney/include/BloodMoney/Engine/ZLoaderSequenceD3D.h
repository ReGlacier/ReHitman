#pragma once

#include <d3d9.h>

namespace Hitman::BloodMoney
{
    // Created with ReClass.NET 1.2 by KN4CK3R
    class ZEntryGeom
    {
    public:
        int m_field0;
        int m_field4;
        int m_field8;
        int m_fieldC;
        int m_field10;
        int m_field14;
        int m_field18;
        int m_field1C;
        int m_field20;
        int m_field24;
        int m_field28;
        int m_field2C;
        int m_field30;
        int m_field34;
        int m_field38;
        int m_field3C;
        int m_field40;
        int m_field44;
        int m_field48;
        IDirect3DVertexBuffer9* m_pGeomVertexBuffer; //0x004C

        void foo() {
            m_pGeomVertexBuffer->Release();
        }
    }; //Size: 0x0050

    class ZEntryGeomInfo
    {
    public:
        D3DFVF_DIFFUSE
        D3DUSAGE_AUTOGENMIPMAP
        ZEntryGeom* m_pGeom; //0x0000
        int32_t m_iVariants; //0x0004
    }; //Size: 0x0008

    class ZLoader_Sequence_Wintel_D3D
    {
    public:
        int m_field4;
        int m_field8;
        int m_fieldC;
        int m_field10;
        int m_field14;
        int m_field18;
        int m_field1C;
        ZEntryGeomInfo** m_ppEntries; //0x0020
        int32_t m_iEntriesCount; //0x0024
        char pad_0028[280]; //0x0028 (Holder)

        virtual void Function0(); // Just a holder
    }; //Size: 0x0140




}