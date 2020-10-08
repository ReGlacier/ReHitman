#pragma once

#include <Glacier.h>

namespace Glacier
{
    class REFTAB32
    {
    public:
        // === members ===
        uint32_t m_pStart; //0x0004
        char pad_0008[28]; //0x0008
        int32_t m_itemsCount; //0x0024
        char pad_0028[4]; //0x0028
        uint32_t m_firstItem; //0x002C
        char pad_0030[128]; //0x0030

        // === vftable ===
        virtual REFTAB32* Release(bool doFreeMemory); //#0 +0 .rdata:00756adc
        virtual void Function_0001(); //#1 +4 .rdata:00756ae0
        virtual void Function_0002(); //#2 +8 .rdata:00756ae4
        virtual void Function_0003(); //#3 +c .rdata:00756ae8
        virtual void Function_0004(); //#4 +10 .rdata:00756aec
        virtual void Function_0005(); //#5 +14 .rdata:00756af0
        virtual void Function_0006(); //#6 +18 .rdata:00756af4
        virtual void Function_0007(); //#7 +1c .rdata:00756af8
        virtual void Function_0008(); //#8 +20 .rdata:00756afc
        virtual void Function_0009(); //#9 +24 .rdata:00756b00
        virtual void Function_0010(); //#10 +28 .rdata:00756b04
        virtual void Function_0011(); //#11 +2c .rdata:00756b08
        virtual void Function_0012(); //#12 +30 .rdata:00756b0c
        virtual void Function_0013(); //#13 +34 .rdata:00756b10
        virtual void Function_0014(); //#14 +38 .rdata:00756b14
        virtual void Function_0015(); //#15 +3c .rdata:00756b18
        virtual void Function_0016(); //#16 +40 .rdata:00756b1c
        virtual void Function_0017(); //#17 +44 .rdata:00756b20
        virtual void Function_0018(); //#18 +48 .rdata:00756b24
        virtual void Function_0019(); //#19 +4c .rdata:00756b28
        virtual void* getFirstEntity(void* pResult); //#20 +50 .rdata:00756b2c
        virtual void Function_0021(); //#21 +54 .rdata:00756b30
        virtual void Function_0022(); //#22 +58 .rdata:00756b34
        virtual void Function_0023(); //#23 +5c .rdata:00756b38
        virtual int* getNextEntityAfter(int* pEntity); //#24 +60 .rdata:00756b3c
        virtual void Function_0025(); //#25 +64 .rdata:00756b40
        virtual void Function_0026(); //#26 +68 .rdata:00756b44
        virtual void Function_0027(); //#27 +6c .rdata:00756b48
        virtual void Function_0028(); //#28 +70 .rdata:00756b4c
        virtual void Function_0029(); //#29 +74 .rdata:00756b50
        virtual void Function_0030(); //#30 +78 .rdata:00756b54
        virtual void Function_0031(); //#31 +7c .rdata:00756b58
        virtual void Function_0032(); //#32 +80 .rdata:00756b5c
        virtual void Function_0033(void* a0); //#33 +84 .rdata:00756b60
        virtual void* Function_0034(); //#34 +88 .rdata:00756b64
    };

    template <typename T> T* get(REFTAB32* reftab, size_t index)
    {
        if (!reftab || index >= reftab->m_itemsCount) return nullptr;
        return reinterpret_cast<T*>(&reftab->m_firstItem) + index;
    }
}