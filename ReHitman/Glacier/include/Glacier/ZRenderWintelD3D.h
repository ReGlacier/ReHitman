#pragma once

#include <Glacier/Glacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZSTL/LINKSORTREFTAB.h>
#include <Windows.h>

namespace Glacier
{
    class ZRenderDrawD3D;

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
        // VTBL
        virtual void Release(bool shouldFreeMem); //+0x0
        virtual void Close(); //+0x4
        virtual void CrashClose(); //+0x8
        virtual void SetWideScreen(bool wideScreenMode); //+0xc
        virtual void SetLetterBox(bool letterBox); //+0x10
        virtual void SetSurroundGaming(bool); //+0x14
        virtual int ShadowMode(); //+0x18
        virtual void SetCopyToFrontAlpha(uint); //+0x1c (nullstub)
        virtual ZRenderDrawD3D* GetDrawBase(); //+0x20
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
        virtual Glacier::ZREF GetCameraIdByIndex(int); //+0x98
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

        // Data
        int m_field4; //+0x4
        int m_field8; //+0x8
        int m_fieldC; //+0xC
        int m_field10; //+0x10
        int m_field14; //+0x14
        int m_field18; //+0x18
        int m_field1C; //+0x1C
        int m_field20; //+0x20
        int m_field24; //+0x24
        int m_field28; //+0x28
        int m_field2C; //+0x2C
        int m_field30; //+0x30
        int m_field34; //+0x34
        int m_field38; //+0x38
        int m_field3C; //+0x3C
        int m_field40; //+0x40
        int m_field44; //+0x44
        int m_field48; //+0x48
        int m_field4C; //+0x4C
        int m_field50; //+0x50
        int m_field54; //+0x54
        int m_field58; //+0x58
        int m_field5C; //+0x5C
        int m_field60; //+0x60
        int m_field64; //+0x64
        int m_field68; //+0x68
        int m_field6C; //+0x6C
        int m_field70; //+0x70
        int m_field74; //+0x74
        int m_field78; //+0x78
        int m_field7C; //+0x7C
        int m_field80; //+0x80
        int m_field84; //+0x84
        int m_field88; //+0x88
        int m_field8C; //+0x8C
        int m_field90; //+0x90
        int m_field94; //+0x94
        int m_field98; //+0x98
        int m_field9C; //+0x9C
        int m_fieldA0; //+0xA0
        int m_fieldA4; //+0xA4
        int m_fieldA8; //+0xA8
        int m_fieldAC; //+0xAC
        int m_fieldB0; //+0xB0
        int m_fieldB4; //+0xB4
        int m_fieldB8; //+0xB8
        int m_fieldBC; //+0xBC
        int m_fieldC0; //+0xC0
        int m_fieldC4; //+0xC4
        int m_fieldC8; //+0xC8
        int m_fieldCC; //+0xCC
        int m_fieldD0; //+0xD0
        int m_fieldD4; //+0xD4
        int m_fieldD8; //+0xD8
        int m_fieldDC; //+0xDC
        int m_fieldE0; //+0xE0
        int m_fieldE4; //+0xE4
        int m_fieldE8; //+0xE8
        int m_fieldEC; //+0xEC
        int m_fieldF0; //+0xF0
        int m_fieldF4; //+0xF4
        int m_fieldF8; //+0xF8
        int m_fieldFC; //+0xFC
        int m_field100; //+0x100
        int m_field104; //+0x104
        int m_field108; //+0x108
        int m_field10C; //+0x10C
        int m_field110; //+0x110
        int m_field114; //+0x114
        int m_field118; //+0x118
        int m_field11C; //+0x11C
        int m_field120; //+0x120
        int m_field124; //+0x124
        int m_field128; //+0x128
        int m_field12C; //+0x12C
        int m_field130; //+0x130
        int m_field134; //+0x134
        int m_field138; //+0x138
        int m_field13C; //+0x13C
        int m_field140; //+0x140
        int m_field144; //+0x144
        int m_field148; //+0x148
        int m_field14C; //+0x14C
        int m_field150; //+0x150
        int m_field154; //+0x154
        int m_field158; //+0x158
        int m_field15C; //+0x15C
        int m_field160; //+0x160
        int m_field164; //+0x164
        int m_field168; //+0x168
        int m_field16C; //+0x16C
        int m_field170; //+0x170
        int m_field174; //+0x174
        int m_field178; //+0x178
        int m_field17C; //+0x17C
        int m_field180; //+0x180
        int m_field184; //+0x184
        int m_field188; //+0x188
        int m_field18C; //+0x18C
        int m_field190; //+0x190
        int m_field194; //+0x194
        int m_field198; //+0x198
        int m_field19C; //+0x19C
        int m_field1A0; //+0x1A0
        int m_field1A4; //+0x1A4
        int m_field1A8; //+0x1A8
        int m_field1AC; //+0x1AC
        int m_field1B0; //+0x1B0
        int m_field1B4; //+0x1B4
        int m_field1B8; //+0x1B8
        int m_field1BC; //+0x1BC
        int m_field1C0; //+0x1C0
        int m_field1C4; //+0x1C4
        int m_field1C8; //+0x1C8
        int m_field1CC; //+0x1CC
        int m_field1D0; //+0x1D0
        int m_field1D4; //+0x1D4
        int m_field1D8; //+0x1D8
        int m_field1DC; //+0x1DC
        int m_field1E0; //+0x1E0
        int m_field1E4; //+0x1E4
        int m_field1E8; //+0x1E8
        int m_field1EC; //+0x1EC
        int m_field1F0; //+0x1F0
        int m_field1F4; //+0x1F4
        int m_field1F8; //+0x1F8
        int m_field1FC; //+0x1FC
        int m_field200; //+0x200
        int m_field204; //+0x204
        int m_field208; //+0x208
        int m_field20C; //+0x20C
        int m_field210; //+0x210
        int m_field214; //+0x214
        int m_field218; //+0x218
        int m_shadowMode; //+0x21C
        int m_field220; //+0x220
        int m_field224; //+0x224
        int m_field228; //+0x228
        int m_field22C; //+0x22C
        int m_field230; //+0x230
        int m_field234; //+0x234
        int m_field238; //+0x238
        int m_field23C; //+0x23C
        int m_field240; //+0x240
        int m_field244; //+0x244
        int m_field248; //+0x248
        int m_field24C; //+0x24C
        int m_field250; //+0x250
        int m_field254; //+0x254
        int m_field258; //+0x258
        int m_field25C; //+0x25C
        int m_field260; //+0x260
        int m_field264; //+0x264
        int m_field268; //+0x268
        int m_field26C; //+0x26C
        int m_field270; //+0x270
        int m_field274; //+0x274
        int m_field278; //+0x278
        int m_field27C; //+0x27C
        int m_field280; //+0x280
        int m_field284; //+0x284
        int m_field288; //+0x288
        int m_field28C; //+0x28C
        int m_frameUpdateInterval; //+0x290
        Glacier::ZVector3 m_mousePos; //0x294 (in 3D space) In Range {[-0.5f; 0.5f]; [-0.5f; 0.5f]; [0.f, 0.f]}
        int m_field2A0; //+0x2A0
        int m_field2A4; //+0x2A4
        bool m_mouse0;  //+0x2A8 (DOES NOT WORK)
        bool m_mouse1;  //+0x2A9 (DOES NOT WORK)
        bool m_mouse2;  //+0x2AA (DOES NOT WORK)
        char m_pad2AB;  //+0x2AB
        int m_field2AC; //+0x2AC
        int m_field2B0; //+0x2B0
        int m_field2B4; //+0x2B4
        bool m_field2B8; //0x2B8
        bool m_bWideScreenMode; //0x2B9
        bool m_bLetterBox; //0x2BA
        bool m_field2BB; //0x2BB
        int m_sizeX; //+0x2BC (READ ONLY)
        int m_sizeY; //+0x2C0 (READ ONLY)
        LINKSORTREFTAB m_lnkSortREFTAB_2C4; //0x2C4
        int m_field2F0; //+0x2F0
        Glacier::ZVector2 m_fontSize; //0x2F4;
        int m_pRenderDraw; //+0x2FC
        int m_field300; //+0x300
        int m_field304; //+0x304
        int m_field308; //+0x308
        int m_field30C; //+0x30C
        int m_field310; //+0x310
        int m_field314; //+0x314
        int m_field318; //+0x318
        int m_field31C; //+0x31C
        HWND m_HWND; // 0x320
        int m_unkPoolElementsCount; //0x0324
        int m_unkPool[1024]; //0x0328
        int m_field1328; //+0x1328
        int m_field132C; //+0x132C
        int m_field1330; //+0x1330
        int m_field1334; //+0x1334
        int m_field1338; //+0x1338
        int m_field133C; //+0x133C
        int m_field1340; //+0x1340
        int m_field1344; //+0x1344
        bool m_bDisableRender; //0x1348 NOTE: Be aware, this flag stops all rendering, includes ImGUI overlay
        char m_pad1349;  //+0x1349
        char m_pad134A;  //+0x134A
        char m_pad134B;  //+0x134B
        int m_field134C; //+0x134C
        int m_field1350; //+0x1350
        int m_field1354; //+0x1354
        int m_field1358; //+0x1358
        int m_field135C; //+0x135C
        int m_field1360; //+0x1360
        int m_field1364; //+0x1364
        int m_field1368; //+0x1368
        int m_field136C; //+0x136C
        int m_field1370; //+0x1370
        int m_field1374; //+0x1374
        int m_field1378; //+0x1378
        int m_field137C; //+0x137C
        int m_field1380; //+0x1380
        int m_field1384; //+0x1384
        int m_field1388; //+0x1388
        int m_field138C; //+0x138C
        int m_field1390; //+0x1390
        int m_field1394; //+0x1394
        int m_field1398; //+0x1398
        int m_field139C; //+0x139C
        int m_field13A0; //+0x13A0
        int m_field13A4; //+0x13A4
        int m_field13A8; //+0x13A8
        int m_field13AC; //+0x13AC
        int m_field13B0; //+0x13B0
        int m_field13B4; //+0x13B4
        int m_field13B8; //+0x13B8
        int m_field13BC; //+0x13BC
        int m_field13C0; //+0x13C0
        int m_field13C4; //+0x13C4
        int m_field13C8; //+0x13C8
        int m_field13CC; //+0x13CC
        int m_field13D0; //+0x13D0
        int m_field13D4; //+0x13D4
        int m_field13D8; //+0x13D8
        int m_field13DC; //+0x13DC
        int m_field13E0; //+0x13E0
        int m_field13E4; //+0x13E4
        int m_field13E8; //+0x13E8
        int m_field13EC; //+0x13EC
        int m_field13F0; //+0x13F0
        int m_field13F4; //+0x13F4
        int m_field13F8; //+0x13F8
        int m_field13FC; //+0x13FC
        int m_field1400; //+0x1400
        int m_field1404; //+0x1404
        int m_field1408; //+0x1408
        int m_field140C; //+0x140C
        int m_field1410; //+0x1410
        int m_field1414; //+0x1414
        int m_field1418; //+0x1418
        int m_field141C; //+0x141C
        int m_field1420; //+0x1420
        int m_field1424; //+0x1424
        int m_field1428; //+0x1428
        int m_field142C; //+0x142C
        int m_field1430; //+0x1430
        int m_field1434; //+0x1434
        int m_field1438; //+0x1438
        int m_field143C; //+0x143C
        int m_field1440; //+0x1440
        int m_field1444; //+0x1444
        int m_field1448; //+0x1448
        int m_field144C; //+0x144C
        int m_field1450; //+0x1450
        int m_field1454; //+0x1454
        int m_field1458; //+0x1458
        int m_field145C; //+0x145C
        int m_field1460; //+0x1460
        int m_field1464; //+0x1464
        int m_field1468; //+0x1468
        int m_field146C; //+0x146C
        int m_field1470; //+0x1470
        int m_field1474; //+0x1474
        int m_field1478; //+0x1478
        int m_field147C; //+0x147C
        int m_field1480; //+0x1480
        int m_field1484; //+0x1484
        int m_field1488; //+0x1488
        int m_field148C; //+0x148C
        int m_field1490; //+0x1490
        int m_field1494; //+0x1494
        int m_field1498; //+0x1498
        int m_field149C; //+0x149C
        int m_field14A0; //+0x14A0
        int m_field14A4; //+0x14A4
        int m_field14A8; //+0x14A8
        int m_field14AC; //+0x14AC
        int m_field14B0; //+0x14B0
        int m_field14B4; //+0x14B4
        int m_field14B8; //+0x14B8
        int m_field14BC; //+0x14BC
        int m_field14C0; //+0x14C0
        int m_field14C4; //+0x14C4
        int m_field14C8; //+0x14C8
        int m_field14CC; //+0x14CC
        int m_field14D0; //+0x14D0
        int m_field14D4; //+0x14D4
        int m_field14D8; //+0x14D8
        int m_field14DC; //+0x14DC
        int m_field14E0; //+0x14E0
        int m_field14E4; //+0x14E4
        int m_field14E8; //+0x14E8
        int m_field14EC; //+0x14EC
        int m_field14F0; //+0x14F0
        int m_field14F4; //+0x14F4
        int m_field14F8; //+0x14F8
        int m_field14FC; //+0x14FC
        int m_field1500; //+0x1500
        int m_field1504; //+0x1504
        int m_field1508; //+0x1508
        int m_field150C; //+0x150C
        int m_field1510; //+0x1510
        int m_field1514; //+0x1514
        int m_field1518; //+0x1518
        int m_field151C; //+0x151C
        int m_field1520; //+0x1520
        int m_field1524; //+0x1524
        int m_field1528; //+0x1528
        int m_field152C; //+0x152C
        int m_field1530; //+0x1530
        int m_field1534; //+0x1534
        int m_field1538; //+0x1538
        int m_field153C; //+0x153C
        int m_field1540; //+0x1540
        int m_field1544; //+0x1544
        int m_field1548; //+0x1548
        int m_field154C; //+0x154C
        int m_field1550; //+0x1550
        int m_field1554; //+0x1554
        int m_field1558; //+0x1558
        int m_field155C; //+0x155C
        int m_field1560; //+0x1560
        int m_field1564; //+0x1564
        int m_field1568; //+0x1568
        int m_field156C; //+0x156C
        int m_field1570; //+0x1570
        int m_field1574; //+0x1574
        int m_field1578; //+0x1578
        int m_field157C; //+0x157C
        int m_field1580; //+0x1580
        int m_field1584; //+0x1584
        int m_field1588; //+0x1588
        int m_field158C; //+0x158C
        int m_field1590; //+0x1590
        int m_field1594; //+0x1594
        int m_field1598; //+0x1598
        int m_field159C; //+0x159C
        int m_field15A0; //+0x15A0
        int m_field15A4; //+0x15A4
        int m_field15A8; //+0x15A8
        int m_field15AC; //+0x15AC
        int m_field15B0; //+0x15B0
        int m_field15B4; //+0x15B4
        int m_field15B8; //+0x15B8
        int m_field15BC; //+0x15BC
        int m_field15C0; //+0x15C0
        int m_field15C4; //+0x15C4
        int m_field15C8; //+0x15C8
        int m_field15CC; //+0x15CC
        int m_field15D0; //+0x15D0
        int m_field15D4; //+0x15D4
        int m_field15D8; //+0x15D8
        int m_field15DC; //+0x15DC
        int m_field15E0; //+0x15E0
        int m_field15E4; //+0x15E4
        int m_field15E8; //+0x15E8
        int m_field15EC; //+0x15EC
        int m_field15F0; //+0x15F0
        int m_field15F4; //+0x15F4
        int m_field15F8; //+0x15F8
        int m_field15FC; //+0x15FC
        int m_field1600; //+0x1600
        int m_field1604; //+0x1604
        int m_field1608; //+0x1608
        int m_field160C; //+0x160C
        int m_field1610; //+0x1610
        int m_field1614; //+0x1614
        int m_field1618; //+0x1618
        int m_field161C; //+0x161C
        int m_field1620; //+0x1620
        int m_field1624; //+0x1624
        int m_field1628; //+0x1628
        int m_field162C; //+0x162C
        int m_field1630; //+0x1630
        int m_field1634; //+0x1634
        int m_field1638; //+0x1638
        int m_field163C; //+0x163C
        int m_field1640; //+0x1640
        int m_field1644; //+0x1644
        int m_field1648; //+0x1648
        int m_field164C; //+0x164C
        int m_field1650; //+0x1650
        int m_field1654; //+0x1654
        int m_field1658; //+0x1658
        int m_field165C; //+0x165C
        int m_field1660; //+0x1660
        int m_field1664; //+0x1664
        int m_field1668; //+0x1668
        int m_field166C; //+0x166C
        int m_field1670; //+0x1670
        int m_field1674; //+0x1674
        int m_field1678; //+0x1678
        int m_field167C; //+0x167C
        int m_field1680; //+0x1680
        int m_field1684; //+0x1684
        int m_field1688; //+0x1688
        int m_field168C; //+0x168C
        int m_field1690; //+0x1690
        int m_field1694; //+0x1694
        int m_field1698; //+0x1698
        int m_field169C; //+0x169C
        int m_field16A0; //+0x16A0
        int m_field16A4; //+0x16A4
        int m_field16A8; //+0x16A8
        int m_field16AC; //+0x16AC
        int m_field16B0; //+0x16B0
        int m_field16B4; //+0x16B4
        int m_field16B8; //+0x16B8
        int m_field16BC; //+0x16BC
        int m_field16C0; //+0x16C0
        int m_field16C4; //+0x16C4
        int m_field16C8; //+0x16C8
        int m_field16CC; //+0x16CC
        int m_field16D0; //+0x16D0
        int m_field16D4; //+0x16D4
        int m_field16D8; //+0x16D8
        int m_field16DC; //+0x16DC
        int m_field16E0; //+0x16E0
        int m_field16E4; //+0x16E4
        int m_field16E8; //+0x16E8
        int m_field16EC; //+0x16EC
        int m_field16F0; //+0x16F0
        int m_field16F4; //+0x16F4
        int m_field16F8; //+0x16F8
        int m_field16FC; //+0x16FC
        int m_field1700; //+0x1700
        int m_field1704; //+0x1704
        int m_field1708; //+0x1708
        int m_field170C; //+0x170C
        int m_field1710; //+0x1710
        int m_field1714; //+0x1714
        int m_field1718; //+0x1718
        int m_field171C; //+0x171C
        int m_field1720; //+0x1720
        int m_field1724; //+0x1724
        int m_field1728; //+0x1728
        int m_field172C; //+0x172C
        int m_field1730; //+0x1730
        int m_field1734; //+0x1734
        int m_field1738; //+0x1738
        int m_field173C; //+0x173C
        int m_field1740; //+0x1740
        int m_field1744; //+0x1744
        int m_field1748; //+0x1748
        int m_field174C; //+0x174C
        int m_field1750; //+0x1750
        int m_field1754; //+0x1754
        int m_field1758; //+0x1758
        int m_field175C; //+0x175C
        int m_field1760; //+0x1760
        int m_field1764; //+0x1764
        int m_field1768; //+0x1768
        int m_field176C; //+0x176C
        int m_field1770; //+0x1770
        int m_field1774; //+0x1774
        int m_field1778; //+0x1778
        int m_field177C; //+0x177C
        int m_field1780; //+0x1780
        int m_field1784; //+0x1784
        int m_field1788; //+0x1788
        int m_field178C; //+0x178C
        int m_field1790; //+0x1790
    }; //Size: 0x0324
}