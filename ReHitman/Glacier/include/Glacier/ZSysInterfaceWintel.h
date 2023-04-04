#pragma once

#include <Glacier/Glacier.h>

namespace Glacier
{
    class ZEngineDataBase;
    class ZRenderWintelD3D;
    class CConfiguration;

    class ZSysInterfaceWintel
    {
    public:
        int32_t m_field4; //0x0004
        int32_t m_field8; //0x0008
        int32_t m_fieldC; //0x000C
        int32_t m_field10; //0x0010
        float m_field14; //0x0014
        int32_t m_field18; //0x0018
        float m_field1C; //0x001C
        int32_t m_field20; //0x0020
        float m_field24; //0x0024
        int32_t m_lastTickAt; //0x0028
        char pad_002C[75]; //0x002C
        bool m_noBlood; //0x0077
        char pad_0078[64]; //0x0078
        ZEngineDataBase* m_engineDataBase; //0x00B8
        char* m_basePath; //0x00BC
        char pad_00C0[252]; //0x00C0
        char* m_scenesFolderName; //0x01BC
        char pad_01C0[252]; //0x01C0
        char* m_currentScene; //0x02BC
        char pad_02C0[508]; //0x02C0
        char* m_currentConfig; //0x04BC
        char pad_04C0[892]; //0x04C0
        char* m_gameName; //0x083C
        char pad_0840[252]; //0x0840
        int32_t m_heroControlMode; //0x093C
        char pad_0940[20]; //0x0940
        bool m_disableLight; //0x0954
        bool m_field0955;
        bool m_field0956;
        bool m_field0957;
        int m_field958;
        int m_field95C;
        int m_field960;
        int m_field964;
        int m_field968;
        int m_field96C;
        int m_appWindowHWND;
        int m_field974;
        int m_field978;
        ZRenderWintelD3D* m_renderer; //0x097C
        char pad_0980[132]; //0x0980
        char* m_iniPath0; //0x0A04
        char pad_0A08[124]; //0x0A08
        char* m_iniPath; //0x0A84
        char pad_0A88[128]; //0x0A88
        bool m_disableConfig; //0x0B08
        char pad_0B09[3]; //0x0B09
        uint32_t m_soundWintelDLL; //0x0B0C
        char pad_0B10[10]; //0x0B10
        bool m_scriptDebugPrint; //0x0B1A
        char pad_0B1B[25]; //0x0B1B
        CConfiguration* m_configuration; //0x0B34
        char pad_0B38[72]; //0x0B38

