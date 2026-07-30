#pragma once

#include <Glacier/Component/ZGlobalComponentBase.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    template <typename TComponent, typename TBase>
    struct ZComponentSingleton : public TBase
    {
        // types
        using Base_t = TBase;
        using Singleton_t = ZComponentSingleton<TComponent, TBase>;

        // methods
        ZComponentSingleton()
        {
            ZASSERT(m_pInstance == nullptr);
            m_pInstance = static_cast<TComponent*>(this);
        }

        ~ZComponentSingleton() override
        {
            m_pInstance = nullptr;
        }

        static TComponent& Instance()
        {
            ZASSERT(m_pInstance);

            return *m_pInstance;
        }

        static bool Exists()
        {
            return m_pInstance != nullptr;
        }

        // members
        static TComponent* m_pInstance;
    };
}
