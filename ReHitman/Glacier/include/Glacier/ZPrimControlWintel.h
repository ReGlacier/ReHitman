#pragma once

namespace Glacier
{
    class SPrimInfo
    {
    public:
        int8_t m_byte0; //0x0000
        int8_t m_byte1; //0x0001
        int16_t m_kind; //0x0002
        int32_t m_field4; //0x0004
        int32_t m_field8; //0x0008
        int8_t m_byteC; //0x000C
        int8_t m_byteD; //0x000D
        int8_t m_byteE; //0x000E
        int8_t m_byteF; //0x000F
        int m_field10; //0x0010
    }; //Size: 0x0014 (struct is not done!!!)

    class ZPrimControlWintel
    {
    public:
        // vftable
        virtual void Release(bool); //0x0
        virtual void Function_4(); //+4
        virtual void Function_8(); //+8
        virtual void Function_C(); //+C
        virtual void Function_10(); //+10
        virtual SPrimInfo* GetPrimData(int primitiveId);
        virtual void Function_18(); //+18
        virtual void Function_1C(); //+1C
        virtual void Function_20(); //+20
        virtual void Function_24(); //+24
        virtual void Function_28(); //+28
        virtual void Function_2C(); //+2C
        virtual void Function_30(); //+30
        virtual void Function_34(); //+34
        virtual void Function_38(); //+38
        virtual void Function_3C(); //+3C
        virtual void Function_40(); //+40
        virtual void Function_44(); //+44
        virtual void Function_48(); //+48
        virtual void Function_4C(); //+4C
        virtual void Function_50(); //+50
        virtual void Function_54(); //+54
        virtual void Function_58(); //+58
        virtual void Function_5C(); //+5C
        virtual void Function_60(); //+60
        virtual void Function_64(); //+64
        virtual void Function_68(); //+68
        virtual void Function_6C(); //+6C
        virtual void Function_70(); //+70
        virtual void Function_74(); //+74
        virtual void Function_78(); //+78
        virtual void Function_7C(); //+7C
        virtual void Function_80(); //+80
        virtual void Function_84(); //+84
        virtual void Function_88(); //+88
        virtual void Function_8C(); //+8C
        virtual void Function_90(); //+90
        virtual void Function_94(); //+94
        virtual void Function_98(); //+98
        virtual void Function_9C(); //+9C
        virtual void Function_A0(); //+A0
        virtual void Function_A4(int primitiveId); //+A4
        virtual void Function_A8(int primitiveId); //+A8
        virtual void Function_AC(); //+AC
        virtual void Function_B0(); //+B0
        virtual void Function_B4(); //+B4
        virtual void Function_B8(); //+B8
        virtual void Function_BC(); //+BC
        virtual void Function_C0(); //+C0
        virtual void Function_C4(int primitiveId); //+C4
        virtual void Function_C8(); //+C8
        virtual void Function_CC(); //+CC
        virtual void Function_D0(); //+D0
        virtual void Function_D4(); //+D4
        virtual void Function_D8(); //+D8
        virtual void Function_DC(); //+DC
        virtual void Function_E0(); //+E0
        virtual void Function_E4(); //+E4
        virtual void Function_E8(); //+E8
        virtual void Function_EC(); //+EC
        virtual void Function_F0(); //+F0
        virtual void Function_F4(); //+F4
        virtual void Function_F8(); //+F8
        virtual void Function_FC(); //+FC
        // data
    };
}