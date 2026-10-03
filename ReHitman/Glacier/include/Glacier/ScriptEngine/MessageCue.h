#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>


namespace Glacier
{
    struct _MessageCue
    {
        struct _MessageCue* m_pLast;
        struct _MessageCue* m_pNext;
        ZMSGID msg;

        void* GetData()
        {
            return this + 1;
        }

        const void* GetData() const
        {
            return this + 1;
        }
    };
    RE_VERIFY_SIZE(_MessageCue, 0xC);

    using MessageCue = _MessageCue;
}
