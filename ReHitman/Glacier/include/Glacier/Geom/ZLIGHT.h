#pragma once

#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/ReGlacier.h>

namespace Glacier {
    class ZLIGHT : public ZGEOM
    {
    public:
        STATIC_CLASS_VAR(ZLIGHT, uint32_t, m_Id);
        STATIC_CLASS_VAR(ZLIGHT, uint32_t, m_Mask);

        //vftable
        virtual void Enable();
        virtual void Disable();
        virtual void SetMultiplier(float fValue); // In default implementation it's do nothing
        virtual void SetDiffuseColor(unsigned int rgba); // In default implementation it's do nothing
        virtual void MakeLightUnique();

        //data (total size is 0x20, base size is 0x10)
        unsigned int m_lLightCon;
        unsigned int* m_pExcludeList;
        unsigned int m_rMasterLight;
        unsigned int* m_pStaticShadowList;
    };
    RE_VERIFY_SIZE(ZLIGHT, 0x20); // Verified
}
