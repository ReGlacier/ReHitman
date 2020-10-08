#pragma once

namespace Glacier
{
    struct Vector2
    {
        float x, y;
    };

    struct Vector3
    {
        float x, y, z;
    };

    struct Vector4
    {
        float x, y, z, w;
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