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

        // methods
        ParametricCurve();

        float CalcVal(float t) const;
        float CalcLen(float fTess);
        float GetLen() const;

        void DefineBezier(const float (*p0)[3], const float (*p1)[3], const float (*p2)[3], const float (*p3)[3], float fTess);
        void DefineHermite(const float (*p0)[3], const float (*p1)[3], const float (*m0)[3], const float (*m1)[3], float fTess);
        void GetSplinePos(float (*pPos)[3], float t) const;
        void GetSplineVelocity(float (*pVel)[3], float t) const;
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