        /// === vftable ===
        virtual void Function_0000(); //#0 +0 .rdata:0075fdac
        virtual void Function_0001(); //#1 +4 .rdata:0075fdb0
        virtual void Function_0002(); //#2 +8 .rdata:0075fdb4
        virtual void Function_0003(); //#3 +c .rdata:0075fdb8
        virtual void Function_0004(); //#4 +10 .rdata:0075fdbc
        virtual void Function_0005(); //#5 +14 .rdata:0075fdc0
        virtual void Function_0006(); //#6 +18 .rdata:0075fdc4
        virtual void Function_0007(); //#7 +1c .rdata:0075fdc8
        virtual void Function_0008(); //#8 +20 .rdata:0075fdcc
        virtual void Function_0009(); //#9 +24 .rdata:0075fdd0
        virtual void Function_0010(); //#10 +28 .rdata:0075fdd4
        virtual void Function_0011(); //#11 +2c .rdata:0075fdd8
        virtual void Function_0012(); //#12 +30 .rdata:0075fddc
        virtual void Function_0013(); //#13 +34 .rdata:0075fde0
        virtual void Function_0014(); //#14 +38 .rdata:0075fde4
        virtual void Function_0015(); //#15 +3c .rdata:0075fde8
        virtual void Function_0016(); //#16 +40 .rdata:0075fdec
        virtual void Function_0017(); //#17 +44 .rdata:0075fdf0
        virtual void Function_0018(); //#18 +48 .rdata:0075fdf4
        virtual void Function_0019(); //#19 +4c .rdata:0075fdf8
        virtual void Function_0020(); //#20 +50 .rdata:0075fdfc
        virtual void Function_0021(); //#21 +54 .rdata:0075fe00
        virtual void Function_0022(); //#22 +58 .rdata:0075fe04
        virtual CConfiguration* LoadConfiguration(); //#23 +5c .rdata:0075fe08
        virtual void Function_0024(); //#24 +60 .rdata:0075fe0c
        virtual void Function_0025(); //#25 +64 .rdata:0075fe10
        virtual void Function_0026(); //#26 +68 .rdata:0075fe14
        virtual void Function_0027(); //#27 +6c .rdata:0075fe18
        virtual void Function_0028(); //#28 +70 .rdata:0075fe1c
        virtual void Function_0029(); //#29 +74 .rdata:0075fe20
        virtual void Function_0030(); //#30 +78 .rdata:0075fe24
        virtual void Function_0031(); //#31 +7c .rdata:0075fe28
        virtual void Function_0032(); //#32 +80 .rdata:0075fe2c
        virtual void Function_0033(); //#33 +84 .rdata:0075fe30
        virtual void Function_0034(); //#34 +88 .rdata:0075fe34
        virtual void Function_0035(); //#35 +8c .rdata:0075fe38
        virtual void Function_0036(); //#36 +90 .rdata:0075fe3c
        virtual void Function_0037(); //#37 +94 .rdata:0075fe40
        virtual void Function_0038(); //#38 +98 .rdata:0075fe44
        virtual void Function_0039(); //#39 +9c .rdata:0075fe48
        virtual void Function_0040(); //#40 +a0 .rdata:0075fe4c
        virtual void Function_0041(); //#41 +a4 .rdata:0075fe50
        virtual void Function_0042(); //#42 +a8 .rdata:0075fe54
        virtual void Function_0043(); //#43 +ac .rdata:0075fe58
        virtual void Function_0044(); //#44 +b0 .rdata:0075fe5c
        virtual void Function_0045(); //#45 +b4 .rdata:0075fe60
        virtual void Function_0046(); //#46 +b8 .rdata:0075fe64
        virtual void Function_0047(); //#47 +bc .rdata:0075fe68
        virtual void Function_0048(); //#48 +c0 .rdata:0075fe6c
        virtual void Function_0049(); //#49 +c4 .rdata:0075fe70
        virtual void Function_0050(); //#50 +c8 .rdata:0075fe74
        virtual void Function_0051(); //#51 +cc .rdata:0075fe78
        virtual void Function_0052(); //#52 +d0 .rdata:0075fe7c
        virtual void Function_0053(); //#53 +d4 .rdata:0075fe80
        virtual void Function_0054(); //#54 +d8 .rdata:0075fe84
        virtual void Function_0055(); //#55 +dc .rdata:0075fe88
        virtual void Function_0056(); //#56 +e0 .rdata:0075fe8c
        virtual void Function_0057(); //#57 +e4 .rdata:0075fe90
        virtual void Function_0058(); //#58 +e8 .rdata:0075fe94
        virtual void Function_0059(); //#59 +ec .rdata:0075fe98
        virtual void Function_0060(); //#60 +f0 .rdata:0075fe9c
        virtual void Function_0061(); //#61 +f4 .rdata:0075fea0
        virtual void Function_0062(); //#62 +f8 .rdata:0075fea4
        virtual void Function_0063(); //#63 +fc .rdata:0075fea8
        virtual void Function_0064(); //#64 +100 .rdata:0075feac
        virtual void Function_0065(); //#65 +104 .rdata:0075feb0
        virtual void Function_0066(); //#66 +108 .rdata:0075feb4
        virtual void Function_0067(); //#67 +10c .rdata:0075feb8
        virtual void Function_0068(); //#68 +110 .rdata:0075febc
        virtual void Function_0069(); //#69 +114 .rdata:0075fec0
        virtual void Function_0070(); //#70 +118 .rdata:0075fec4
        virtual void Function_0071(); //#71 +11c .rdata:0075fec8
        virtual void Function_0072(); //#72 +120 .rdata:0075fecc
        virtual void Function_0073(); //#73 +124 .rdata:0075fed0
        virtual void Function_0074(); //#74 +128 .rdata:0075fed4
        virtual void Function_0075(); //#75 +12c .rdata:0075fed8
        virtual void Function_0076(); //#76 +130 .rdata:0075fedc
    };
}