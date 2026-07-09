#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/Geom/ZSTDOBJ.h>


namespace Glacier
{
    class ZSHADOWMESHOBJ : public ZSTDOBJ
    {
    public:
        // vtbl
        virtual bool GetParentBox(float *pMin, float *pMax);

        // data
        int m_iFlags;
        ZREF m_rLight;
        class ZLIGHT *m_pLight;
    }; // Verified size 0x1C
    RE_VERIFY_SIZE(ZSHADOWMESHOBJ, 0x1C);
}