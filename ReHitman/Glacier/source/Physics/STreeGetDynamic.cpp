#include <Glacier/Physics/STreeGetDynamic.h>
#include <Glacier/Geom/GeomControlMasks.h>
#include <Glacier/Geom/ZBaseGeom.h>


namespace Glacier
{
    void STreeGetDynamic::ClearMarks()
    {
        const auto lCount = lNrMarkedGeoms;
        auto lGeomIdx = 0;

        if (lCount)
        {
            ZBaseGeom** pCurrentGeom = pMarkedGeoms;
            do
            {
                (*pCurrentGeom)->SetControl(0, ZCTEMPDRAW);
                ++lGeomIdx;
                ++pCurrentGeom;
            }
            while (lGeomIdx != lCount);
        }
        
        lNrMarkedGeoms = 0;
    }
}