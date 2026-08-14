#pragma once

#include <Glacier/ZSTL/ZMemory.h>
#include <algorithm>
#include <cmath>


namespace Glacier
{
    struct Vector2
    {
        float x = 0.f, y = 0.f;

        bool operator==(const Vector2& with) const {
            return x == with.x && y == with.y;
        }

        Vector2& operator=(const Vector2& v)
        {
            x = v.x;
            y = v.y;

            return *this;
        }
    };

    struct Vector3
    {
        float x = 0.f, y = 0.f, z = 0.f;

        Vector3() = default;
        Vector3(float scalar) : x(scalar), y(scalar), z(scalar) {}
        Vector3(const float* pvPos) : x(pvPos[0]), y(pvPos[1]), z(pvPos[2]) {}
        Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

        bool operator==(const Vector3& with) const {
            return x == with.x && y == with.y && z == with.z;
        }

        Vector3& operator=(const float* p)
        {
            x = p[0];
            y = p[1];
            z = p[2];
            return *this;
        }

        Vector3& operator=(const Vector3& v)
        {
            x = v.x;
            y = v.y;
            z = v.z;

            return *this;
        }

        Vector3 operator-(const Vector3& other) const
        {
            return {
                x - other.x,
                y - other.y,
                z - other.z
            };
        }

        Vector3& operator-=(const Vector3& other)
        {
            x -= other.x;
            y -= other.y;
            z -= other.z;

            return *this;
        }

        Vector3& operator*(float fScalar)
        {
            x *= fScalar;
            y *= fScalar;
            z *= fScalar;
            return *this;
        }

        Vector3& operator/(float fScalar)
        {
            x /= fScalar;
            y /= fScalar;
            z /= fScalar;
            return *this;
        }

        Vector3& operator+(float fScalar)
        {
            x += fScalar;
            y += fScalar;
            z += fScalar;
            return *this;
        }

        Vector3& operator+=(const Vector3& v)
        {
            x += v.x;
            y += v.y;
            z += v.z;
            return *this;
        }

        float* Get() { return &x; }
        const float* Get() const { return &x; }

        Vector3& Reset()
        {
            x = y = z = 0.f;
            return *this;
        }

        inline float Length() const
        {
            return std::sqrtf((x * x)+ (y * y) + (z * z));
        }
    };

    struct Vector4
    {
        float x = 0.f, y = 0.f, z = 0.f, w = 0.f;

        Vector4() = default;
        Vector4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}

        bool operator==(const Vector4& with) const {
            return x == with.x && y == with.y && z == with.z && w == with.w;
        }

