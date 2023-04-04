#pragma once

#include <Glacier/Geom/ZLIGHT.h>

namespace Glacier {
    class ZSPOTLIGHT : public ZLIGHT {
    public:
        //vftable
        virtual float GetHotSpotAngle();
        virtual float GetFallOffAngle();
        virtual float GetNearRange();
        virtual float GetFarRange();

        //data (total size is 0x20, base size is 0x20)
    };
}