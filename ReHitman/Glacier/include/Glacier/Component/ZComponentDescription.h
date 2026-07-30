#pragma once

#include <Glacier/Component/Component.h>
#include <Glacier/Component/ZComponent.h>
#include <cstdint>
#include <limits>
#include <new>


namespace Glacier
{
    template <ZComponentGroupId ComponentGroup, typename Allocator>
    struct ZComponentDescription : public ZComponentBase
    {
        // types
        using Allocator_t = Allocator;

        // constants
        static constexpr ZComponentGroupId COMPONENT_GROUP = ComponentGroup;
        static_assert(ComponentGroup < ZCOMPONENT_GROUP_COUNT, "Component group is outside the runtime group table");

        // methods
        template <typename T> static void SetInstance(T* pInstance)
        {
            if constexpr (requires { typename T::Singleton_t; })
            {
                using Singleton_t = typename T::Singleton_t;
                Singleton_t::m_pInstance = pInstance;
            }
        }

        template <typename T> static void Destroy(T* pInstance)
        {
            pInstance->~T();
            Allocator::Free(static_cast<void*>(pInstance));
        }

        template <typename T> static T* Create()
        {
            void* pMemory = Allocator::Alloc(sizeof(T));
            if (!pMemory)
                return nullptr;

            return new (pMemory) T();
        }

        template <typename T>
        struct MakeFunctions
        {
            static ZComponentBase* Create()
            {
                return ZComponentDescription::template Create<T>();
            }

            static void SetInstance(ZComponentBase* pInstance)
            {
                ZComponentDescription::template SetInstance<T>(static_cast<T*>(pInstance));
            }

            static void Destroy(ZComponentBase* pInstance)
            {
                ZComponentDescription::template Destroy<T>(static_cast<T*>(pInstance));
            }
        };
    };
}
