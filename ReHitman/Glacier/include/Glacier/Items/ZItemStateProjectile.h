#pragma once

#include <Glacier/Items/ZItemState.h>

namespace Glacier
{
    class ZItemStateProjectile : public ZItemState
    {
    public:
        // PC RTTI: 0x773D9C; no additional instance data.
        ZGEOM* GetUseGeom(ZItem*) override;
    };
    RE_VERIFY_SIZE(ZItemStateProjectile, 0x5C);
}
