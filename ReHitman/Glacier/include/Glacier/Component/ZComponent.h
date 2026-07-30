#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Component/ZComponentBase.h>
#include <Glacier/Component/ZComponentManager.h>


namespace Glacier
{
    template <typename T, typename TDescription>
    struct ZComponent : public ZComponentBase
    {
        // types
        using Description_t = TDescription;
        
        // vtbl
        void InitializeComponent() override
        {
            ZComponentManager<T>::Instance().Add(this);
        }

        // methods
        ZComponent() : ZComponentBase()
        {
        }

        ~ZComponent() override = default;
    };
}
