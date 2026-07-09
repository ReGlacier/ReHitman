#pragma once

#include <Glacier/Component/ZGlobalComponentBase.h>


namespace Glacier
{
    template <typename TComponent, typename TBase>
    struct ZComponentSingleton : public ZGlobalComponentBase
    {
        static TComponent* m_pInstance;
    };
}
