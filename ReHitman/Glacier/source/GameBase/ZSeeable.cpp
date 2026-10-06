#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/GameBase/ZSeeable.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    ZSeeable::ZSeeable()
        : pSeeable(nullptr)
        , locator()
        , pos(0.0f)
        , type(0)
    {
    }

    ZSeeable::ZSeeable(const ZSeeable& copy)
        : pSeeable(copy.pSeeable)
        , locator(copy.locator)
        , pos(copy.pos)
        , type(copy.type)
    {
    }

    void ZSeeable::DecrementLocatorIndex()
    {
        ZASSERT(locator.IsValid());
        locator.DecrementCellIndex();
    }

    bool ZSeeable::IsDecal() const
    {
        return (type & 0x30) != 0;
    }

    bool ZSeeable::Exists() const
    {
        return pSeeable != nullptr;
    }

    void ZSeeable::LoadSave(ISerializerStream& stream, bool bSaving)
    {
        stream.Exchange("type", type);
        if (IsDecal())
        {
            stream.Exchange("wTime", wTime);
            stream.Exchange("wDecalType", wDecalType);
        }
        else
        {
            stream.Exchange("pSeeable", pSeeable);
        }
        stream.ExchangeArray("pos", pos.Get(), 3);
    }
}
