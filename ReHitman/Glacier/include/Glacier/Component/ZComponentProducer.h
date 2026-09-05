#pragma once

#include <Glacier/Component/ZComponentProducerData.h>

namespace Glacier
{
    template <typename Component>
    class ZComponentProducer : public ZComponentProducerData
    {
    public:
        // types
        using Description_t = typename Component::Description_t;

        // constants
        static constexpr ZComponentGroupId GROUP_ID = Description_t::COMPONENT_GROUP;

        // methods
        ZComponentProducer(ZComponentId id, char* creationParameters)
            : ZComponentProducerData(
                id,
                GROUP_ID,
                &Description_t::template MakeFunctions<Component>::Create,
                &Description_t::template MakeFunctions<Component>::Destroy,
                &Description_t::template MakeFunctions<Component>::SetInstance,
                creationParameters
            )
        {
            if (GetType() == ZComponentProducerData::Types::DEPENDENT)
            {
                AddDependency(Component::ComponentId());
            }
        }

        ~ZComponentProducer() = default;
    };
}