        Vector4& operator=(const Vector4& v)
        {
            x = v.x;
            y = v.y;
            z = v.z;
            w = v.w;

            return *this;
        }
    };

    struct Matrix3x3
    {
        float data[9];

        Matrix3x3() = default;
        Matrix3x3(float _m00, float _m01, float _m02, float _m10, float _m11, float _m12, float _m20, float _m21, float _m22)
        {
            #define FILL_GROUP(d, si, a1, a2, a3) d[si] = a1; d[si+1] = a2; d[si+2] = a3;
            FILL_GROUP(data, 0, _m00, _m01, _m02)
            FILL_GROUP(data, 3, _m10, _m11, _m12)
            FILL_GROUP(data, 6, _m20, _m21, _m22)
            #undef FILL_GROUP
        }

        bool operator==(const Matrix3x3& with) const {
            return std::equal(std::begin(data), std::end(data), std::begin(with.data), std::end(with.data));
        }

        const float* Get() const { return &data[0]; }

        Matrix3x3& operator=(const float* p)
        {
            std::memcpy(&data[0], p, sizeof(float) * 9);
            return *this;
        }

        Matrix3x3& operator*=(const Matrix3x3& mat)
        {
            const float zX = data[0];
            const float zY = data[1];
            const float zZ = data[2];
            const float yX = data[3];
            const float yY = data[4];
            const float yZ = data[5];
            const float xX = data[6];
            const float xY = data[7];
            const float xZ = data[8];

            data[6] = xX * mat.data[6] + xY * mat.data[7] + xZ * mat.data[8];
            data[7] = xX * mat.data[3] + xY * mat.data[4] + xZ * mat.data[5];
            data[8] = xX * mat.data[0] + xY * mat.data[1] + xZ * mat.data[2];

            data[3] = yX * mat.data[6] + yY * mat.data[7] + yZ * mat.data[8];
            data[4] = yX * mat.data[3] + yY * mat.data[4] + yZ * mat.data[5];
            data[5] = yX * mat.data[0] + yY * mat.data[1] + yZ * mat.data[2];

            data[0] = zX * mat.data[6] + zY * mat.data[7] + zZ * mat.data[8];
            data[1] = zX * mat.data[3] + zY * mat.data[4] + zZ * mat.data[5];
            data[2] = zX * mat.data[0] + zY * mat.data[1] + zZ * mat.data[2];

            return *this;
        }

        Matrix3x3& Reset()
        {
            data[0] = 0.0f;
            data[1] = 0.0f;
            data[2] = 1.0f;

            data[3] = 0.0f;
            data[4] = 1.0f;
            data[5] = 0.0f;

            data[6] = 1.0f;
            data[7] = 0.0f;
            data[8] = 0.0f;

            return *this;
        }

        Vector3& XAxis()
        {
            return *reinterpret_cast<Vector3*>(&data[6]);
        }

        Vector3& YAxis()
        {
            return *reinterpret_cast<Vector3*>(&data[3]);
        }

        Vector3& ZAxis()
        {
            return *reinterpret_cast<Vector3*>(&data[0]);
        }

        const Vector3& XAxis() const
        {
            return *reinterpret_cast<const Vector3*>(&data[6]);
        }

        const Vector3& YAxis() const
        {
            return *reinterpret_cast<const Vector3*>(&data[3]);
        }

        const Vector3& ZAxis() const
        {
            return *reinterpret_cast<const Vector3*>(&data[0]);
        }
    };

    struct Matrix4x4
    {
        float data[16];

        bool operator==(const Matrix4x4& with) const {
            return std::equal(std::begin(data), std::end(data), std::begin(with.data), std::end(with.data));
        }
    };

    using ZVector2 = Vector2;
    using ZVector3 = Vector3;
    using ZVector4 = Vector4;
    using ZMat4x4 = Matrix4x4;
    using ZMat3x3 = Matrix3x3;

    struct SMatPos
    {
        int m_id;
        ZMat3x3 transform;
        ZVector3 position;
    }; //size is 0x34

    struct ZQuat
    {
        float i { 0.f }, j { 0.f }, k { 0.f }, w { 1.f };
    };

    inline void qmul(float* out, const float* lhs, const float* rhs)
    {
        const float x = rhs[3] * lhs[0] + rhs[0] * lhs[3] + rhs[1] * lhs[2] - rhs[2] * lhs[1];
        const float y = rhs[3] * lhs[1] - rhs[0] * lhs[2] + rhs[1] * lhs[3] + rhs[2] * lhs[0];
        const float z = rhs[3] * lhs[2] + rhs[0] * lhs[1] - rhs[1] * lhs[0] + rhs[2] * lhs[3];
        const float w = rhs[3] * lhs[3] - rhs[0] * lhs[0] - rhs[1] * lhs[1] - rhs[2] * lhs[2];

        out[0] = x;
        out[1] = y;
        out[2] = z;
        out[3] = w;
    }

    inline void qmul(ZQuat& out, const ZQuat& lhs, const ZQuat& rhs)
    {
        qmul(&out.i, &lhs.i, &rhs.i);
    }

    inline void qrotaxis(float* out, const float* axis, float angle)
    {
        constexpr float TwoPi = 6.2831855f;
        constexpr float InvTwoPi = 0.15915494f;
        constexpr float Pi = 3.1415927f;

        float halfAngle = 0.0f;
        if (angle < 0.0f || angle >= TwoPi)
        {
            const float turns = angle * InvTwoPi;
            halfAngle = (turns - std::floor(turns)) * Pi;
        }
        else
        {
            halfAngle = angle * 0.5f;
        }

        const float s = std::sin(halfAngle);
        out[0] = axis[0] * s;
        out[1] = axis[1] * s;
        out[2] = axis[2] * s;
        out[3] = std::cos(halfAngle);
    }

    inline void qrotaxis(ZQuat& out, const float* axis, float angle)
    {
        qrotaxis(&out.i, axis, angle);
    }

    inline void quattomat(float* pMat, const float* pQuat)
    {
        const float twoW = pQuat[3] + pQuat[3];
        const float twoI = pQuat[0] + pQuat[0];
        const float twoJ = pQuat[1] + pQuat[1];

        const float twoWK = twoW * pQuat[2];
        const float twoIK = twoI * pQuat[2];
        const float twoJK = twoJ * pQuat[2];
        const float twoWW = twoW * pQuat[3];
        const float twoIW = twoI * pQuat[3];
        const float twoJW = twoJ * pQuat[3];
        const float twoII = twoI * pQuat[0];
        const float twoJI = twoJ * pQuat[0];
        const float twoJJ = twoJ * pQuat[1];

        pMat[6] = 1.0f - (twoJJ + twoII);
        pMat[7] = twoIW + twoJK;
        pMat[8] = twoJW - twoIK;
        pMat[3] = twoIW - twoJK;
        pMat[4] = 1.0f - (twoJJ + twoWW);
        pMat[5] = twoWK + twoJI;
        pMat[0] = twoJW + twoIK;
        pMat[1] = twoJI - twoWK;
        pMat[2] = 1.0f - (twoII + twoWW);
    }

    inline void quattomat(ZMat3x3& mat, const ZQuat& quat)
    {
        quattomat(mat.data, &quat.i);
    }

    struct ZMatrix
    {
        ZMat3x3 m0;
        ZVector3 p0;
    };


    inline void TransformRootVector(ZVector3& vec, const ZMat3x3& mat) // GetRootVec_Asm
    {
        const float x = vec.x;
        const float y = vec.y;
        const float z = vec.z;

        vec.x = x * mat.data[6] + y * mat.data[3] + z * mat.data[0];
        vec.y = x * mat.data[7] + y * mat.data[4] + z * mat.data[1];
        vec.z = x * mat.data[8] + y * mat.data[5] + z * mat.data[2];
    }

    inline void TransformLocalVector(ZVector3& vec, const ZMat3x3& mat) // vmtmul
    {
        const float x = vec.x;
        const float y = vec.y;
        const float z = vec.z;

        vec.x = x * mat.data[6] + y * mat.data[7] + z * mat.data[8];
        vec.y = x * mat.data[3] + y * mat.data[4] + z * mat.data[5];
        vec.z = x * mat.data[0] + y * mat.data[1] + z * mat.data[2];
    }

    inline void TransformLocalMatrix(ZMat3x3& mat, const ZMat3x3& rootMat)
    {
        TransformLocalVector(mat.XAxis(), rootMat);
        TransformLocalVector(mat.YAxis(), rootMat);
        TransformLocalVector(mat.ZAxis(), rootMat);
    }

