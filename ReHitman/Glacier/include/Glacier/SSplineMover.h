#pragma once

#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/GlacierFWD.h>


namespace Glacier
{
    struct ParametricCurve 
    {
        ZVector3 m_vA;
        ZVector3 m_vB;
        ZVector3 m_vC;
        ZVector3 m_vD;
        float m_fLen;

        virtual void LoadSave(ISerializerStream&);
        virtual int RecSub(float *,float *,float *,float *,float *,float,int,int);
    };
    RE_VERIFY_SIZE(ParametricCurve, 0x38);

    struct SSplineMover 
    {
        float t;
        float fDone;
        float fLen;
        int32_t iCurv;
        ParametricCurve Curve;
    };
    RE_VERIFY_SIZE(SSplineMover, 0x48);

}