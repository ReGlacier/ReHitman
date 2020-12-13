#pragma once

namespace Glacier
{
    class ZActionMapTree;

    struct ZHandle
    {
        const char* hotkey;
        ZActionMapTree* m_field4;
        int m_pAction;
    };
}