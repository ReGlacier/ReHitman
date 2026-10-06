#include <Glacier/RTP/Base.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/Serializer/IOutputSerializerStream.h>


namespace Glacier::RTP
{
    STATIC_CLASS_VAR_IMPL(cBase, ZPropertyInfo, Info, 0x007F580C, (ZPropertyInfo {
        .First = nullptr,
        .Super = nullptr,
        .Name = "RTP::cBase"
    }));

    void cBase::LoadObject(IInputSerializerStream& stream)
    {
        RTP::LoadSerializable(this, &GetProperties(), stream);
    }

    void cBase::SaveObject(IOutputSerializerStream& stream) 
    {
        RTP::SaveSerializable(this, &GetProperties(), stream);
    }

    const ZPropertyInfo& cBase::GetProperties() const
    {
        return cBase::Info;
    }
}
