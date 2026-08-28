#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Geom/ZCAMERA.h>

namespace Glacier {
    class ZCameraSpace
    {
    public:
        // data
        ZMatrix m_CameraToRoot;
        float m_ClipPlanes[48];
        float m_fFogNear;
        float m_fFogFar;
        float m_fFogMin;
        float m_fFogMax;
        unsigned int m_lFogColor;
        float m_fNear;
        float m_fFar;
        float m_fFieldOfView;
        float m_fFieldOfViewFirstPerson;
        float m_fPers;
        float m_fScaleX;
        float m_fScaleY;
        float m_fViewport[4];
        float m_fViewAspect;
        float m_vCameraStartPoint[3];
        unsigned int m_lNrClipPlanes;
        unsigned int m_rTopNode;
        bool m_bIsOrthogonal;
        bool m_bIsMain;
        bool m_bGatesEnabled;
        bool m_bGeomBoundsEnabled;
        const ZCAMERA *m_pZCamera;

        // operators
        ZCameraSpace& operator=(ZCAMERA* pCamera);

        // api
        bool IsMirror();
        bool IsMain();
        void Proj2D(Vector2* pResult, const Vector3* pPoint);
        void Proj3D(Vector3* pResult, const Vector3* pPoint);
        void GetLocalMatPos(Matrix3x3* mat, Vector3* pos);
        void TransformInversMatPos(Matrix3x3* mat, Vector3* pos);
        void GetViewport(Vector4* pViewport);
    };
    RE_VERIFY_SIZE(ZCameraSpace, 0x150);
}
