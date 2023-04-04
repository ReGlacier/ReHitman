#pragma once

#include <Glacier/Geom/ZLIGHT.h>

namespace Glacier {
    class ZOMNILIGHT : public ZLIGHT {
    public:
        //vftable
        virtual void SetNearRange(float value);
        virtual void SetFarRange(float value);
        virtual float GetNearRange();
        virtual float GetFarRange();

        //data (total size is 0x20, base size is 0x20)
    };
}