#   pragma region " --- Glacier pure math --- " // NOTE: Need optimize all this stuff to AVX2 (?)
    inline void vmmul(float* out, const float* in, const float* mat)
    {
        const float x = in[0];
        const float y = in[1];
        const float z = in[2];

        out[0] = x * mat[6] + y * mat[3] + z * mat[0];
        out[1] = x * mat[7] + y * mat[4] + z * mat[1];
        out[2] = x * mat[8] + y * mat[5] + z * mat[2];
    }

    inline void vmmul(float* vec, const float* mat)
    {
        const float x = vec[0];
        const float y = vec[1];
        const float z = vec[2];

        vec[0] = x * mat[6] + y * mat[3] + z * mat[0];
        vec[1] = x * mat[7] + y * mat[4] + z * mat[1];
        vec[2] = x * mat[8] + y * mat[5] + z * mat[2];
    }

    inline void vmtmul(float* out, const float* in, const float* mat)
    {
        const float x = in[0];
        const float y = in[1];
        const float z = in[2];

        out[0] = x * mat[6] + y * mat[7] + z * mat[8];
        out[1] = x * mat[3] + y * mat[4] + z * mat[5];
        out[2] = x * mat[0] + y * mat[1] + z * mat[2];
    }

    inline void vmtmul(float* vec, const float* mat)
    {
        const float x = vec[0];
        const float y = vec[1];
        const float z = vec[2];

        vec[0] = x * mat[6] + y * mat[7] + z * mat[8];
        vec[1] = x * mat[3] + y * mat[4] + z * mat[5];
        vec[2] = x * mat[0] + y * mat[1] + z * mat[2];
    }

    inline void TransformBox(const float* mat, float* size)
    {
        const float x = size[0];
        const float y = size[1];
        const float z = size[2];

        size[0] = std::fabs(x * mat[6]) + std::fabs(y * mat[3]) + std::fabs(z * mat[0]);
        size[1] = std::fabs(x * mat[7]) + std::fabs(y * mat[4]) + std::fabs(z * mat[1]);
        size[2] = std::fabs(x * mat[8]) + std::fabs(y * mat[5]) + std::fabs(z * mat[2]);
    }
#   pragma endregion
}

