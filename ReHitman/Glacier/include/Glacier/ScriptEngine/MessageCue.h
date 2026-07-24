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
    };
    RE_VERIFY_SIZE(_MessageCue, 0xC);

    using MessageCue = _MessageCue;

    void PrepareMessageCue(_MessageCue* pQue);
}