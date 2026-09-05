#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Component/ZComponentBase.h>
#include <cstdint>


namespace Glacier
{
    enum CallDirection
    {
        FRONT = 0,
        BACK = 1,
    };


    class ZComponentManagerBase : public ZComponentBase
    {
    public:
        // vtbl
        ~ZComponentManagerBase() override;
        
        // methods
        ZComponentManagerBase();

        void Add(ZComponentBase* pComponent);
        ZComponentBase* Begin(CallDirection eDirection);
        ZComponentBase* Next(ZComponentBase* pComponent, CallDirection eDirection);

        // members
        ZComponentBase* m_pFirst{nullptr};
        ZComponentBase* m_pLast{nullptr};
    };
}
