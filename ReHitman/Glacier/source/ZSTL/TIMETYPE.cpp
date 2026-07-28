#include <Glacier/ZSTL/TIMETYPE.h>
#include <Glacier/Serializer/ISerializerStream.h>


namespace Glacier
{
    void TIMETYPE::LoadSave(ISerializerStream& stream)
    {
        stream.Exchange("secs", secs);
    }
}