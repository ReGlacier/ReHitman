#pragma once

namespace Glacier {
    class ZScheduledEvent {
    public:
        enum EScheduledEventState {
            STATE_RUNNING = 0x40000000u
        };

        // vftable
        virtual void Run();
        virtual int GetNextThread();
        virtual ZScheduledEvent* Fork();

        // data (no base size, current size is 0x1C)
        int m_rOwnerEntity;
        int m_field8;
        int m_rNextThread;
        int m_field10;
        int m_field14;  // some kind of mask, see EScheduledEventState for details
        int m_field18;
    };
}