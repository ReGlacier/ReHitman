#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/GameBase/Locator.h>
#include <cstdint>


namespace Glacier
{
    class ZSeeable
    {
    public:
        // methods
        ZSeeable();
        ZSeeable(const ZSeeable& copy);

        void DecrementLocatorIndex();
        bool IsDecal() const;
        bool Exists() const;
        void LoadSave(ISerializerStream& stream, bool bSaving);

        // members
        union
        {
            ZGEOM* pSeeable;
            int16_t wTime;
            uint16_t wDecalType;
        };

        Locator locator;
        ZVector3 pos;
        uint16_t type;
    };
    RE_VERIFY_SIZE(ZSeeable, 0x18); // PC verified
    RE_VERIFY_OFFSET(ZSeeable, locator, 0x4);
    RE_VERIFY_OFFSET(ZSeeable, pos, 0x8);
    RE_VERIFY_OFFSET(ZSeeable, type, 0x14);
}
