#pragma once

#include <Glacier/ZListNodeBase.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/EventBase/ZEventBase.h>

namespace Glacier
{
    class ZEventBuffer
    {
    public:
        // members
        char pad_0004[4]; //0x0004
        ZEventBase* m_events; //0x0008
        char pad_000C[4]; //0x000C
        ZOffsetAlloc* m_offsetAlloc; //0x0010
        char pad_0014[44]; //0x0014

        /// === vftable ===
        virtual void Function_0000(); //#0 +0 .rdata:00760fd8
        virtual void Function_0001(); //#1 +4 .rdata:00760fdc
        virtual void Function_0002(); //#2 +8 .rdata:00760fe0
        virtual void Function_0003(); //#3 +c .rdata:00760fe4
        virtual void Function_0004(); //#4 +10 .rdata:00760fe8
        virtual void Function_0005(); //#5 +14 .rdata:00760fec
        virtual void Function_0006(); //#6 +18 .rdata:00760ff0
        virtual void Function_0007(); //#7 +1c .rdata:00760ff4
        virtual void Function_0008(); //#8 +20 .rdata:00760ff8
        virtual void Function_0009(); //#9 +24 .rdata:00760ffc
        virtual void Function_0010(); //#10 +28 .rdata:00761000
        virtual void Function_0011(); //#11 +2c .rdata:00761004
    };

}