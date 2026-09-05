#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/CBaseEvent.h>
#include <Glacier/Geom/ZGEOM.h>


namespace Hitman::BloodMoney
{
    class ZM13PosController : public Glacier::CBaseEvent<Glacier::ZGEOM>
    {
    public:
        // vtbl
        // data
        Glacier::ZGEOM* m_aActors[10];
        int m_iNrOfActors;
    }; // Size 0x5C
    RE_VERIFY_SIZE(ZM13PosController, 0x5C);
}