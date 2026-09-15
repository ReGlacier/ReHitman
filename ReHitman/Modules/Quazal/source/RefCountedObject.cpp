#include <Quazal/RefCountedObject.h>


namespace Quazal
{
    RefCountedObject::RefCountedObject()
    {
        m_ui16RefCount = 1;
    }

    RefCountedObject::~RefCountedObject()
    {
        m_ui16RefCount = 0;
    }

    RefCountedObject* RefCountedObject::AcquireRef()
    {
        // NOTE: In KL2 here we have sync primitive Quazal::MutexPrimitive::s_bNoOp
        ++m_ui16RefCount;
        return this;
    }

    void RefCountedObject::ReleaseRef()
    {
        // NOTE: In KL2 here we have sync primitive Quazal::MutexPrimitive::s_bNoOp
        bool bNeedDestroy = false;

        if (m_ui16RefCount == 1)
        {
            bNeedDestroy = true;
        }
        else
        {
            --m_ui16RefCount;
        }

        if (bNeedDestroy)
        {
            delete this;
        }
    }

    uint16_t RefCountedObject::GetRefCount() const
    {
        return m_ui16RefCount;
    }
}
