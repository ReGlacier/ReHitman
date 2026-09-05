#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/EventBase/ZScheduledEvent.h>

namespace Glacier 
{
    class ZScheduledScript : public ZScheduledEvent 
    {
    public:
        // vtbl
        void Run() override;
        ZScheduledEvent* GetNextThread() override;
        ZScheduledEvent* Fork() override;

        // methods
        ZScheduledScript(ZEventBase* pEvent);
        void Sleep(float fTime);
        void Restart();
        void SetUserData(void* pData);
        void* GetUserData() const;

        // members (none)
    };
}