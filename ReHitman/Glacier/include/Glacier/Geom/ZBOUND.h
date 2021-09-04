#pragma once

#include <Glacier/Geom/ZSTDOBJ.h>

namespace Glacier {
    class ZBOUND : public ZSTDOBJ {
    public:
        //vftable
        void Draw(ZDrawBuffer* pDrawBuffer, ZCAMERA* pCamera, const float* a2, const float* a3);
        //data (total size is 0x10, base size is 0x10)
    };
}