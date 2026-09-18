#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    template <typename T>
    struct IDynamicSingleton
    {
        // static
        static T* m_Instance;

        // methods
        IDynamicSingleton()
        {
            m_PrevInstance = m_Instance;
            m_Instance = static_cast<T*>(this);
        }

        ~IDynamicSingleton()
        {
            m_Instance = m_PrevInstance;
        }

        static T& Instance()
        {
            ZASSERT(m_Instance);

            return *m_Instance;
        }

        // members
        T* m_PrevInstance;
    };

    template <typename T>
    T* IDynamicSingleton<T>::m_Instance = nullptr;
}