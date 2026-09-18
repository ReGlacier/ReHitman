#include <Glacier/SSplineMover.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <cmath>
#include <cstdio>
#include <cstring>


namespace Glacier
{
    namespace
    {
        /**
         * @brief Checks whether the curve defined by the four control points is "straight enough",
         *        i.e. the directions to the end point are nearly parallel to the total direction.
         */
        bool IsStraightAngle(float f, const float* p0, const float* p1, const float* p2, const float* p3)
        {
            float d1[3];
            float d2[3];
            float d3[3];

            for (int i = 0; i < 3; ++i)
            {
                d1[i] = p1[i] - p0[i];
                d2[i] = p2[i] - p0[i];
                d3[i] = p3[i] - p0[i];
            }

            const float l1 = d1[0] * d1[0] + d1[1] * d1[1] + d1[2] * d1[2];
            const float l2 = d2[0] * d2[0] + d2[1] * d2[1] + d2[2] * d2[2];
            const float l3 = d3[0] * d3[0] + d3[1] * d3[1] + d3[2] * d3[2];

            if (l3 >= 0.001f)
            {
                const float d13 = d1[0] * d3[0] + d1[1] * d3[1] + d1[2] * d3[2];
                const float d23 = d2[0] * d3[0] + d2[1] * d3[1] + d2[2] * d3[2];

                if (l1 < 0.001f || (d13 * d13) / (l1 * l3) >= f)
                {
                    return (l2 < 0.001f) || (d23 * d23) / (l2 * l3) >= f;
                }

                return false;
            }

            return true;
        }

        /**
         * @brief Recursively subdivides the bezier via de Casteljau and samples the control
         *        points that are straight enough into the output buffer.
         */
        void RecSubImpl(ParametricCurve* pCurve, float** ppOut, int* pCount, const float* p0, const float* p1, const float* p2, const float* p3, float f, int iMaxDepth, int iDepth)
        {
            if (*pCount >= iMaxDepth - 1)
            {
                return;
            }

            if (IsStraightAngle(f, p0, p1, p2, p3))
            {
                if (*ppOut)
                {
                    std::memcpy(*ppOut, p0, sizeof(float) * 3);
                    *ppOut += 3;
                }
                ++*pCount;
                return;
            }

            float a[3];
            float b[3];
            float c[3];
            float d[3];
            float e[3];
            float g[3];

            for (int i = 0; i < 3; ++i)
            {
                a[i] = (p0[i] + p1[i]) * 0.5f;
                b[i] = (p1[i] + p2[i]) * 0.5f;
                c[i] = (a[i] + b[i]) * 0.5f;
                d[i] = (p2[i] + p3[i]) * 0.5f;
                e[i] = (b[i] + d[i]) * 0.5f;
                g[i] = (c[i] + e[i]) * 0.5f;
            }

            RecSubImpl(pCurve, ppOut, pCount, p0, a, c, g, f, iMaxDepth, iDepth);
            RecSubImpl(pCurve, ppOut, pCount, g, e, d, p3, f, iMaxDepth, iDepth);
        }
    }

    ParametricCurve::ParametricCurve()
    {
        m_fLen = -1.0f;
    }

    void ParametricCurve::LoadSave(ISerializerStream& stream)
    {
        stream.ExchangeArray("m_vA", &m_vA.x, 3);
        stream.ExchangeArray("m_vB", &m_vB.x, 3);
        stream.ExchangeArray("m_vC", &m_vC.x, 3);
        stream.ExchangeArray("m_vD", &m_vD.x, 3);
        stream.Exchange("m_fLen", m_fLen);
    }

    int ParametricCurve::RecSub(float* pOut, float* p0, float* p1, float* p2, float* p3, float f, int iMaxDepth, int iDepth)
    {
        float* pCur = pOut;
        int lCount = 0;

        RecSubImpl(this, &pCur, &lCount, p0, p1, p2, p3, f, iMaxDepth, iDepth);

        std::memcpy(pCur, p3, sizeof(float) * 3);
        return lCount + 1;
    }

