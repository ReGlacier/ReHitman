#pragma once

namespace Glacier
{
    struct Vector2
    {
        float x = 0.f, y = 0.f;
    };

    struct Vector3
    {
        float x = 0.f, y = 0.f, z = 0.f;
    };

    struct Vector4
    {
        float x = 0.f, y = 0.f, z = 0.f, w = 0.f;
    };

    struct Matrix3x3
    {
        float data[9];
    };

    struct Matrix4x4
    {
        float data[16];
    };

    using ZVector2 = Vector2;
    using ZVector3 = Vector3;
    using ZVector4 = Vector4;
    using ZMat4x4 = Matrix4x4;
    using ZMat3x3 = Matrix3x3;
    using ZRGBA = Vector4;
    using ZRGB = Vector3;
}