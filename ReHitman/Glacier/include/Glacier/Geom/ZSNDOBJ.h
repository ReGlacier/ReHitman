#pragma once

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
        int m_field10;
        int m_field14;
        int m_field18;
    };
}