#pragma once

#include <Glacier/Geom/ZSTDOBJ.h>

namespace Hitman::BloodMoney
{
    class ZHM3ClothBundle : public Glacier::ZSTDOBJ
    {
    public:
        //vftable
        virtual void CopyDataFromClothBundle(ZHM3ClothBundle* from);

        //data (total size is 0x40, base size is 0x10)
        int field_10;
        int field_14;
        int field_18;
        int field_1C;
        int field_20;
        int field_24;
        int field_28;
        int field_2C;
        int field_30;
        int field_34;
        int field_38;
        int field_3C;
    };
}