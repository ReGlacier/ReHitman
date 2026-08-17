#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Prim/ZPrimHandle.h>
#include <cstdint>


namespace Glacier
{
    class ZPrimAccess
    {
    public:
        // types
        enum STATUS_FLAGS
        {
            SF_LOCKED = 1,
            SF_INPLACE = 2,
            SF_DELAYDELETION = 4
        };

        enum LOCK_FLAGS
        {
            LF_READONLY = 1,
            LF_WRITEONLY = 2
        };
        
        // vtbl
        virtual void Destroy();
        virtual void Lock(uint32_t lLockFlags);
        virtual void Unlock();
        virtual void ReleasePrim();
        virtual ~ZPrimAccess();

        // methods
        static ZPrimAccess* Create(const ZPrimHandle& hPrim);

        // members
        ZPrimHandle m_hPrim {};
        uint32_t m_lStatusFlags { 0u };
    };
}