#pragma once

namespace Glacier
{
    class FsZip_t
    {
    public:
        // == vftable ==
        virtual void Function_0000(); // [0000] +0000 [.rdata at 0x0075692C]
        virtual void Function_0001(); // [0001] +0004 [.rdata at 0x00756930]
        virtual void Function_0002(); // [0002] +0008 [.rdata at 0x00756934]
        virtual void Function_0003(); // [0003] +000C [.rdata at 0x00756938]
        virtual int getFileSize(const char* fileName); // [0004] +0010 [.rdata at 0x0075693C]
        virtual void Function_0005(const char* supFileName); // [0005] +0014 [.rdata at 0x00756940]
        virtual void Function_0006(); // [0006] +0018 [.rdata at 0x00756944]
        virtual void Function_0007(const char* fileName, char* dst, int unk1 = 0, int unk2 = 0); // [0007] +001C [.rdata at 0x00756948]
        virtual void Function_0008(); // [0008] +0020 [.rdata at 0x0075694C]
        virtual void Function_0009(); // [0009] +0024 [.rdata at 0x00756950]
        virtual void Function_0010(); // [0010] +0028 [.rdata at 0x00756954]
        virtual void Function_0011(); // [0011] +002C [.rdata at 0x00756958]
        virtual void Function_0012(); // [0012] +0030 [.rdata at 0x0075695C]
        virtual void Function_0013(); // [0013] +0034 [.rdata at 0x00756960]
        virtual void Function_0014(); // [0014] +0038 [.rdata at 0x00756964]
        virtual void Function_0015(); // [0015] +003C [.rdata at 0x00756968]
        virtual void Function_0016(); // [0016] +0040 [.rdata at 0x0075696C]
        virtual void Function_0017(); // [0017] +0044 [.rdata at 0x00756970]
        virtual void Function_0018(); // [0018] +0048 [.rdata at 0x00756974]
        virtual void Function_0019(); // [0019] +004C [.rdata at 0x00756978]
        virtual void Function_0020(); // [0020] +0050 [.rdata at 0x0075697C]
        virtual void Function_0021(); // [0021] +0054 [.rdata at 0x00756980]
        virtual void Function_0022(); // [0022] +0058 [.rdata at 0x00756984]
        virtual void Function_0023(); // [0023] +005C [.rdata at 0x00756988]
        virtual void Function_0024(); // [0024] +0060 [.rdata at 0x0075698C]
        virtual void Function_0025(); // [0025] +0064 [.rdata at 0x00756990]
        virtual void Function_0026(); // [0026] +0068 [.rdata at 0x00756994]

        // == data ==
        int field_4;
        int field_8;
        int field_c;
        int field_10;
        int field_14;
        int field_18;
        int field_1c;
        int field_20;
        int field_24;
        int field_28;
        int field_2c;
        int field_30;
        int field_34;
        int field_38;
        int field_3c;
        int field_40;
        int field_44;
        int field_48;
        int field_4c;
        int field_50;
        int field_54;
        int field_58;
        int field_5c;
        int field_60;
        int field_64;
        int field_68;
        int field_6c;
        int field_70;
        int field_74;
        int field_78;
        int field_7c;
        int field_80;
        int field_84;
        int field_88;
        int field_8c;
        int field_90;
        int field_94;
    };
}