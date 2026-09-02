#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/GUI/ZWINOBJ.h>
#include <Glacier/Runtime/Macro.h>


namespace Glacier
{
    class ZWINPIC : public ZWINOBJ
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZWINPIC, 0x200046u);

        // vtbl
        virtual void SetUV(float u0, float v0, float u1, float v1);
        virtual void Mirror(bool bMirror);

        // data
        uint32_t m_iOriginalSizeX;
        uint32_t m_iOriginalSizeY;
        uint32_t m_dwModifications;
        ZVector4 m_v4UVModified;
    };
    RE_VERIFY_SIZE(ZWINPIC, 0xA4);
}
