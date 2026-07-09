#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZSTDOBJ.h>

namespace Glacier
{
    class ZSNDOBJ : public ZSTDOBJ
    {
    public:
        // vftable
        virtual void ModifyGeomCon(int, int);

        // api
        bool AttachToDefaultRoom(bool a1);

        // data (total size is 0x1C, base size is 0x10)
        int m_AudioRef;
        int m_rZSoundRef;
        bool m_bPlayable;
        bool m_bActive;
        RE_ADD_PADDING(2);
    }; // Size 0x1C 
    RE_VERIFY_SIZE(ZSNDOBJ, 0x1C); // Verified
}