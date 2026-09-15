#pragma once

#include <Quazal/RootObject.h>
#include <cstdint>


namespace Quazal
{
    class RefCountedObject : public RootObject
    {
    public:
        // vtbl
        virtual ~RefCountedObject();
        virtual RefCountedObject* AcquireRef();
        virtual void ReleaseRef();

        // methods
        RefCountedObject();
        uint16_t GetRefCount() const;

        // members
        uint16_t m_ui16RefCount;
    };
}
