#pragma once

#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/ZSTL/ZSimpleArray.h>
#include <cstdint>


namespace Glacier
{
    struct ZSerializable;

    struct IInputSerializerStream : public ISerializerStream
    {
        ZSimpleArray<ZSerializable*> m_ProcessTable;
        uint32_t PostProcessWrite;
    };
}