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
        uint32_t m_lFogColor;
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
        uint32_t m_lNrClipPlanes;
        ZREF m_rTopNode;
        bool m_bIsOrthogonal;
        bool m_bIsMain;
        bool m_bGatesEnabled;
        bool m_bGeomBoundsEnabled;
        // operators
        ZCameraSpace& operator=(const ZCAMERA* pCamera);

        // api
        uint32_t CalcRootClipPlanes(float* pPlanes, uint32_t lMaxPlanes) const;
        float CalcFieldOfViewX() const { return m_fFieldOfView; }
        float CalcFieldOfViewXFirstPerson() const { return m_fFieldOfViewFirstPerson; }
        uint32_t GetNrClipPlanes() const { return m_lNrClipPlanes; }
        const float* GetClipPlanes() const { return m_ClipPlanes; }
        float GetNear() const { return m_fNear; }
        float GetFar() const { return m_fFar; }
        float GetPers() const { return m_fPers; }
        float GetScaleX() const { return m_fScaleX; }
        float GetScaleY() const { return m_fScaleY; }
        float GetViewAspect() const { return m_fViewAspect; }
        const float* GetCameraStartPoint() const { return m_vCameraStartPoint; }
        void SetCameraStartPoint(const float* pPoint);
        bool IsMirror() const;
        bool IsOrthogonal() const { return m_bIsOrthogonal; }
        bool IsMain() const;
        void Proj2D(Vector2* pResult, const Vector3* pPoint) const;
        void Proj3D(Vector3* pResult, const Vector2* pPoint) const;
        void GetLocalMatPos(Matrix3x3* mat, Vector3* pos);
        void TransformInversMatPos(Matrix3x3* mat, Vector3* pos) const;
        void GetViewport(Vector4* pViewport) const;
        float GetFogNear() const { return m_fFogNear; }
        float GetFogFar() const { return m_fFogFar; }
        uint32_t GetFogColor() const { return m_lFogColor; }
        void SetFogNear(float fFogNear) { m_fFogNear = fFogNear; }
        void SetFogFar(float fFogFar) { m_fFogFar = fFogFar; }
        void SetFogColor(uint32_t lFogColor) { m_lFogColor = lFogColor; }
        void SetGatesEnabled(bool bEnabled) { m_bGatesEnabled = bEnabled; }
        bool GetGatesEnabled() const { return m_bGatesEnabled; }
        bool GetGeomBoundsEnabled() const { return m_bGeomBoundsEnabled; }
        ZREF GetTopNode() const { return m_rTopNode; }
    };
    RE_VERIFY_SIZE(ZCameraSpace, 0x144);
    RE_VERIFY_OFFSET(ZCameraSpace, m_fFogNear, 0xF0);
    RE_VERIFY_OFFSET(ZCameraSpace, m_fPers, 0x10C);
    RE_VERIFY_OFFSET(ZCameraSpace, m_fViewport, 0x118);
    RE_VERIFY_OFFSET(ZCameraSpace, m_lNrClipPlanes, 0x138);
}
