#pragma once

#include <Glacier/Glacier.h>

namespace Glacier
{
    struct ZRTTI
    {
        char* ComplexTypeName;		//0x0000
        char pad_0x0004[0x14];		//0x0004
        int32_t TypeID;				//0x0018
        char pad_0x001C[0x4];		//0x001C
        char* SelfType;				//0x0020
        char* Parent;				//0x0024
        char pad_0x0028[0x4];		//0x0028
        int32_t Unk0;				//0x002C
        int32_t Unk1;				//0x0030
        char* SelfName2;			//0x0034
        char pad_0x0038[0x4];		//0x0038
        ZRTTI* NextEntity;	        //0x003C
    };
}