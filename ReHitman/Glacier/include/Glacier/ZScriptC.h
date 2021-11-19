#pragma once

#include <Glacier/EventBase/ZEventBase.h>

namespace Glacier
{
    class ZScriptC : public ZEventBase
    {
    public:
        // vftable

        // api
        int FindScript(const char* scriptName);

        // data (0x38)
        int m_pScriptsTable; //+0x30
        int m_field34;
    };
}