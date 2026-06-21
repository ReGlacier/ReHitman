#pragma once

#include <Glacier/Geom/ZSTDOBJ.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ReGlacier.h>

namespace Glacier
{
    class ZBoxPrimitive : public ZSTDOBJ
    {
    public:
        //vftable
        virtual void CalcPositionInBox(ZVector3*);
        virtual bool GetClosestPosDirInBox(ZVector3* outPos, float fUnk, ZVector3* outDir, ZVector3* pInPos);
        virtual void SetScale(float x, float y, float z);
        virtual Glacier::ZVector3* GetScale(ZVector3* outScale);

        //data (total size is 0x1C, original size is ox10)
        Glacier::ZVector3 m_vScale;
    };
    RE_VERIFY_SIZE(ZBoxPrimitive, 0x1C); // Verified
    RE_VERIFY_OFFSET(ZBoxPrimitive, m_vScale, 0x10);
}