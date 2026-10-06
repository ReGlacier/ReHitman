#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Component/ZComponentSingleton.h>
#include <Glacier/Component/ZRuntimeComponentBase.h>


namespace Glacier
{
    template <typename T>
    class ZDrawDebugObjectComponent : public ZComponentSingleton<ZDrawDebugObjectComponent<T>, ZRuntimeComponentBase>
    {
    public:
        // methods
        ZDrawDebugObjectComponent() = default;

        // members
        T* m_Component;
    };
}