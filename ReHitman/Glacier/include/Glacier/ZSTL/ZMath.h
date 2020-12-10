#pragma once

#include <Glacier/ZSTL/ZMemory.h>

namespace Glacier
{
    struct Vector2 : public UseManagedAllocator
    {
        float x = 0.f, y = 0.f;
    };

    struct Vector3 : public UseManagedAllocator
    {
        float x = 0.f, y = 0.f, z = 0.f;
    };

    struct Vector4 : public UseManagedAllocator
    {
        float x = 0.f, y = 0.f, z = 0.f, w = 0.f;
    };

    struct ZRGB : public UseManagedAllocator
    {
        float r = 0.f, g = 0.f, b = 0.f;
    };

    struct ZRGBA : public UseManagedAllocator
    {
        float r = 0.f, g = 0.f, b = 0.f, a = 0.f;
    };

    struct Matrix3x3 : public UseManagedAllocator
    {
        float data[9];
    };

    struct Matrix4x4 : public UseManagedAllocator
    {
        float data[16];
    };

    using ZVector2 = Vector2;
    using ZVector3 = Vector3;
    using ZVector4 = Vector4;
    using ZMat4x4 = Matrix4x4;
    using ZMat3x3 = Matrix3x3;
}