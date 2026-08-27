#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZSTDOBJ.h>

namespace Glacier
{
    class ZSNDOBJ : public ZSTDOBJ
    {
    public:
        STATIC_CLASS_VAR(ZSNDOBJ, uint32_t, m_Id);
        STATIC_CLASS_VAR(ZSNDOBJ, uint32_t, m_Mask);

        // vftable
        virtual void ModifyGeomCon(int AddBits, int RemoveBits);

        // methods
        bool AttachToDefaultRoom(bool bOverride);

        // data (total size is 0x1C, base size is 0x10)
        int m_AudioRef;
        int m_rZSoundRef;
        bool m_bPlayable;
        bool m_bActive;
        RE_ADD_PADDING(2);
    }; // Size 0x1C
    RE_VERIFY_SIZE(ZSNDOBJ, 0x1C); // Verified
}
