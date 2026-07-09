#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/Geom/ZSTDOBJ.h>


namespace Hitman::BloodMoney
{
    class ZUsePoint : public Glacier::ZSTDOBJ
    {
    public:
        // vtbl
        // data
        float m_fProbability;
        bool m_bLocked;
        RE_ADD_PADDING(3);
        Glacier::ZREF m_rRelocBox;
        Glacier::ZGEOM* m_pRelocBox;
    };
    RE_VERIFY_SIZE(ZUsePoint, 0x20);
}