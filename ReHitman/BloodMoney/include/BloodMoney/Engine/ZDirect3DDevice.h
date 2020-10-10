#pragma once

#include <d3d9.h>

namespace Hitman::BloodMoney::Engine
{
    // Created with ReClass.NET 1.2 by KN4CK3R
    class ZDirect3DDevice
    {
    public:
        IDirect3DDevice9* m_d3dDevice; //0x0004
        int32_t m_unknown; //0x0008

        virtual void Function_0000(); //#0 +0 .rdata:0076419c
        virtual void Function_0001(); //#1 +4 .rdata:007641a0
        virtual void Function_0002(); //#2 +8 .rdata:007641a4
        virtual void Function_0003(); //#3 +c .rdata:007641a8
        virtual void Function_0004(); //#4 +10 .rdata:007641ac
        virtual void Function_0005(); //#5 +14 .rdata:007641b0
        virtual void Function_0006(); //#6 +18 .rdata:007641b4
        virtual void Function_0007(); //#7 +1c .rdata:007641b8
        virtual void setTexture(int a0, int a1); //#8 +20 .rdata:007641bc
        virtual int getTextureStageState(); //#9 +24 .rdata:007641c0
        virtual void Function_0010(); //#10 +28 .rdata:007641c4
        virtual void Function_0011(); //#11 +2c .rdata:007641c8
        virtual void Function_0012(); //#12 +30 .rdata:007641cc
        virtual void Function_0013(); //#13 +34 .rdata:007641d0
        virtual void Function_0014(); //#14 +38 .rdata:007641d4
        virtual void Function_0015(); //#15 +3c .rdata:007641d8
        virtual void Function_0016(); //#16 +40 .rdata:007641dc
        virtual void Function_0017(); //#17 +44 .rdata:007641e0
        virtual void Function_0018(); //#18 +48 .rdata:007641e4
        virtual void Function_0019(); //#19 +4c .rdata:007641e8
        virtual void Function_0020(); //#20 +50 .rdata:007641ec
    }; //Size: 0x000C
}