#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/ZPrimHandle.h>
#include <cstdint>


namespace Glacier
{
    class ZPrimAccess
    {
    public:
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