#pragma once

#include <Glacier/Component/ZComponentProducerData.h>
#include <Glacier/Component/ZComponentDescription.h>


namespace Glacier
{
    template <typename T>
    class ZComponentManagerProducer : public ZComponentProducerData
    {
    public:
        // constants
        static constexpr ZComponentGroupId GROUP_ID = T::Description_t::COMPONENT_GROUP;

        // types
        using Description_t = typename T::Description_t;
        
        // methods

        ZComponentManagerProducer(const char* pszCreationParameters)
            : ZComponentProducerData(
                T::ComponentId(),
                GROUP_ID,
                &Description_t::template MakeFunctions<ZComponentManager<T>>::Create,
                &Description_t::template MakeFunctions<ZComponentManager<T>>::Destroy,
                &Description_t::template MakeFunctions<ZComponentManager<T>>::SetInstance,
                pszCreationParameters
            )
        {
        }

        ~ZComponentManagerProducer() = default;
    };
}
