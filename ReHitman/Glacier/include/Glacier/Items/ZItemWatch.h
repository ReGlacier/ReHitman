#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/CBaseEvent.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/Runtime/Macro.h>

namespace Glacier
{
    class ZItemWatch : public CBaseEvent<ZItem>
    {
    public:
        ZItemWatch();

        DECLARE_ROUT_CLASS(ZItemWatch, ZItem, ItemWatch, 0, 0);

        virtual const RTP::ZPropertyInfo& GetProperties() const override;

        void OnPickup();
        void OnPutdown();

        REFTAB m_Receivers;
    };
    RE_VERIFY_SIZE(ZItemWatch, 0x4C);
}
