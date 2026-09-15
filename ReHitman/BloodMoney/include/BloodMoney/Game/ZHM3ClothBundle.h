#pragma once

#include <Glacier/Geom/ZSTDOBJ.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/Glacier.h>

namespace Hitman::BloodMoney
{
    struct SAccessoryGeom
    {
        Glacier::ZREF rGeom;
        uint32_t iBoneId;
    };
    RE_VERIFY_SIZE(SAccessoryGeom, 0x8);

    class ZHM3ClothBundle : public Glacier::ZSTDOBJ
    {
    public:
        //vftable
        void CopyData(ZGEOM const*) override;

        //data (total size is 0x40, base size is 0x10)
        Glacier::ZREF m_rHitmanAs;
        bool m_bBloody;
        Glacier::ZREF m_rOriginalParent;
        Glacier::ZREF m_rContainingElevator;
        Glacier::ZREF m_rAction;
        SAccessoryGeom m_aAccessoryGeoms[3];
        int8_t m_iNumAccessoryGeoms;

    };
    RE_VERIFY_SIZE(ZHM3ClothBundle, 0x40);
}