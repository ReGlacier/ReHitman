#include <Glacier/Data/SCompiledGeom.h>
#include <Glacier/Geom/ZSTDOBJ.h>


namespace Glacier
{
    SCompiledGeom::SCompiledGeom() = default;

    bool SCompiledGeom::CanExtraGeomBeZero() const
    {
        return (lGeomAndGroupCon & 0x200000u) == 0 && lGeomType == ZSTDOBJ::m_TypeId && !this->lExData && !lEvents && !lScrits;
    }
}
