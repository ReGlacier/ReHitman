#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZSTL/ZMemory.h>
#include <algorithm>
#include <numbers>
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

        operator const float*() const
        {
            return &x;
        }

        operator float*()
        {
            return &x;
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

        Vector3 operator*(float fScalar) const
        {
            return Vector3(x * fScalar, y * fScalar, z * fScalar);
        }

        Vector3 operator+(const Vector3& other) const
        {
            return Vector3(x + other.x, y + other.y, z + other.z);
        }

        friend Vector3 operator*(float fScalar, const Vector3& v)
        {
            return v * fScalar;
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

        Matrix3x3(const float* m)
        {
            memcpy(&data[0], m, sizeof(data));
        }

        Matrix3x3& operator=(const float* m)
        {
            memcpy(&data[0], m, sizeof(data));
            return *this;
        }

        Matrix3x3 TransposedAntidiagonal() const
        {
            return {
                data[8], data[5], data[2],
                data[7], data[4], data[1],
                data[6], data[3], data[0]
            };
        }

        Matrix3x3& SetRow(int lRow, const Vector3& vRow)
        {
            const int idx = lRow * 3;
            data[idx]     = vRow.x;
            data[idx + 1] = vRow.y;
            data[idx + 2] = vRow.z;
            return *this;
        }

        bool operator==(const Matrix3x3& with) const {
            return std::equal(std::begin(data), std::end(data), std::begin(with.data), std::end(with.data));
        }

        const float* Get() const { return &data[0]; }
        float* Get() { return &data[0]; }

        operator float*() { return &data[0]; }
        operator const float*() const { return &data[0]; }

        Matrix3x3& operator*=(const Matrix3x3& mat)
        {
            const float zX = data[0], zY = data[1], zZ = data[2];
            const float yX = data[3], yY = data[4], yZ = data[5];
            const float xX = data[6], xY = data[7], xZ = data[8];

            // X Axis
            data[6] = xX * mat.data[6] + xY * mat.data[3] + xZ * mat.data[0];
            data[7] = xX * mat.data[7] + xY * mat.data[4] + xZ * mat.data[1];
            data[8] = xX * mat.data[8] + xY * mat.data[5] + xZ * mat.data[2];

            // Y Axis
            data[3] = yX * mat.data[6] + yY * mat.data[3] + yZ * mat.data[0];
            data[4] = yX * mat.data[7] + yY * mat.data[4] + yZ * mat.data[1];
            data[5] = yX * mat.data[8] + yY * mat.data[5] + yZ * mat.data[2];

            // Z Axis
            data[0] = zX * mat.data[6] + zY * mat.data[3] + zZ * mat.data[0];
            data[1] = zX * mat.data[7] + zY * mat.data[4] + zZ * mat.data[1];
            data[2] = zX * mat.data[8] + zY * mat.data[5] + zZ * mat.data[2];

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

        Vector3& Row(int lRow)
        {
            assert(lRow >= 0 && lRow < 3);
            return *reinterpret_cast<Vector3*>(data + lRow * 3);
        }

        const Vector3& Row(int lRow) const
        {
            assert(lRow >= 0 && lRow < 3);
            return *reinterpret_cast<const Vector3*>(data + lRow * 3);
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

        operator float*() { return &i; }
        operator const float*() const { return &i; }
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
        const float twoK = pQuat[2] + pQuat[2];

        const float twoWK = twoW * pQuat[2];
        const float twoIK = twoI * pQuat[2];
        const float twoJK = twoJ * pQuat[2];
        const float twoKK = twoK * pQuat[2];
        const float twoIW = twoI * pQuat[3];
        const float twoJW = twoJ * pQuat[3];
        const float twoII = twoI * pQuat[0];
        const float twoJI = twoJ * pQuat[0];
        const float twoJJ = twoJ * pQuat[1];

        pMat[6] = 1.0f - (twoJJ + twoII);
        pMat[7] = twoIW + twoJK;
        pMat[8] = twoJW - twoIK;
        pMat[3] = twoIW - twoJK;
        pMat[4] = 1.0f - (twoJJ + twoKK);
        pMat[5] = twoWK + twoJI;
        pMat[0] = twoJW + twoIK;
        pMat[1] = twoJI - twoWK;
        pMat[2] = 1.0f - (twoII + twoKK);
    }

    inline void quattomat(ZMat3x3& mat, const ZQuat& quat)
    {
        quattomat(mat.data, &quat.i);
    }

    struct ZMatrix
    {
        ZMat3x3 m0;
        ZVector3 p0;

        ZMatrix& operator=(const ZMatrix& copy)
        {
            // Or just memcpy?
            m0 = copy.m0;
            p0 = copy.p0;
            return *this;
        }

        void Reset()
        {
            m0.Reset();
            p0.Reset();
        }
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

    inline void TransformRootVector(float* vec, const float* mat)
    {
        ZVector3* pVec = reinterpret_cast<ZVector3*>(vec);
        const ZMat3x3* pMat = reinterpret_cast<const ZMat3x3*>(mat);

        TransformRootVector(*pVec, *pMat);
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
    /**
     * @brief Copies 3-component vector: dst = src (PS2: vcpy<float,float>)
     */
    inline void vcpy(float* dst, const float* src)
    {
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
    }

    /**
     * @brief Sets 3-component vector components (PS2: vset)
     */
    inline void vset(float* vec, float x, float y, float z)
    {
        vec[0] = x;
        vec[1] = y;
        vec[2] = z;
    }

    /**
     * @brief Zeroes 3-component vector (PS2: vreset)
     */
    inline void vreset(float* vec)
    {
        vec[0] = 0.0f;
        vec[1] = 0.0f;
        vec[2] = 0.0f;
    }

    /**
     * @brief Component-wise minimum: vec = min(vec, b) (PS2: vmin)
     */
    inline void vmin(float* vec, const float* b)
    {
        vec[0] = (std::min)(vec[0], b[0]);
        vec[1] = (std::min)(vec[1], b[1]);
        vec[2] = (std::min)(vec[2], b[2]);
    }

    /**
     * @brief Component-wise maximum: vec = max(vec, b) (PS2: vmax)
     */
    inline void vmax(float* vec, const float* b)
    {
        vec[0] = (std::max)(vec[0], b[0]);
        vec[1] = (std::max)(vec[1], b[1]);
        vec[2] = (std::max)(vec[2], b[2]);
    }

    /**
     * @brief out = a * scalar (PS2: vscalar)
     */
    inline void vscalar(float* out, const float* a, float scalar)
    {
        out[0] = a[0] * scalar;
        out[1] = a[1] * scalar;
        out[2] = a[2] * scalar;
    }

    /**
     * @brief vec *= scalar (PS2: vscalar)
     */
    inline void vscalar(float* vec, float scalar)
    {
        vec[0] *= scalar;
        vec[1] *= scalar;
        vec[2] *= scalar;
    }

    /**
     * @brief Dot product of two 3-component vectors (PS2: vdot)
     */
    inline float vdot(const float* a, const float* b)
    {
        return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
    }

    /**
     * @brief Cross product: out = a x b (PS2: vcross)
     */
    inline void vcross(float* out, const float* a, const float* b)
    {
        out[0] = a[1] * b[2] - a[2] * b[1];
        out[1] = a[2] * b[0] - a[0] * b[2];
        out[2] = a[0] * b[1] - a[1] * b[0];
    }

    /**
     * @brief Length of 3-component vector (PS2: vlen)
     */
    inline float vlen(const float* vec)
    {
        return std::sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
    }

    /**
     * @brief Squared length of 3-component vector (PS2: vlen2)
     */
    inline float vlen2(const float* vec)
    {
        return vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2];
    }

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

    /**
     * @brief Matrix multiply with transposed second operand: out = a * b^T (PC 0x436D80).
     * @note out must not alias a or b.
     */
    inline void mmtmul(float* out, const float* a, const float* b)
    {
        ZASSERT(out != a && out != b);

        vmtmul(&out[6], &a[6], b);
        vmtmul(&out[3], &a[3], b);
        vmtmul(out, a, b);
    }

    inline void mmtmul(ZMat3x3& out, const ZMat3x3& a, const ZMat3x3& b)
    {
        mmtmul(out.data, a.data, b.data);
    }

    inline void vrot(float* vec, const float* rot)
    {
        float sinX = sin(rot[0]);
        float cosX = cos(rot[0]);
        float sinY = sin(rot[1]);
        float cosY = cos(rot[1]);
        float sinZ = sin(rot[2]);
        float cosZ = cos(rot[2]);

        float vx = vec[0];
        float vy = vec[1];
        float vz = vec[2];

        // Rotate around Z-Axis
        float x1 =  cosZ * vx - sinZ * vy;
        float y1 =  sinZ * vx + cosZ * vy;

        // Rotate around X-Axis
        float y2 =  cosX * y1 - sinX * vz;
        float z2 =  sinX * y1 + cosX * vz;

        // Rotate around Y-Axis
        vec[0] =  cosY * x1 + sinY * z2;
        vec[1] =  y2;
        vec[2] =  cosY * z2 - sinY * x1;
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

    inline void vsub(float* out, const float* a, const float* b)
    {
        out[0] = a[0] - b[0];
        out[1] = a[1] - b[1];
        out[2] = a[2] - b[2];
    }

    inline void vsub(float* vec, const float* b)
    {
        vec[0] -= b[0];
        vec[1] -= b[1];
        vec[2] -= b[2];
    }

    inline bool vzero(const float* v)
    {
        return v[0] == 0.0f && v[1] == 0.0f && v[2] == 0.0f;
    }

    /**
     * @brief out = a + b (PS2: vadd)
     */
    inline void vadd(float* out, const float* a, const float* b)
    {
        out[0] = a[0] + b[0];
        out[1] = a[1] + b[1];
        out[2] = a[2] + b[2];
    }

    inline void vadd(float* vec, const float* b)
    {
        vec[0] += b[0];
        vec[1] += b[1];
        vec[2] += b[2];
    }

    /**
     * @brief out = a + b * scalar (PC: 0x428690)
     */
    inline void vaddscalar(float* out, const float* a, const float* b, float scalar)
    {
        out[0] = a[0] + b[0] * scalar;
        out[1] = a[1] + b[1] * scalar;
        out[2] = a[2] + b[2] * scalar;
    }

    /**
     * @brief Scales a 3-component vector to the requested length (PS2: vsetlen).
     */
    inline void vsetlen(float* vec, float len)
    {
        const float l = std::sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
        if (l != 0.0f)
        {
            const float s = len / l;
            vec[0] *= s;
            vec[1] *= s;
            vec[2] *= s;
        }
    }

    /**
     * @brief Rotates a 3-component vector by a unit quaternion (PS2: qtran).
     *
     * @param[out] out  Result vector (3 floats).
     * @param[in]  quat Unit quaternion (4 floats: x, y, z, w).
     * @param[in]  vec  Input vector (3 floats).
     */
    inline void qtran(float* out, const float* quat, const float* vec)
    {
        const float xx = quat[0] * quat[0] * 2.0f;
        const float xy = quat[0] * quat[1] * 2.0f;
        const float xz = quat[0] * quat[2] * 2.0f;
        const float xw = quat[0] * quat[3] * 2.0f;
        const float yy = quat[1] * quat[1] * 2.0f;
        const float yz = quat[1] * quat[2] * 2.0f;
        const float yw = quat[1] * quat[3] * 2.0f;
        const float zz = quat[2] * quat[2] * 2.0f;
        const float zw = quat[2] * quat[3] * 2.0f;

        out[0] = (1.0f - zz - yy) * vec[0] + (xy - zw) * vec[1] + (xz + yw) * vec[2];
        out[1] = (zw + xy) * vec[0] + (1.0f - zz - xx) * vec[1] + (yz - xw) * vec[2];
        out[2] = (xz - yw) * vec[0] + (yz + xw) * vec[1] + (1.0f - yy - xx) * vec[2];
    }

    /**
     * @brief Normalizes pVec into pRes.
     * @return Length of the source vector (0.0f when pVec is a zero vector; pRes is zeroed then).
     */
    inline float vnorm(float* pRes, const float* pVec)
    {
        const float fLength = std::sqrt(pVec[0] * pVec[0] + pVec[1] * pVec[1] + pVec[2] * pVec[2]);

        if (fLength == 0.0f)
        {
            pRes[0] = 0.0f;
            pRes[1] = 0.0f;
            pRes[2] = 0.0f;
        }
        else
        {
            const float fInvLength = 1.0f / fLength;

            pRes[0] = pVec[0] * fInvLength;
            pRes[1] = fInvLength * pVec[1];
            pRes[2] = fInvLength * pVec[2];
        }

        return fLength;
    }

    /**
     * @brief Normalizes vector in-place (PS2: vnorm)
     * @return Length of the source vector (0.0f when the vector is a zero vector).
     */
    inline float vnorm(float* vec)
    {
        return vnorm(vec, vec);
    }

    /**
     * @brief Compares two 3-component vectors for exact equality (PS2: vcmp).
     */
    inline bool vcmp(const float* a, const float* b)
    {
        return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
    }

    /**
     * @brief Copies 3x3 matrix (PS2: mcpy).
     */
    inline void mcpy(float* dst, const float* src)
    {
        memcpy(dst, src, sizeof(float) * 9);
    }

    /**
     * @brief Builds a rotation matrix from a unit axis and an angle in degrees
     *        (Rodrigues' formula, PC: 0x435B40).
     *        Unlike mrotaxis2, takes the angle in degrees and computes sin/cos itself,
     *        and the axis is passed as three separate floats instead of a pointer.
     *        Result uses the Glacier transposed storage (rows at [6], [3], [0]).
     */
    inline void mrotaxis(float* pMat, float fDeg, float fAxisX, float fAxisY, float fAxisZ)
    {
        const float fAngle = fDeg * 0.017453292f; // deg -> rad
        const float fSin = std::sin(fAngle);
        const float fCos = std::cos(fAngle);
        const float fOmCos = 1.0f - fCos;

        pMat[6] = fAxisX * fAxisX * fOmCos + fCos;
        pMat[7] = fAxisX * fAxisY * fOmCos - fAxisZ * fSin;
        pMat[8] = fAxisX * fAxisZ * fOmCos + fAxisY * fSin;
        pMat[3] = fAxisY * fAxisX * fOmCos + fAxisZ * fSin;
        pMat[4] = fAxisY * fAxisY * fOmCos + fCos;
        pMat[5] = fAxisY * fAxisZ * fOmCos - fAxisX * fSin;
        pMat[0] = fAxisZ * fAxisX * fOmCos - fAxisY * fSin;
        pMat[1] = fAxisZ * fAxisY * fOmCos + fAxisX * fSin;
        pMat[2] = fAxisZ * fAxisZ * fOmCos + fCos;
    }

    /**
     * @brief Builds rotation matrix from unit axis and angle (Rodrigues' formula, PS2: mrotaxis2).
     *        Uses the Glacier transposed storage (rows at [6],[3],[0]).
     */
    inline void mrotaxis2(float cosAngle, float sinAngle, const float* axis, float* out)
    {
        const float t = 1.0f - cosAngle;

        out[6] = axis[0] * axis[0] * t + cosAngle;
        out[7] = axis[0] * axis[1] * t - axis[2] * sinAngle;
        out[8] = axis[0] * axis[2] * t + axis[1] * sinAngle;
        out[3] = axis[1] * axis[0] * t + axis[2] * sinAngle;
        out[4] = axis[1] * axis[1] * t + cosAngle;
        out[5] = axis[1] * axis[2] * t - axis[0] * sinAngle;
        out[0] = axis[2] * axis[0] * t - axis[1] * sinAngle;
        out[1] = axis[2] * axis[1] * t + axis[0] * sinAngle;
        out[2] = axis[2] * axis[2] * t + cosAngle;
    }

    /**
     * @brief Matrix multiply in Glacier convention: out = a * b (PS2: mmmul).
     * @note Safe when out aliases a (rows are computed independently).
     */
    inline void mmmul(float* out, const float* a, const float* b)
    {
        vmmul(&out[6], &a[6], b);
        vmmul(&out[3], &a[3], b);
        vmmul(out, a, b);
    }

    /**
     * @brief Matrix multiply in-place in Glacier convention: mat *= b (PS2: mmmul).
     */
    inline void mmmul(float* mat, const float* b)
    {
        mmmul(mat, mat, b);
    }

    /**
     * @brief Reset matrix to identity (PC: mreset).
     */
    inline void mreset(float* mat)
    {
        mat[7] = 0.0f;
        mat[8] = 0.0f;
        mat[6] = 1.0f;
        mat[3] = 0.0f;
        mat[4] = 1.0f;
        mat[5] = 0.0f;
        mat[0] = 0.0f;
        mat[1] = 0.0f;
        mat[2] = 1.0f;
    }

    /**
     * @brief Transpose matrix (PC: tmat).
     */
    inline void tmat(float* out, const float* in)
    {
        ZASSERT(out != in);

        out[6] = in[6];
        out[3] = in[7];
        out[0] = in[8];
        out[7] = in[3];
        out[4] = in[4];
        out[1] = in[5];
        out[8] = in[0];
        out[5] = in[1];
        out[2] = in[2];
    }

    /**
     * @brief Convert matrix to quaternion (PC: mattoquat).
     */
    inline void mattoquat(float* pQuat, const float* pMat)
    {
        const float m00 = pMat[6];
        const float m01 = pMat[7];
        const float m02 = pMat[8];
        const float m10 = pMat[3];
        const float m11 = pMat[4];
        const float m12 = pMat[5];
        const float m20 = pMat[0];
        const float m21 = pMat[1];
        const float m22 = pMat[2];

        const float trace = m00 + m11 + m22;
        if (trace > 0.0f)
        {
            const float s = std::sqrt(trace + 1.0f);
            pQuat[3] = 0.5f * s;
            const float invS = 0.5f / s;
            pQuat[0] = (m12 - m21) * invS;
            pQuat[1] = (m20 - m02) * invS;
            pQuat[2] = (m01 - m10) * invS;
        }
        else
        {
            int i = 0;
            if (m11 > m00)
                i = 1;
            if (m22 > (i == 0 ? m00 : m11))
                i = 2;

            const int j = (i + 1) % 3;
            const int k = (i + 2) % 3;

            const float* m[3] = { &pMat[6], &pMat[3], &pMat[0] };
            const float s = std::sqrt(m[i][i] - (m[j][j] + m[k][k]) + 1.0f);
            pQuat[i] = 0.5f * s;
            const float invS = 0.5f / s;
            pQuat[3] = (m[j][k] - m[k][j]) * invS;
            pQuat[j] = (m[i][j] + m[j][i]) * invS;
            pQuat[k] = (m[i][k] + m[k][i]) * invS;
        }
    }

    /**
     * @brief Returns distance between a and b, i.e. |a - b| (PC: 0x436980)
     */
    inline float vdist(const float* a, const float* b)
    {
        const float dx = a[0] - b[0];
        const float dy = a[1] - b[1];
        const float dz = a[2] - b[2];

        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    /**
     * @brief Transforms a world-space point into matrix-local space: pOut = (pIn - mat.p0) * mat.m0^T.
     */
    inline void MatrixTransformInverse(float* pOut, const float* pIn, const ZMatrix& mat)
    {
        vsub(pOut, pIn, mat.p0.Get());
        vmtmul(pOut, mat.m0.Get());
    }

    /**
     * @brief Inverts a 3x3 system (adjugate / determinant).
     * @return false when the matrix is (near-)singular, true otherwise.
     */
    inline bool Invert3x3System(float* inv, const float* m)
    {
        const float det = m[7] * m[5] * m[0]
            - m[3] * m[8] * m[1]
            - m[6] * m[4] * m[2]
            + m[8] * m[4] * m[0]
            + m[3] * m[7] * m[2]
            + m[6] * m[5] * m[1];

        if (std::fabs(det) < 0.000001f)
        {
            return false;
        }

        const float fInvDet = 1.0f / det;

        inv[0] = (m[4] * m[8] - m[5] * m[7]) * fInvDet;
        inv[1] = (m[7] * m[2] - m[8] * m[1]) * fInvDet;
        inv[2] = (m[5] * m[1] - m[4] * m[2]) * fInvDet;
        inv[3] = (m[5] * m[6] - m[8] * m[3]) * fInvDet;
        inv[4] = (m[8] * m[0] - m[6] * m[2]) * fInvDet;
        inv[5] = (m[3] * m[2] - m[5] * m[0]) * fInvDet;
        inv[6] = (m[3] * m[7] - m[4] * m[6]) * fInvDet;
        inv[7] = (m[1] * m[6] - m[0] * m[7]) * fInvDet;
        inv[8] = (m[4] * m[0] - m[3] * m[1]) * fInvDet;

        return true;
    }

    /**
     * @brief Computes the normalized face normal of a triangle (PS2: CalcNormal).
     *        Falls back to the +X axis for degenerate triangles.
     */
    inline void CalcNormal(float* pOutNormal, const float* pVert0, const float* pVert1, const float* pVert2)
    {
        float vEdge0[3];
        float vEdge1[3];

        vsub(vEdge0, pVert1, pVert0);
        vsub(vEdge1, pVert2, pVert0);
        vcross(pOutNormal, vEdge1, vEdge0);

        if (vnorm(pOutNormal) < 0.99998999f)
        {
            vset(pOutNormal, 1.0f, 0.0f, 0.0f);
        }
    }

    /**
     * @brief Tests whether the plane (pNormal, fDist) intersects the AABB of half-extents pSize
     *        centered at the origin (PS2: PlaneBoxOverlap).
     */
    inline bool PlaneBoxOverlap(const float* pNormal, float fDist, const float* pSize)
    {
        float vMin[3];
        float vMax[3];

        for (int i = 0; i < 3; ++i)
        {
            if (pNormal[i] <= 0.0f)
            {
                vMin[i] = pSize[i];
                vMax[i] = -pSize[i];
            }
            else
            {
                vMin[i] = -pSize[i];
                vMax[i] = pSize[i];
            }
        }

        return (vdot(pNormal, vMin) + fDist) <= 0.0f && (vdot(pNormal, vMax) + fDist) >= 0.0f;
    }

    /**
     * @brief Separating-axis test between an origin-centered AABB (half-extents pSize)
     *        and a triangle already expressed in box space (PS2/XBOX: TriangleAABBOverlap core).
     */
    inline bool TriangleAABBOverlap(const float* pSize, const float* pVert0, const float* pVert1, const float* pVert2)
    {
        float vEdge0[3];
        float vEdge1[3];
        float vEdge2[3];

        vsub(vEdge0, pVert1, pVert0);
        vsub(vEdge1, pVert2, pVert1);
        vsub(vEdge2, pVert0, pVert2);

        // 9 cross-product axes (box axes x triangle edges)
        const float* pEdges[3] = { vEdge0, vEdge1, vEdge2 };
        const float* pOtherVerts[3][2] =
        {
            { pVert0, pVert2 },
            { pVert0, pVert1 },
            { pVert1, pVert2 },
        };

        for (int iEdge = 0; iEdge < 3; ++iEdge)
        {
            const float* pEdge = pEdges[iEdge];
            const float fAbsX = std::fabs(pEdge[0]);
            const float fAbsY = std::fabs(pEdge[1]);
            const float fAbsZ = std::fabs(pEdge[2]);
            const float* pA = pOtherVerts[iEdge][0];
            const float* pB = pOtherVerts[iEdge][1];

            // axis (1, 0, 0) x edge
            {
                const float fP0 = pEdge[2] * pA[1] - pEdge[1] * pA[2];
                const float fP1 = pEdge[2] * pB[1] - pEdge[1] * pB[2];
                const float fMin = (std::min)(fP0, fP1);
                const float fMax = (std::max)(fP0, fP1);
                const float fRad = fAbsZ * pSize[1] + fAbsY * pSize[2];

                if (fMin > fRad || fMax < -fRad)
                {
                    return false;
                }
            }

            // axis (0, 1, 0) x edge
            {
                const float fP0 = -pEdge[2] * pA[0] + pEdge[0] * pA[2];
                const float fP1 = -pEdge[2] * pB[0] + pEdge[0] * pB[2];
                const float fMin = (std::min)(fP0, fP1);
                const float fMax = (std::max)(fP0, fP1);
                const float fRad = fAbsZ * pSize[0] + fAbsX * pSize[2];

                if (fMin > fRad || fMax < -fRad)
                {
                    return false;
                }
            }

            // axis (0, 0, 1) x edge
            {
                const float fP0 = pEdge[1] * pA[0] - pEdge[0] * pA[1];
                const float fP1 = pEdge[1] * pB[0] - pEdge[0] * pB[1];
                const float fMin = (std::min)(fP0, fP1);
                const float fMax = (std::max)(fP0, fP1);
                const float fRad = fAbsY * pSize[0] + fAbsX * pSize[1];

                if (fMin > fRad || fMax < -fRad)
                {
                    return false;
                }
            }
        }

        // 3 box face axes
        for (int iAxis = 0; iAxis < 3; ++iAxis)
        {
            const float fMin = (std::min)((std::min)(pVert0[iAxis], pVert1[iAxis]), pVert2[iAxis]);
            const float fMax = (std::max)((std::max)(pVert0[iAxis], pVert1[iAxis]), pVert2[iAxis]);

            if (fMin > pSize[iAxis] || fMax < -pSize[iAxis])
            {
                return false;
            }
        }

        // triangle plane axis
        float vNormal[3];
        vcross(vNormal, vEdge0, vEdge1);

        return PlaneBoxOverlap(vNormal, -vdot(vNormal, pVert0), pSize);
    }

    /**
     * @brief OBB wrapper: transforms the triangle into box space (centered at pCenter,
     *        rotated by pMat) and runs the AABB test (PS2/XBOX: TriangleAABBOverlap with mat).
     */
    inline bool TriangleAABBOverlap(const float* pCenter, const float* pSize, const float* pMat,
        const float* pVert0, const float* pVert1, const float* pVert2)
    {
        float v0[3];
        float v1[3];
        float v2[3];

        vsub(v0, pVert0, pCenter);
        vsub(v1, pVert1, pCenter);
        vsub(v2, pVert2, pCenter);
        vmtmul(v0, pMat);
        vmtmul(v1, pMat);
        vmtmul(v2, pMat);

        return TriangleAABBOverlap(pSize, v0, v1, v2);
    }

    inline void vneg(float* v)
    {
        v[0] = -v[0];
        v[1] = -v[1];
        v[2] = -v[2];
    }

    inline void vmuls(float* vOut, const float* pV1, const float fScalar)
    {
        vOut[0] = fScalar * pV1[0];
        vOut[1] = fScalar * pV1[1];
        vOut[2] = fScalar * pV1[2];
    }

    /**
     * @brief Angular pull: rotates the unit vector pFrom toward the unit vector pTo
     *        by at most fMaxAngle radians (PC: vangpul, 0x436EB0).
     *        The result length is lerped between |pFrom| and |pTo|.
     * @return true when the angular distance fits in fMaxAngle (result == pTo), false otherwise.
     */
    inline bool vangpul(float* pOut, const float* pFrom, const float* pTo, float fMaxAngle)
    {
        float vFrom[3];
        float vTo[3];

        const float fFromLen = vnorm(vFrom, pFrom);
        const float fToLen = vnorm(vTo, pTo);

        float fDot = vTo[0] * vFrom[0] + vTo[1] * vFrom[1] + vTo[2] * vFrom[2];
        float fAngle;

        if (fDot > 1.0f)
        {
            fAngle = 0.0f;
        }
        else if (fDot < -1.0f)
        {
            fAngle = 3.1415927f;
        }
        else
        {
            fAngle = std::acos(fDot);
        }

        if (fAngle <= fMaxAngle)
        {
            pOut[0] = pTo[0];
            pOut[1] = pTo[1];
            pOut[2] = pTo[2];
            return true;
        }

        // Perpendicular component: pPerp = pTo_normalized - vFrom * dot
        float vPerp[3] =
        {
            vTo[0] - vFrom[0] * fDot,
            vTo[1] - vFrom[1] * fDot,
            vTo[2] - vFrom[2] * fDot
        };

        if (vnorm(vPerp) < 0.00012207031f)
        {
            // Fallback axis perpendicular to pTo
            float vAxis[3] = { 0.0f, 1.0f, 0.0f };
            vcross(vPerp, vAxis, pTo);
            vnorm(vPerp);
        }

        const float fSin = -std::sin(fMaxAngle);
        const float fCos = std::cos(fMaxAngle);

        pOut[0] = vFrom[0] * fCos + vPerp[0] * fSin;
        pOut[1] = vFrom[1] * fCos + vPerp[1] * fSin;
        pOut[2] = vFrom[2] * fCos + vPerp[2] * fSin;

        float fScale = 0.0f;
        if (fAngle > 0.00012207031f)
        {
            fScale = fMaxAngle / fAngle;
        }

        const float fLen = fScale * (fToLen - fFromLen) + fFromLen;
        pOut[0] *= fLen;
        pOut[1] *= fLen;
        pOut[2] *= fLen;

        return false;
    }

    /**
     * @brief Builds a full 3x3 rotation matrix from a direction vector and an up vector
     *        (PC: createmat, 0x437550).
     *        The first row (Z-axis in Glacier storage) is the normalized direction.
     *        When pvUp is null, a suitable fallback axis is chosen.
     */
    inline void createmat(float* pMat, const float* pvDir, const float* pvUp)
    {
        // Z-axis (forward): normalize direction, fallback to (0, 0, 1)
        if (!pvDir || vnorm(&pMat[0], pvDir) < 0.00012207031f)
        {
            pMat[0] = 0.0f;
            pMat[1] = 0.0f;
            pMat[2] = 1.0f;
        }

        float vDefaultUp[3] = { 0.0f, 1.0f, 0.0f };
        const float* pUp = pvUp;

        if (pUp)
        {
            // When the up vector is nearly parallel to the direction, fall back to Y axis
            if (std::fabs(vdot(pUp, &pMat[0])) > 0.99987793f)
            {
                pMat[0] = 0.0f;
                pMat[1] = 1.0f;
                pMat[2] = 0.0f;
            }
        }
        else
        {
            // No up vector: pick Y axis, or X axis when direction is nearly parallel to Y
            if (std::fabs(pMat[1]) > 0.99987793f)
            {
                vDefaultUp[0] = 1.0f;
                vDefaultUp[1] = 0.0f;
                vDefaultUp[2] = 0.0f;
            }
            pUp = vDefaultUp;
        }

        // X-axis = up x forward, Y-axis = forward x X-axis
        vcross(&pMat[6], pUp, &pMat[0]);
        vnorm(&pMat[6]);
        vcross(&pMat[3], &pMat[0], &pMat[6]);
    }

    /**
     * @brief OBB vs OBB collision test (PC: 0x437640).
     *        Boxes are described by a rotation matrix mMat, a center vPos and half-extents.
     *        Separating axis theorem over 15 axes (6 face + 9 edge cross products).
     */
    inline bool rectBoxColi(const float* mMat1, const float* vPos1, const float* a, const float* mMat2, const float* vPos2, const float* b)
    {
        // Box 2 axes expressed in box 1 local space.
        float mRel[9];
        vmtmul(&mRel[6], &mMat2[6], mMat1); // X axis
        vmtmul(&mRel[3], &mMat2[3], mMat1); // Y axis
        vmtmul(&mRel[0], &mMat2[0], mMat1); // Z axis

        // Box 2 center relative to box 1 center, in box 1 local space.
        float vT[3];
        vsub(vT, vPos2, vPos1);
        vmtmul(vT, mMat1);

        constexpr float kEps = 0.000001f;
        float mAbs[9];
        for (int i = 0; i < 9; ++i)
        {
            mAbs[i] = std::fabs(mRel[i]) + kEps;
        }

        const float R00 = mAbs[0], R01 = mAbs[1], R02 = mAbs[2];
        const float R10 = mAbs[3], R11 = mAbs[4], R12 = mAbs[5];
        const float R20 = mAbs[6], R21 = mAbs[7], R22 = mAbs[8];

        const float tx = vT[0];
        const float ty = vT[1];
        const float tz = vT[2];

        // Box 1 face axes
        if (R20 * b[0] + R10 * b[1] + R00 * b[2] + a[0] < std::fabs(tx))
            return false;

        // Box 2 face axes
        if (R20 * a[0] + R21 * a[1] + R22 * a[2] + b[0] < std::fabs(tx * mRel[6] + ty * mRel[7] + tz * mRel[8]))
            return false;
        if (R21 * b[0] + R11 * b[1] + R01 * b[2] + a[1] < std::fabs(ty))
            return false;
        if (R22 * b[0] + R12 * b[1] + R02 * b[2] + a[2] < std::fabs(tz))
            return false;
        if (R10 * a[0] + R11 * a[1] + R12 * a[2] + b[1] < std::fabs(tx * mRel[3] + ty * mRel[4] + tz * mRel[5]))
            return false;
        if (R00 * a[0] + R01 * a[1] + R02 * a[2] + b[2] < std::fabs(tx * mRel[0] + ty * mRel[1] + tz * mRel[2]))
            return false;

        // Edge cross-product axes
        if (R22 * a[1] + R21 * a[2] + R00 * b[1] + R10 * b[2] < std::fabs(tz * mRel[7] - ty * mRel[8]))
            return false;
        if (R12 * a[1] + R11 * a[2] + R20 * b[2] + R00 * b[0] < std::fabs(tz * mRel[4] - ty * mRel[5]))
            return false;
        if (R02 * a[1] + R01 * a[2] + R20 * b[1] + R10 * b[0] < std::fabs(tz * mRel[1] - ty * mRel[2]))
            return false;
        if (R01 * b[1] + R11 * b[2] + R20 * a[2] + R22 * a[0] < std::fabs(tx * mRel[8] - tz * mRel[6]))
            return false;
        if (R21 * b[2] + R10 * a[2] + R12 * a[0] + R01 * b[0] < std::fabs(tx * mRel[5] - tz * mRel[3]))
            return false;
        if (R21 * b[1] + R00 * a[2] + R02 * a[0] + R11 * b[0] < std::fabs(tx * mRel[2] - tz * mRel[0]))
            return false;
        if (R02 * b[1] + R12 * b[2] + R20 * a[1] + R21 * a[0] < std::fabs(ty * mRel[6] - tx * mRel[7]))
            return false;
        if (R22 * b[2] + R10 * a[1] + R11 * a[0] + R02 * b[0] < std::fabs(ty * mRel[3] - tx * mRel[4]))
            return false;
        if (R22 * b[1] + R00 * a[1] + R01 * a[0] + R12 * b[0] < std::fabs(ty * mRel[0] - tx * mRel[1]))
            return false;

        return true;
    }

    inline void mirrorquat(float* q)
    {
        q[2] = -q[2];
        q[3] = -q[3];
    }

    /**
     * @brief Normalizes a 4D quaternion in-place to ensure unit length.
     *
     * Scaled components are modified directly if the squared magnitude
     * exceeds a safe epsilon threshold (> 1e-8).
     *
     * @param[in,out] q Pointer to a 4-element float array representing the quaternion (x, y, z, w).
     */
    inline void qnorm(float* q)
    {
        float fLenSq = q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3];

        if (fLenSq != 0.0f) // Rly zero? ok...
        {
            float fInvLen = 1.0f / std::sqrt(fLenSq);

            q[0] *= fInvLen;
            q[1] *= fInvLen;
            q[2] *= fInvLen;
            q[3] *= fInvLen;
        }
    }

    /**
     * @brief Quaternion partial unroll / fast slerp approximation.
     *
     * Performs high-performance quaternion interpolation between q0 and q1 by time factor t.
     * Uses iterative half-angle subdivision (up to 3 steps) to narrow large angular
     * distances until the dot product exceeds 0.9, falling back to LERP with a final normalization.
     * Automatically handles the shortest path on the 4D hypersphere.
     *
     * @param[out] qres Output quaternion array (4 floats: x, y, z, w).
     * @param[in]  q0   Starting unit quaternion (4 floats).
     * @param[in]  q1   Ending unit quaternion (4 floats).
     * @param[in]  t    Interpolation factor in range [0.0, 1.0].
     */
    inline void qpul(float* qres, const float* q0, const float* q1, float t)
    {
        static const float SQRT_0_5 = std::sqrt(0.5f);

        ZASSERT(std::sqrt(q0[0]*q0[0] + q0[1]*q0[1] + q0[2]*q0[2] + q0[3]*q0[3]) > 0.95f);
        ZASSERT(std::sqrt(q1[0]*q1[0] + q1[1]*q1[1] + q1[2]*q1[2] + q1[3]*q1[3]) > 0.95f);

        float qA[4] = { q0[0], q0[1], q0[2], q0[3] };
        float qB[4] = { q1[0], q1[1], q1[2], q1[3] };

        // Dot Product / cos(theta)
        float dot = qB[0]*qA[0] + qB[1]*qA[1] + qB[2]*qA[2] + qB[3]*qA[3];

        // Shortest Path Flip
        if (dot < 0.0f)
        {
            qB[0] = -qB[0];
            qB[1] = -qB[1];
            qB[2] = -qB[2];
            qB[3] = -qB[3];
            dot = -dot;
        }

        // Half-angle subdivision
        float localT = t;
        int maxSteps = 3;

        while (dot < 0.9f && maxSteps > 0)
        {
            maxSteps--;

            float cosHalf = dot + 1.0f;
            float invLen = SQRT_0_5 / std::sqrt(cosHalf); // Scale midpoint

            float mid[4] = {
                (qB[0] + qA[0]) * invLen,
                (qB[1] + qA[1]) * invLen,
                (qB[2] + qA[2]) * invLen,
                (qB[3] + qA[3]) * invLen
            };

            if (localT >= 0.5f)
            {
                // Second half [0.5, 1.0]
                for (int i = 0; i < 4; ++i)
                    qA[i] = mid[i];

                localT -= 0.5f;
            }
            else
            {
                // First half  [0.0, 0.5]
                for (int i = 0; i < 4; ++i) qB[i] = mid[i];
            }

            dot = cosHalf * invLen;
            localT *= 2.0f; // scaleup T
        }

        // Final Lerp
        float factorB = localT;
        float factorA = 1.0f - factorB;

        qres[0] = qA[0] * factorA + qB[0] * factorB;
        qres[1] = qA[1] * factorA + qB[1] * factorB;
        qres[2] = qA[2] * factorA + qB[2] * factorB;
        qres[3] = qA[3] * factorA + qB[3] * factorB;

        // Final norm
        qnorm(qres);
        ZASSERT(std::sqrt(qres[0]*qres[0] + qres[1]*qres[1] + qres[2]*qres[2] + qres[3]*qres[3]) > 0.95f);
    }

    inline void MatrixToMatPos(ZMat3x3& m, ZVector3& p, const ZMatrix& mp)
    {
        m = mp.m0;
        p = mp.p0;
    }

    inline void ConvertZMatrixToMat44(ZMat4x4& m, const ZMatrix& mp)
    {
        // TODO: Finish me
    }

    inline void pcpy(float* vres, const float* v0)
    {
        vres[0] = v0[0];
        vres[1] = v0[1];
        vres[2] = v0[2];
        vres[3] = v0[3];
    }

    /**
     * @brief Pulls a value toward a target by at most a fixed step (PC: PullToValue).
     *
     * Steps @c value toward @c target by @c rate and clamps to @c target when
     * overshooting. Returns true once @c target is reached.
     */
    template <typename T>
    inline bool PullToValue(T& value, const T& target, const T& rate)
    {
        if (value < target)
        {
            value += rate;
            if (value > target)
            {
                value = target;
                return true;
            }
        }
        else if (value > target)
        {
            value -= rate;
            if (value < target)
            {
                value = target;
                return true;
            }
        }
        else
        {
            return true;
        }

        return false;
    }

    /**
     * @brief Builds the minimal-rotation quaternion that aligns the +X axis to @c dir
     *        (PC: minTransformQuat, PS2: minTransformQuat).
     *
     * @param[out] q   Result quaternion (x, y, z, w).
     * @param[in]  dir Unit direction vector (x, y, z).
     */
    inline void minTransformQuat(float* q, const float* dir)
    {
        const float s = std::sqrt(dir[0] + 1.0f);
        const float f = 0.70710677f / s;

        q[0] = 0.0f;
        q[1] = -dir[2] * f;
        q[2] = dir[1] * f;
        q[3] = s * 0.70710677f;
    }

    inline float GetAngle(float x, float y)
    {
        if (x == 0.0f)
        {
            return (y <= 0.0f) ? std::numbers::pi_v<float> * 1.5f : std::numbers::pi_v<float> * 0.5f;
        }

        float angle = atanf(y / x);
        if (x < 0.0f)
        {
            return angle + std::numbers::pi_v<float>;
        }
        else if (y < 0.0f)
        {
            return angle + (std::numbers::pi_v<float> * 2.0f);
        }

        return angle;
    }

    template <typename T>
    T clamp(T value, T min, T max)
    {
        if (value < min)
        {
            value = min;
        }

        if (value > max)
        {
            value = max;
        }

        return value;
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
vcross(float *,float const *,float const *)	.text	820EA4F8	00000058	00000000	00000001	R	.	.	.	.	.	.	T	.	.
vcpy<float,float>(float *,float const *)	.text	820D96D8	0000001C	00000000	00000001	R	.	.	.	.	.	.	T	.	.
vangularpul(float * const,float const * const,float const * const,float,float)	.text	820ED7F8	00000314	000000F0		R	.	.	.	.	.	.	T	.	.
vangpul(float * const,float const * const,float const * const,float)	.text	820EB590	00000230	000000E0		R	.	.	.	.	.	.	T	.	.
vangle(float const * const,float const * const)	.text	820EAC00	00000058	00000000	00000001	R	.	.	.	.	.	.	T	.	.
vaddscalar(float *,float const *,float)	.text	820EA488	00000034	00000000	00000001	R	.	.	.	.	.	.	T	.	.
vadd(float *,float const *)	.text	820D9668	00000034	00000000	00000001	R	.	.	.	.	.	.	T	.	.
vabs(float *,float const *)	.text	820EAD18	00000028	00000000	00000001	R	.	.	.	.	.	.	T	.	.
vabs(float *)	.text	820EACF0	00000028	00000000	00000001	R	.	.	.	.	.	.	T	.	.
toupper	.text	8241DB40	00000018	00000000	00000001	R	.	.	.	.	.	.	T	.	.
tolower	.text	8241ED38	00000018	00000000	00000001	R	.	.	.	.	.	.	T	.	.
tanh	.text	8265AD20	00000100	00000070		R	.	.	.	.	.	.	T	.	.
tan	.text	8241CB30	000000D4	00000010		R	.	.	.	.	.	.	T	.	.
*/