/*
DronCode: List of math methods to reverse (MiniNinjas)
vscalar(float *,float)	.text	820EA3D8	00000028	00000000	00000001	R	.	.	.	.	.	.	T	.	.
vscalar(float *,float const *,float)	.text	820EA3B0	00000028	00000000	00000001	R	.	.	.	.	.	.	T	.	.
vrotz(float *,float const *,float)	.text	820EBA00	0000008C	00000080		R	.	.	.	.	.	.	T	.	.
vroty(float *,float)	.text	820EB990	00000070	00000070		R	.	.	.	.	.	.	T	.	.
vrot(float *,float const *,float const *)	.text	820EB888	00000108	00000090		R	.	.	.	.	.	.	T	.	.
vrot(float *,float const *)	.text	820EB7C0	000000C8	00000080		R	.	.	.	.	.	.	T	.	.
vreset(float * const)	.text	820DA1F8	00000018	00000000	00000001	R	.	.	.	.	.	.	T	.	.
vrand(float *)	.text	820ED6E8	00000110	00000080		R	.	.	.	.	.	.	T	.	.
vnorm2(float *)	.text	820EAE68	00000044	00000000	00000001	R	.	.	.	.	.	.	T	.	.
vnorm(float *,float const *)	.text	820EADA8	00000070	00000000	00000001	R	.	.	.	.	.	.	T	.	.
vnorm(float *)	.text	820EAD40	00000068	00000000	00000001	R	.	.	.	.	.	.	T	.	.
vneg(float *,float const *)	.text	820EA428	00000028	00000000	00000001	R	.	.	.	.	.	.	T	.	.
vneg(float *)	.text	820EA400	00000028	00000000	00000001	R	.	.	.	.	.	.	T	.	.
vmul(float *,float const *,float const *)	.text	820EA640	00000034	00000000	00000001	R	.	.	.	.	.	.	T	.	.
vmul(float *,float const *)	.text	820EA450	00000034	00000000	00000001	R	.	.	.	.	.	.	T	.	.
vmtmul(float *,float const *,float const *)	.text	820EAAD0	00000064	00000000	00000001	R	.	.	.	.	.	.	T	.	.
vmtmul(float *,float const *)	.text	820DA230	00000064	00000000	00000001	R	.	.	.	.	.	.	T	.	.
vmmul(float *,float const *,float const *)	.text	820EAB38	00000064	00000000	00000001	R	.	.	.	.	.	.	T	.	.
vmin(float *,float const *)	.text	820EA5B0	00000040	00000000	00000001	R	.	.	.	.	.	.	T	.	.
vmax(float *,float const *)	.text	820EA570	00000040	00000000	00000001	R	.	.	.	.	.	.	T	.	.
vlen2(float const *)	.text	820DA210	0000001C	00000000	00000001	R	.	.	.	.	.	.	T	.	.
vfscanf	.text	8241D550	000001AC	000000A0		R	.	.	.	.	.	.	T	.	.
vdist2d(float const *,float const *)	.text	820EAC90	00000028	00000000	00000001	R	.	.	.	.	.	.	T	.	.
vdist2(float const *,float const *)	.text	820EACB8	00000034	00000000	00000001	R	.	.	.	.	.	.	T	.	.
vdist(float const *,float const *)	.text	820EAC58	00000038	00000000	00000001	R	.	.	.	.	.	.	T	.	.
vcross(float *,float const *,float const *)	.text	820EA4F8	00000058	00000000	00000001	R	.	.	.	.	.	.	T	.	.
vcpy<float,float>(float *,float const *)	.text	820D96D8	0000001C	00000000	00000001	R	.	.	.	.	.	.	T	.	.
vangularpul(float * const,float const * const,float const * const,float,float)	.text	820ED7F8	00000314	000000F0		R	.	.	.	.	.	.	T	.	.
vangpul(float * const,float const * const,float const * const,float)	.text	820EB590	00000230	000000E0		R	.	.	.	.	.	.	T	.	.
vangle(float const * const,float const * const)	.text	820EAC00	00000058	00000000	00000001	R	.	.	.	.	.	.	T	.	.
vaddscalar(float *,float const *,float)	.text	820EA488	00000034	00000000	00000001	R	.	.	.	.	.	.	T	.	.
vaddscalar(float *,float const *,float const *,float)	.text	820EA4C0	00000034	00000000	00000001	R	.	.	.	.	.	.	T	.	.
vadd(float *,float const *)	.text	820D9668	00000034	00000000	00000001	R	.	.	.	.	.	.	T	.	.
vabs(float *,float const *)	.text	820EAD18	00000028	00000000	00000001	R	.	.	.	.	.	.	T	.	.
vabs(float *)	.text	820EACF0	00000028	00000000	00000001	R	.	.	.	.	.	.	T	.	.
toupper	.text	8241DB40	00000018	00000000	00000001	R	.	.	.	.	.	.	T	.	.
tolower	.text	8241ED38	00000018	00000000	00000001	R	.	.	.	.	.	.	T	.	.
tmat(float *,float const *)	.text	820EA5F0	0000004C	00000000	00000001	R	.	.	.	.	.	.	T	.	.
terminate(void)	.text	82426F58	00000058	00000060		.	.	.	.	.	.	.	T	.	.
tanh	.text	8265AD20	00000100	00000070		R	.	.	.	.	.	.	T	.	.
tan	.text	8241CB30	000000D4	00000010		R	.	.	.	.	.	.	T	.	.
*/
