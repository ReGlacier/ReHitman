#pragma once

#include <Glacier/Geom/ZSTDOBJ.h>

namespace Hitman::BloodMoney
{
    class ZCloth : public Glacier::ZSTDOBJ
    {
    public:
        // vftable
        virtual void HandleTensions(float *);
        virtual void ClothInitialize(bool);

        // Data (total size is 0xB0, ZSTDOBJ ends at 0xC)
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
        int field_40;
        int field_44;
        int field_48;
        int field_4C;
        int field_50;
        int field_54;
        int field_58;
        int field_5C;
        int field_60;
        int field_64;
        int field_68;
        int field_6C;
        int field_70;
        int field_74;
        int field_78;
        int field_7C;
        int field_80;
        int field_84;
        int field_88;
        int field_8C;
        int field_90;
        int field_94;
        int field_98;
        int field_9C;
        int field_A0;
        int field_A4;
        int field_A8;
        int field_AC;
    };
}