#pragma once

#include <Glacier/ZSTL/ZMemory.h>
#include <algorithm>

namespace Glacier
{
    struct Vector2 : public UseManagedAllocator
    {
        float x = 0.f, y = 0.f;

        bool operator==(const Vector2& with) const {
            return x == with.x && y == with.y;
        }
    };

    struct Vector3 : public UseManagedAllocator
    {
        float x = 0.f, y = 0.f, z = 0.f;

        Vector3() = default;
        Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

        bool operator==(const Vector3& with) const {
            return x == with.x && y == with.y && z == with.z;
        }
    };

    struct Vector4 : public UseManagedAllocator
    {
        float x = 0.f, y = 0.f, z = 0.f, w = 0.f;

        Vector4() = default;
        Vector4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}

        bool operator==(const Vector4& with) const {
            return x == with.x && y == with.y && z == with.z && w == with.w;
        }
    };

    struct Matrix3x3 : public UseManagedAllocator
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
    };

    struct Matrix4x4 : public UseManagedAllocator
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
}