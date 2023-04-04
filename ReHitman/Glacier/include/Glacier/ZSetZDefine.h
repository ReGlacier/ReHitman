#pragma once

#include <Glacier/EventBase/ZEventBase.h>

namespace Glacier {
    class ZSetZDefine : public ZEventBase {
    public:
        //vftable (no changes)
        //data (total size is 0x38, base size is 0x30)
        int m_field30;
        int m_field34;
    };
}