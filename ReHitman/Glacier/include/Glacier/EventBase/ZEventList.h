#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZList.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    class ZEventBase;

    struct ZEventList
    {
        // constants
        // static
        // methods
        ZEventList();
        ~ZEventList();
        void DoInit();
        void Clear();
        void FrameUpdate();
        void AddEvent(ZEventBase* pEvent);
        void RemoveEvent(ZEventBase* pEvent);
        void ActivateFrameUpdate(ZEventBase* pEvent);
        void DeactivateFrameUpdate(ZEventBase* pEvent);

        // members
        uint32_t m_NumberOfEvents;
        ZList<ZEventBase, false, 0> m_EventLists[10];
        ZListIterator<ZEventBase, 0> m_Iterator;
        ZEventBase* m_pCurrentEvent;
    };
    RE_VERIFY_SIZE(ZEventList, 0x5C);
}