    float ParametricCurve::CalcVal(float t) const
    {
        const float fT2 = t * t * 3.0f;
        const float fT = t + t;

        const float vx = m_vA.x * fT2 + m_vB.x * fT + m_vC.x;
        const float vy = m_vA.y * fT2 + m_vB.y * fT + m_vC.y;
        const float vz = m_vA.z * fT2 + m_vB.z * fT + m_vC.z;

        return std::sqrt(vx * vx + vy * vy + vz * vz);
    }

    float ParametricCurve::CalcLen(float fTess)
    {
        m_fLen = 0.0f;

        float t = 0.0f;
        float v0 = CalcVal(0.0f);

        while (t <= (1.0f - fTess))
        {
            t += fTess;
            const float v1 = CalcVal(t);
            m_fLen += (v0 + v1) * fTess * 0.5f;
            v0 = v1;
        }

        const float vFinal = CalcVal(1.0f);
        m_fLen += (v0 + vFinal) * (1.0f - t) * 0.5f;

        return m_fLen;
    }

    float ParametricCurve::GetLen() const
    {
        return m_fLen;
    }

    void ParametricCurve::DefineBezier(const float (*p0)[3], const float (*p1)[3], const float (*p2)[3], const float (*p3)[3], float fTess)
    {
        float aM0[3];
        float aM1[3];

        for (int i = 0; i < 3; ++i)
        {
            aM0[i] = (p1[0][i] - p0[0][i]) * 3.0f;
            aM1[i] = (p3[0][i] - p2[0][i]) * 3.0f;
        }

        DefineHermite(p0, p3, reinterpret_cast<const float(*)[3]>(aM0), reinterpret_cast<const float(*)[3]>(aM1), fTess);
    }

    void ParametricCurve::DefineHermite(const float (*p0)[3], const float (*p1)[3], const float (*m0)[3], const float (*m1)[3], float fTess)
    {
        m_vA.x = (2.0f * (p0[0][0] - p1[0][0]) + m0[0][0]) + m1[0][0];
        m_vA.y = (2.0f * (p0[0][1] - p1[0][1]) + m0[0][1]) + m1[0][1];
        m_vA.z = (2.0f * (p0[0][2] - p1[0][2]) + m0[0][2]) + m1[0][2];

        m_vB.x = (-3.0f * (p0[0][0] - p1[0][0]) - 2.0f * m0[0][0]) - m1[0][0];
        m_vB.y = (-3.0f * (p0[0][1] - p1[0][1]) - 2.0f * m0[0][1]) - m1[0][1];
        m_vB.z = (-3.0f * (p0[0][2] - p1[0][2]) - 2.0f * m0[0][2]) - m1[0][2];

        m_vC.x = m0[0][0];
        m_vC.y = m0[0][1];
        m_vC.z = m0[0][2];

        m_vD.x = p0[0][0];
        m_vD.y = p0[0][1];
        m_vD.z = p0[0][2];

        m_fLen = CalcLen(fTess);
    }

    void ParametricCurve::GetSplinePos(float (*pPos)[3], float t) const
    {
        if (t < 0.0f || t > 1.0f)
        {
            printf("WARNING ParametricCurve GetSplinePos with t = %.4f\n", t);
        }

        pPos[0][0] = ((m_vA.x * t + m_vB.x) * t + m_vC.x) * t + m_vD.x;
        pPos[0][1] = ((m_vA.y * t + m_vB.y) * t + m_vC.y) * t + m_vD.y;
        pPos[0][2] = ((m_vA.z * t + m_vB.z) * t + m_vC.z) * t + m_vD.z;
    }

    void ParametricCurve::GetSplineVelocity(float (*pVel)[3], float t) const
    {
        if (t < -0.00012207031f || t > 1.0001221f)
        {
            printf("WARNING ParametricCurve GetSplineVelocity with t = %.4f\n", t);
        }

        const float fT2 = t * t * 3.0f;
        const float fT = t + t;

        pVel[0][0] = m_vA.x * fT2 + m_vB.x * fT + m_vC.x;
        pVel[0][1] = m_vA.y * fT2 + m_vB.y * fT + m_vC.y;
        pVel[0][2] = m_vA.z * fT2 + m_vB.z * fT + m_vC.z;
    }
}
