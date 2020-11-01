#pragma once

#include <Glacier/ZTextureType.h>

namespace Glacier
{
    class ZBitmap
    {
    public:
        // vftable
        virtual ~ZBitmap(); //0x0
        virtual const char* GetTextureName() const; //0x4
        virtual ZTextureType GetTextureType(); //0x8, ret 0x4
        virtual void Function_03(int, bool); //0xC
        virtual void Function_04(); //0x10
        virtual void Function_05(); //0x14
        virtual void SetTextureName(const char* textureName); //0x18
        virtual int GetWidth() const; //0x1C, ret 0x8
        virtual int GetHeight() const; //0x20, ret 0xC
        virtual int Function_09(int value); //0x24, set 0x24 and return new value
        virtual int Function_10() const; //0x28, ret 0x24
        virtual float Function_11(float value); //0x2C, set 0x2C
        virtual float Function_12() const; //0x30, ret 0x2C
        virtual int Function_13() const; //0x34, ret 0x10
        virtual int Function_14(int arg); //0x38, ret *0x14 + (20 * arg)
        virtual unsigned int Function_15(); //0x3C
        virtual char* Function_16(int); //0x40
        virtual int* Function_17(int); //0x44
        virtual int Function_18() const; //0x48, ret 0x20
        virtual int Function_19(int value); //0x4C, set 0x20 and return it
        virtual int Function_20() const; //0x50, ret 0x1C
        virtual int Function_21(int val); //0x54, set 0x1C and ret it
        virtual int UpdateBuffer(void* buffer, int size, int width, int height, int a6, bool shouldUpdateBuffer); //0x58, a6 is always 0
        virtual int Function_23(int); //0x5C
        virtual int Function_24(unsigned int, unsigned int, bool); //0x60
        virtual int Function_25(); //0x64
        virtual void SetWidth(int value); //0x68, set 0x8 value
        virtual void SetHeight(int value); //0x6C, set 0xC value
        virtual void Function_28(int value); //0x70, set 0x10 value
        virtual void Function_29(int, int, int, int); //0x74
        virtual int  Function_30(int, int, int, int); //0x78

        // data
        ZTextureType m_type;
        int m_width;
        int m_height;
        int m_field10;
        int m_field14;
        const char* m_textureName;
        int m_field1C;
        int m_field20;
        int m_field24;
        bool m_isCustomNameUsed;
        bool m_field29;
        bool m_field2A;
        bool m_field2B;
        float m_field2C;
    }; //size 0x30

    class ZBitmap32   : public ZBitmap {};
    class ZBitmapDXT1 : public ZBitmap {};
    class ZBitmapDXT3 : public ZBitmap {};
    class ZBitmapI8   : public ZBitmap {};
    class ZBitmapU8V8 : public ZBitmap {};

    class ZBitmapPal : public ZBitmap
    {
    public:
        //additional data
        int m_field30;
        int m_field34;
        int m_field38;
    }; //size 0x3C

    class ZBitmapPalOpac : public ZBitmapPal
    {
    public:
        // additional data
        int m_field3C;
        int m_field40;
    }; //size 0x44
}