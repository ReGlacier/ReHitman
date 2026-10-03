#include <Glacier/ZSTL/TIMETYPE.h>
#include <Glacier/Serializer/ISerializerStream.h>


namespace Glacier
{
    void ISerializerStream::Exchange(const char* psName, TIMETYPE& data)
    {
        Exchange(GetToken(psName), data);
    }

	void ISerializerStream::Exchange(const ZToken token, TIMETYPE& data)
	{
	    data.LoadSave(*this);
	}

    void TIMETYPE::LoadSave(ISerializerStream& stream)
    {
        stream.Exchange("secs", secs);
    }
}
