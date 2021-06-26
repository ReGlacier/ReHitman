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
        ZEntityLocator* m_pool[128];
        int field_204;
        int field_208; // enable custom light sources? (for weapon in inventory preview)
    };
}