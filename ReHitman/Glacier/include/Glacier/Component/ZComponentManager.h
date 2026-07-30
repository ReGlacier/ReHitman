#pragma once

#include <Glacier/Component/ZComponentManagerBase.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    template <typename T>
    class ZComponentManager : public ZComponentManagerBase
    {
    public:
        // static
        inline static ZComponentManager<T>* m_pInstance { nullptr };

        // vtbl
        ~ZComponentManager() override
        {
            m_pInstance = nullptr;
        }

        void InitializeComponent() override
        {
            // Do nothing
        }

        // methods
        ZComponentManager() : ZComponentManagerBase()
        {
            ZASSERT(m_pInstance == nullptr);
            m_pInstance = this;
        }

        static ZComponentManager<T>& Instance()
        {
            ZASSERT(m_pInstance != nullptr);
            return *m_pInstance;
        }

        // members
    };
}
