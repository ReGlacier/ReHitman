#include <Glacier/Geom/ZGeomListTypeUtils.h>
#include <Glacier/Geom/ZSHAPE.h>
#include <Glacier/Geom/ZSTDOBJ.h>
#include <Glacier/Geom/ZBOUND.h>
#include <Glacier/Geom/ZSNDOBJ.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZLIGHT.h>
#include <Glacier/Geom/ZGEOM.h>


namespace Glacier
{
    eBaseGeomListTypes GetBaseGeomListType(ZBaseGeom* pBaseGeom)
    {
        if (!pBaseGeom)
        {
            return eBaseGeomListTypes::BGLT_Other; 
        }

        if (pBaseGeom->IsDerivedFrom<ZSHAPE>())
        {
            return eBaseGeomListTypes::BGLT_StdObj;
        }

        if (pBaseGeom->IsDerivedFrom<ZSTDOBJ>())
        {
            if (pBaseGeom->IsDerivedFrom<ZBOUND>())
            {
                return eBaseGeomListTypes::BGLT_Other;
            }

            return pBaseGeom->IsDerivedFrom<ZSNDOBJ>() ? eBaseGeomListTypes::BGLT_StdObj : eBaseGeomListTypes::BGLT_Other;
        }

        if (pBaseGeom->IsDerivedFrom<ZGROUP>())
        {
            return eBaseGeomListTypes::BGLT_Group;
        }

        if (pBaseGeom->IsDerivedFrom<ZLIGHT>())
        {
            return eBaseGeomListTypes::BGLT_Light;
        }

        return eBaseGeomListTypes::BGLT_Other;
    }
}
