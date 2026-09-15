#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZSTDOBJ.h>


namespace Hitman::BloodMoney
{
    enum ActorCollisionType : uint32_t
    {
        NONE = 0,
        HERO = 1,
        ALL = 2
    };

    enum WindType : uint32_t
    {
        NO = 0,
        STATIC = 1,
        PERLIN = 2,
        MOVEMENTBASED = 3,
    };

    struct ClothConstraint
    {
        uint16_t ix1;
        uint16_t ix2;
        float m_fWeightedInvMassSum;
        float m_fSquare;
        float m_fDoubleSquare;
    };
    RE_VERIFY_SIZE(ClothConstraint, 0x10);

    // UNFINISHED! Hard class, need debug code more carefuly. Will do later
    class ZCloth : public Glacier::ZSTDOBJ
    {
    public:
        // vftable
        virtual void HandleTensions(float *);
        virtual void Initialize(bool);

        // Data (total size is 0xB0, ZSTDOBJ ends at 0xC)
        int field_10;
        int field_14;
        ActorCollisionType m_iActorCollision; // +0x18
        WindType m_iUseWind; // +0x1C
        int m_field20;
        Glacier::ZVector3 m_WindSpeed; // +0x24
        float m_fElasticity; // +0x2C
        float m_fGravity;
        float m_fDamping;
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

        uint32_t m_lnrBoxes;
        ZGEOM** m_pBoxes;

        int field_7C;
        int field_80;
        int field_84;
        int field_88;
        int field_8C;
        int field_90;
        int field_94;
        int field_98;

        Glacier::ZVector3 m_OldPosition;
        float m_fSyncTimeStep;
        void* m_pDataBlockPtr;
    }; // Total size is 0xB0
    RE_VERIFY_SIZE(ZCloth, 0xB0); // Verified
    RE_VERIFY_OFFSET(ZCloth, m_WindSpeed, 0x24);
    RE_VERIFY_OFFSET(ZCloth, m_fElasticity, 0x30);
}