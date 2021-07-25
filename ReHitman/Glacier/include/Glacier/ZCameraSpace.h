#pragma once

#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZCAMERA.h>

namespace Glacier {
    class ZCameraSpace {
    public:
        // data
        Matrix3x3 matrix;
        Vector3 position;
        // NOTE: There are not all class members, class looks bigger than 3*3*4 + 4*3 bytes (see operator= impl)

        // operators
        ZCameraSpace& operator=(ZCAMERA* pCamera);

        // api
        bool IsMirror();
        void Proj2D(Vector2* pResult, const Vector3* pPoint);
        void Proj3D(Vector3* pResult, const Vector3* pPoint);
        void GetLocalMatPos(Matrix3x3* mat, Vector3* pos);
        void TransformInversMatPos(Matrix3x3* mat, Vector3* pos);
    };
}