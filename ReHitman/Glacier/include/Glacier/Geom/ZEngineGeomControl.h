#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    class ZEngineGeomControl
    {
    public:
        // constant
        static constexpr uint32_t MAX_MOVED_GEOMS_NR = 128;

        // vtbl
        virtual bool GeomMoved(ZBaseGeom* pGeom);
        virtual void UpdateMovedGeoms();
        virtual void Clear();
        virtual void JonsLights(); // crash?
        virtual void UpdateChangedLights(ZBaseGeom** pBaseGeomList, uint32_t lNrLights);

        // methods
        static ZEngineGeomControl& GetInstance();

        ZEngineGeomControl();
        bool GetChangeDetection() const;

        // data
        ZREF m_MovedGeoms[MAX_MOVED_GEOMS_NR];
        int m_lNrMovedGeoms;
        bool m_bChangeDetection;
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(ZEngineGeomControl, 0x20C);
}