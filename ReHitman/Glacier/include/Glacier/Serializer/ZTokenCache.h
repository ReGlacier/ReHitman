#pragma once

#include <Glacier/ZListNodeBase.h>
#include <Glacier/Serializer/ZToken.h>


namespace Glacier
{
    struct ISerializerStream;

    struct ZTokenCache : public ZListNode<ZTokenCache,0>, public ZToken
    {
        const char* m_Name;
        ISerializerStream *m_Stream;
    };
}