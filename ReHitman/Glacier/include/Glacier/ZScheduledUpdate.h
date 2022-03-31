#pragma once

namespace Glacier {
    class ZScheduledEvent;
    class ZEventBase;

    class ZScheduledUpdate {
    public:
        // vftable (no vftable)
        // data (size is 0x6C)
        int m_field0;
        int m_field4;
        int m_field8;
        int m_fieldC;
        int m_field10;
        int m_field14;
        int m_field18;
        int m_field1C;
        int m_field20;
        int m_field24;
        int m_field28;
        int m_field2C;
        int m_field30;
        int m_field34;
        int m_field38;
        int m_field3C;
        int m_field40;
        int m_array44[8];
        int m_field64;
        int m_field68;

        // api
        ZScheduledEvent* AddEvent(ZEventBase* pHolder);
        void RemoveEvent(ZEventBase* pHolder);
        // void SetEvent(ZEventBase*, ZScheduledEvent*) was optimized on PC platform and no need to reconstruct it

    };
}