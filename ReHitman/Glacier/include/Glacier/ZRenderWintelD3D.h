#pragma once

#include <Glacier/Glacier.h>
#include <Windows.h>

namespace Glacier
{
    struct SLODDef
    {
        int field0;
        int field4;
        int field8;
        int fieldC;
        int field10;
        int field14;
        int field18;
        int field1C;
    }; // size 0x20

    class ZRenderWintelD3D
    {
    public:
        char pad_0004[796]; //0x0004
        HWND m_HWND; //0x0320

        virtual void Function0(); //+0x0
        virtual void Function1(); //+0x4
        virtual void Function2(); //+0x8
        virtual void Function3(); //+0xc
        virtual void Function4(); //+0x10
        virtual void Function5(); //+0x14
        virtual void Function6(); //+0x18
        virtual void Function7(); //+0x1c
        virtual void Function8(); //+0x20
        virtual void Function9(); //+0x24
        virtual void Function10(); //+0x28
        virtual void Function11(); //+0x2c
        virtual void Function12(); //+0x30
        virtual void Function13(); //+0x34
        virtual void Function14(); //+0x38
        virtual void Function15(); //+0x3c
        virtual void Function16(); //+0x40
        virtual void Function17(); //+0x44
        virtual void Function18(); //+0x48
        virtual void Function19(); //+0x4c
        virtual void Function20(); //+0x50
        virtual void Function21(); //+0x54
        virtual void Function22(); //+0x58
        virtual void Function23(); //+0x5c
        virtual void Function24(); //+0x60
        virtual void Function25(); //+0x64
        virtual void Function26(); //+0x68
        virtual void Function27(); //+0x6c
        virtual void Function28(); //+0x70
        virtual void Function29(); //+0x74
        virtual void Function30(); //+0x78
        virtual void Function31(); //+0x7c
        virtual void Function32(); //+0x80
        virtual void Function33(); //+0x84
        virtual void Function34(); //+0x88
        virtual void Function35(); //+0x8c
        virtual void Function36(); //+0x90
        virtual void Function37(); //+0x94
        virtual std::uintptr_t getCameraIdByIndex(int); //+0x98
        virtual void Function39(); //+0x9c
        virtual void Function40(); //+0xa0
        virtual void Function41(); //+0xa4
        virtual void Function42(); //+0xa8
        virtual void Function43(); //+0xac
        virtual void Function44(); //+0xb0
        virtual void Function45(); //+0xb4
        virtual void Function46(); //+0xb8
        virtual void Function47(); //+0xbc
        virtual void Function48(); //+0xc0
        virtual void Function49(); //+0xc4
        virtual void Function50(); //+0xc8
        virtual void Function51(); //+0xcc
        virtual void Function52(); //+0xd0
        virtual void Function53(); //+0xd4
        virtual void Function54(); //+0xd8
        virtual void Function55(); //+0xdc
        virtual void Function56(); //+0xe0
        virtual void Function57(); //+0xe4
        virtual void Function58(); //+0xe8
        virtual void Function59(); //+0xec
        virtual void Function60(); //+0xf0
        virtual void Function61(); //+0xf4
        virtual void Function62(); //+0xf8
        virtual void Function63(); //+0xfc
        virtual void Function64(); //+0x100
        virtual void Function65(); //+0x104
        virtual void Function66(); //+0x108
        virtual void Function67(); //+0x10c
        virtual void Function68(); //+0x110
        virtual void Function69(); //+0x114
        virtual void Function70(); //+0x118
        virtual void Function71(); //+0x11c
        virtual void Function72(); //+0x120
        virtual void Function73(); //+0x124
        virtual void Function74(); //+0x128
        virtual void Function75(); //+0x12c
        virtual void Function76(); //+0x130
        virtual void Function77(); //+0x134
        virtual void Function78(); //+0x138
        virtual void Function79(); //+0x13c
        virtual void Function80(); //+0x140
        virtual void Function81(); //+0x144
        virtual void Function82(); //+0x148
        virtual void Function83(); //+0x14c
        virtual void Function84(); //+0x150
        virtual void Function85(); //+0x154
        virtual void Function86(); //+0x158
        virtual void Function87(); //+0x15c
        virtual void Function88(); //+0x160
        virtual void Function89(); //+0x164
        virtual void Function90(); //+0x168
        virtual void Function91(); //+0x16c
        virtual void Function92(); //+0x170
        virtual void Function93(); //+0x174
        virtual void Function94(); //+0x178
        virtual void Function95(); //+0x17c
        virtual void Function96(); //+0x180
        virtual void Function97(); //+0x184
        virtual void Function98(); //+0x188
        virtual void Function99(); //+0x18c
        virtual void Function100(); //+0x190
        virtual void Function101(); //+0x194
        virtual void Function102(); //+0x198
        virtual void Function103(); //+0x19c
        virtual void Function104(); //+0x1a0
        virtual void Function105(); //+0x1a4
        virtual void Function106(); //+0x1a8
        virtual void Function107(); //+0x1ac
        virtual void Function108(); //+0x1b0
        virtual void Function109(); //+0x1b4
        virtual void Function110(); //+0x1b8
        virtual void Function111(); //+0x1bc
        virtual void Function112(); //+0x1c0
        virtual void Function113(); //+0x1c4
        virtual void Function114(); //+0x1c8
        virtual void Function115(); //+0x1cc
        virtual void Function116(); //+0x1d0
        virtual void Function117(); //+0x1d4
        virtual void Function118(); //+0x1d8
        virtual void Function119(); //+0x1dc
        virtual void Function120(); //+0x1e0
        virtual void Function121(); //+0x1e4
        virtual void Function122(); //+0x1e8
        virtual void Function123(); //+0x1ec
        virtual void Function124(); //+0x1f0
        virtual void Function125(); //+0x1f4
        virtual void Function126(); //+0x1f8
        virtual void Function127(); //+0x1fc
        virtual void Function128(); //+0x200
        virtual void Function129(); //+0x204
        virtual void Function130(); //+0x208
        virtual void Function131(); //+0x20c
        virtual void Function132(); //+0x210
        virtual void Function133(); //+0x214
        virtual void Function134(); //+0x218
        virtual void Function135(); //+0x21c
        virtual void Function136(); //+0x220
        virtual void Function137(); //+0x224
        virtual void Function138(); //+0x228
        virtual void Function139(); //+0x22c
        virtual void Function140(); //+0x230
        virtual void Function141(); //+0x234
        virtual void Function142(); //+0x238
        virtual void initializeDevice(); //+0x23c
        virtual void Function144(); //+0x240
        virtual void Function145(); //+0x244
        virtual void Function146(); //+0x248
        virtual void Function147(); //+0x24c
        virtual void Function148(); //+0x250
        virtual void Function149(); //+0x254
        virtual void Function150(); //+0x258
        virtual void Function151(); //+0x25c
        virtual void Function152(); //+0x260
        virtual void Function153(); //+0x264
        virtual void Function154(); //+0x268
        virtual void Function155(); //+0x26c
        virtual void Function156(); //+0x270
        virtual void Function157(); //+0x274
        virtual void Function158(); //+0x278
    }; //Size: 0x0324
}