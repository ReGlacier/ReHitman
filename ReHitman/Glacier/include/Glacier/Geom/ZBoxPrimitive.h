#pragma once

#include <Glacier/Geom/ZSTDOBJ.h>
#include <Glacier/ZSTL/ZMath.h>

namespace Glacier
{
    class ZBoxPrimitive : public ZSTDOBJ
    {
    public:
        //vftable
        virtual void CalcPositionInBox(Glacier::ZVector3*);
        virtual void GetClosestPosDirInBox(Glacier::ZVector3*, float, Glacier::ZVector3*, float);
        virtual void SetScale(float xScale, float yScale, float zScale);
        virtual Glacier::ZVector3* GetScale(Glacier::ZVector3* outScale);

        //data (total size is 0x1C, original size is ox10)
        Glacier::ZVector3 m_scale;
    };
}