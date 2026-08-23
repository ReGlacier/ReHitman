#pragma once

#include <Glacier/Geom/ZSTDOBJ.h>

namespace Glacier
{
    class ZBOUND : public ZSTDOBJ
    {
    public:
        // static
        static constexpr uint32_t m_TypeId = 0x20001Cu;
        //
        //
        STATIC_CLASS_VAR(ZBOUND, uint32_t, m_Id);
        STATIC_CLASS_VAR(ZBOUND, uint32_t, m_Mask);

        //vftable
        void Draw(ZDrawBuffer* pDrawBuffer, ZCAMERA* pCamera, const float* a2, const float* a3);
        //data (total size is 0x10, base size is 0x10)
    };
}
