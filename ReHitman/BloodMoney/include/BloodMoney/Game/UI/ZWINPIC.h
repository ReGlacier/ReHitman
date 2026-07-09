#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <BloodMoney/Game/UI/ZWINOBJ.h>


namespace Hitman::BloodMoney
{
    class ZWINPIC : public ZWINOBJ
    {
    public:
        // vtbl
        virtual void SetUV(float, float, float, float);
        virtual void Mirror(bool);
        
        // data
        uint32_t m_iOriginalSizeX;
        uint32_t m_iOriginalSizeY;
        uint32_t m_dwModifications;
        Glacier::ZVector4 m_v4UVModified;
    };
    RE_VERIFY_SIZE(ZWINPIC, 0xA4);
}
