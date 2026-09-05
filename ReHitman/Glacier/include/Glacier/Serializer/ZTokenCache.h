#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZList.h>
#include <Glacier/Serializer/ZToken.h>


namespace Glacier
{
    struct ISerializerStream;

    struct ZTokenCache : public ZListNode<ZTokenCache,0>, public ZToken
    {
        // methods
        ZTokenCache(const char* psName)
        {
            m_Name = psName;
            m_Stream = nullptr;
        }

        void Flush()
        {
            Unlink();
            m_Stream = nullptr;
        }

        // members
        const char* m_Name;
        ISerializerStream *m_Stream;
    };
    RE_VERIFY_SIZE(ZTokenCache, 0x14);
}
