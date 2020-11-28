#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZLinkedListNodeBase.h>

namespace Glacier
{
    class ZEventBase : public ZLinkedListNodeBase
    {
    public:
        /// === members ===
        int32_t m_field14; //0x0014
        int32_t m_field18; //0x0018
        int32_t m_field1C; //0x001C
        int32_t m_field20; //0x0020
        ZSTDOBJ* m_STDOBJ; //0x0024
        int32_t m_field28; //0x0028

        /// === vftable ===
        virtual void Remove(); //#0 +0 .rdata:00768ce4
        virtual void Function_0001(); //#1 +4 .rdata:00768ce8
        virtual void Function_0002(); //#2 +8 .rdata:00768cec
        virtual void Function_0003(); //#3 +c .rdata:00768cf0
        virtual void Function_0004(); //#4 +10 .rdata:00768cf4
        virtual void Function_0005(); //#5 +14 .rdata:00768cf8
        virtual void Function_0006(); //#6 +18 .rdata:00768cfc
        virtual void Function_0007(); //#7 +1c .rdata:00768d00
        virtual void Function_0008(); //#8 +20 .rdata:00768d04
        virtual void Function_0009(); //#9 +24 .rdata:00768d08
        virtual void Function_0010(); //#10 +28 .rdata:00768d0c
        virtual void Function_0011(); //#11 +2c .rdata:00768d10
        virtual void Function_0012(); //#12 +30 .rdata:00768d14
        virtual void Function_0013(); //#13 +34 .rdata:00768d18
        virtual void Function_0014(); //#14 +38 .rdata:00768d1c
        virtual void Function_0015(); //#15 +3c .rdata:00768d20
        virtual void Function_0016(); //#16 +40 .rdata:00768d24
        virtual void Function_0017(); //#17 +44 .rdata:00768d28
        virtual void Function_0018(); //#18 +48 .rdata:00768d2c
        virtual void Function_0019(); //#19 +4c .rdata:00768d30
        virtual void Function_0020(); //#20 +50 .rdata:00768d34
        virtual void Function_0021(); //#21 +54 .rdata:00768d38
        virtual void Function_0022(); //#22 +58 .rdata:00768d3c
        virtual void Function_0023(); //#23 +5c .rdata:00768d40
        virtual void Function_0024(); //#24 +60 .rdata:00768d44
        virtual void Function_0025(); //#25 +64 .rdata:00768d48
        virtual void Function_0026(); //#26 +68 .rdata:00768d4c
        virtual void Function_0027(); //#27 +6c .rdata:00768d50
        virtual void Function_0028(); //#28 +70 .rdata:00768d54
        virtual void Function_0029(); //#29 +74 .rdata:00768d58
        virtual void Function_0030(); //#30 +78 .rdata:00768d5c
        virtual void Function_0031(); //#31 +7c .rdata:00768d60
        virtual void Function_0032(); //#32 +80 .rdata:00768d64
        virtual void Function_0033(); //#33 +84 .rdata:00768d68
    }; //Size: 0x0030
}