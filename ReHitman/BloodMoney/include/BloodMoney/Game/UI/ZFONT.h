#pragma once

#include <BloodMoney/Game/UI/ZWINOBJ.h>

namespace Hitman::BloodMoney {
    struct SCharInfo {
        uint32_t charCode;
        uint32_t unk0;
        uint32_t unk1;
        uint32_t unk2;
        uint32_t unk3;
    };

    enum EFontFilterMode {
        FFM_Normal,
        FFM_Add,
        FFM_Sub,
        FFM_Invert
    };

    class ZFONT : public ZWINOBJ {
    public:
        // vftable
        // at least 5 virtual methods
        virtual int GetNumTextures();               // It's pure virtual for base class ZFONT
        virtual SCharInfo* GetCharInfo(unsigned int);    // It's pure virtual for base class ZFONT
        virtual int GetMaxHeight();                 // It's pure virtual for base class ZFONT
        virtual bool ContainsChar(unsigned int);    // It's pure virtual for base class ZFONT
        virtual void* GetKerning(unsigned int, unsigned int); // It's pure virtual for base class ZFONT
        // data (size is 0x88, base size is 0x88)
    };
}