#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/Serializer/ISerializerStream.h>


namespace Glacier
{
    ZDeltaBone::ZDeltaBone()
        : m_OffsetQuat{ 0.0f, 0.0f, 0.0f, 1.0f }
        , m_OffsetPos { 0.0f, 0.0f, 0.0f }
    {
    }

    ZDeltaBone::ZDeltaBone(const ZDeltaBone& copy)
    {
        memcpy(this, &copy, sizeof(ZDeltaBone));
    }

    ZDeltaBone& ZDeltaBone::operator=(const ZDeltaBone& copy)
    {
        memcpy(this, &copy, sizeof(ZDeltaBone));
        return *this;
    }

    void ZDeltaBone::LoadSave(ISerializerStream& stream, bool bSaving)
    {
        stream.ExchangeArray("m_OffsetQuat", m_OffsetQuat, 4);
        stream.ExchangeArray("m_OffsetPos", m_OffsetPos, 3);
    }
}