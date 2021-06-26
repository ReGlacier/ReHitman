#pragma once

namespace Glacier
{
    class IPackedInputVisitor
    {
    public:
        virtual void Visit_String(unsigned int token, const char* str);
        virtual void Visit_Float64(unsigned int token, double);
        virtual void Visit_Float32(unsigned int token, float); //+0x8
        virtual void Visit_I32(unsigned int token, void* pData); //+0xC
        virtual void Visit_Unknown1(); //+0x10
        virtual void Visit_U16(unsigned int token, unsigned short value); //+0x14
        virtual void Visit_Unknown2(); //+0x18
        virtual void Visit_U8(unsigned int token, char** buffer); //+0x1C
        virtual void Visit_Bool(unsigned int token, bool value); //+0x20
        virtual void Visit_RawData(unsigned int token, void* buffer, unsigned int size); //+0x24
        virtual void Visit_Unknown3(unsigned int token, void* pData); //+0x28
        virtual void Visit_Reference(unsigned int token, unsigned int reference); //+0x2C
        virtual void Visit_Container(unsigned int token, unsigned int a2); //+0x30
        virtual void Visit_BeginObject(unsigned int token); //+0x34
        virtual void Visit_EndObject(); //+0x38
        virtual void Visit_Array(unsigned int token, void* data); //+0x3C
        virtual void Visit_EndArray(); //+0x40
        virtual void Visit_SkipMark(); //+0x44
        virtual void Visit_EndOfStream(); //+0x48
    };
}