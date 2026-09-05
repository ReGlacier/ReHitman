#include <Glacier/EventBase/ZEventList.h>
#include <Glacier/EventBase/ZEventBase.h>
#include <Glacier/EventBase/ZEventBuffer.h>
#include <Tests/EngineFixture.h>
#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstring>
#include <memory>
#include <new>
#include <vector>

using namespace Glacier;

namespace
{
    struct PriorityEvent : ZEventBase
    {
        explicit PriorityEvent(EEventPriority priority)
            : Priority(priority)
        {
        }

        EEventPriority GetEventPriority() override
        {
            return Priority;
        }

        EEventPriority Priority;
    };

    struct ZEventListTest : Tests::EngineFixture
    {
        ZEventBuffer EventBuffer{ 512 };
        std::vector<PriorityEvent*> Events;
        std::vector<std::byte*> Storage;

        ~ZEventListTest() override
        {
            for (auto it = Events.rbegin(); it != Events.rend(); ++it)
                (*it)->~PriorityEvent();

            for (auto* storage : Storage)
                ::operator delete(storage, std::align_val_t{ alignof(PriorityEvent) });
        }

        PriorityEvent* CreateEvent(ZEventBase::EEventPriority priority)
        {
            auto* storage = static_cast<std::byte*>(::operator new(sizeof(PriorityEvent), std::align_val_t{ alignof(PriorityEvent) }));
            std::memset(storage, 0, sizeof(PriorityEvent));
            Storage.push_back(storage);

            auto* event = std::construct_at(reinterpret_cast<PriorityEvent*>(storage), priority);
            EngineData().m_EventList.RemoveEvent(event);
            Events.push_back(event);
            return event;
        }
    };
}

TEST_F(ZEventListTest, ActivateFrameUpdateMovesEventToPriorityListFront)
{
    ZEventList list;
    auto* first = CreateEvent(ZEventBase::PRIORITY_Normal);
    auto* second = CreateEvent(ZEventBase::PRIORITY_Normal);

    list.AddEvent(first);
    list.AddEvent(second);

    list.ActivateFrameUpdate(first);
    list.ActivateFrameUpdate(second);

    EXPECT_EQ(list.m_NumberOfEvents, 2u);
    EXPECT_EQ(list.m_EventLists[ZEventBase::INACTIVE_LIST].Count(), 0);
    EXPECT_EQ(list.m_EventLists[ZEventBase::PRIORITY_Normal].Count(), 2);
    EXPECT_EQ(static_cast<ZEventBase*>(list.m_EventLists[ZEventBase::PRIORITY_Normal].Begin()), second);
}

TEST_F(ZEventListTest, DeactivateFrameUpdateMovesEventToInactiveListBack)
{
    ZEventList list;
    auto* first = CreateEvent(ZEventBase::PRIORITY_Normal);
    auto* second = CreateEvent(ZEventBase::PRIORITY_Normal);

    list.AddEvent(first);
    list.AddEvent(second);
    list.ActivateFrameUpdate(first);
    list.ActivateFrameUpdate(second);

    list.DeactivateFrameUpdate(second);
    list.DeactivateFrameUpdate(first);

    EXPECT_EQ(list.m_NumberOfEvents, 2u);
    EXPECT_EQ(list.m_EventLists[ZEventBase::PRIORITY_Normal].Count(), 0);
    EXPECT_EQ(list.m_EventLists[ZEventBase::INACTIVE_LIST].Count(), 2);
    EXPECT_EQ(static_cast<ZEventBase*>(list.m_EventLists[ZEventBase::INACTIVE_LIST].Begin()), second);
}

TEST_F(ZEventListTest, DeactivateFrameUpdateAdvancesIteratorAndClearsCurrentEvent)
{
    ZEventList list;
    auto* first = CreateEvent(ZEventBase::PRIORITY_Normal);
    auto* second = CreateEvent(ZEventBase::PRIORITY_Normal);

    list.AddEvent(first);
    list.AddEvent(second);
    list.ActivateFrameUpdate(first);
    list.ActivateFrameUpdate(second);

    list.m_Iterator = list.m_EventLists[ZEventBase::PRIORITY_Normal].Begin();
    list.m_pCurrentEvent = second;

    list.DeactivateFrameUpdate(second);

    EXPECT_EQ(static_cast<ZEventBase*>(list.m_Iterator), first);
    EXPECT_EQ(list.m_pCurrentEvent, nullptr);
    EXPECT_EQ(list.m_EventLists[ZEventBase::INACTIVE_LIST].Count(), 1);
}
