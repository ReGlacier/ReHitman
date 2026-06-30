#pragma once

#include <Glacier/Geom/ZEntityLocator.h>

namespace Glacier
{
    class ZEngineGeomControl
    {
    public:
        // vftable
        virtual bool GeomMoved(ZEntityLocator* pEntity);
        virtual void UpdateMovedGeoms();
        virtual void Clear();
        virtual void JonsLights(); // crash?

        /**
         * @brief Update lights for entities (must be derived from ZLIGHT)
         * @param ppEntities pointer to array of pointers to entities
         * @param iCount count of pointers in array
         */
        virtual void UpdateChangedLights(ZEntityLocator** ppEntities, uint32_t iCount);

        // public api
        static ZEngineGeomControl* GetInstance();

        // data
        ZBaseGeom* m_MovedGeoms[128];
        int m_lNrMovedGeoms;
        bool m_bChangeDetection;
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(ZEngineGeomControl, 0x20C);
}