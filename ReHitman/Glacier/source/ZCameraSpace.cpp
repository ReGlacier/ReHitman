#include <Glacier/ZCameraSpace.h>
#include <algorithm>
#include <cmath>

namespace Glacier {
    ZCameraSpace& ZCameraSpace::operator=(const ZCAMERA* pCamera) {
        pCamera->GetRootTM(m_CameraToRoot.m0, m_CameraToRoot.p0);
        m_vCameraStartPoint[0] = m_CameraToRoot.p0.x;
        m_vCameraStartPoint[1] = m_CameraToRoot.p0.y;
        m_vCameraStartPoint[2] = m_CameraToRoot.p0.z;
        m_fFieldOfView = pCamera->FOV;
        m_fFieldOfViewFirstPerson = pCamera->FOVFirstPerson;
        m_fPers = pCamera->Pers;
        m_fNear = pCamera->Near;
        m_fFar = pCamera->Far;
        m_fScaleX = pCamera->m_vScale.x;
        m_fScaleY = pCamera->m_vScale.y;
        m_lNrClipPlanes = pCamera->m_ClipPlanes.GetClipPlanes(m_ClipPlanes);
        const_cast<ZCAMERA*>(pCamera)->GetViewport(reinterpret_cast<ZVector4*>(m_fViewport));
        m_fViewAspect = const_cast<ZCAMERA*>(pCamera)->GetViewAspect();

        if (pCamera->CameraCon & 0x800000)
        {
            m_fViewport[0] = 0.0f;
            m_fViewport[1] = 0.0f;
            m_fViewport[2] = 1.0f;
            m_fViewport[3] = 1.0f;
            m_fViewAspect = 1.0f;
        }

        m_fFogNear = pCamera->m_fFogNear;
        m_fFogFar = pCamera->m_fFogFar;
        m_lFogColor = pCamera->BackCol;
        if (pCamera->CameraCon & 0x80000)
        {
            m_fFogNear = pCamera->Far;
            m_fFogFar = pCamera->Far * 2.0f;
        }
        else
        {
            m_fFogNear = pCamera->Far * pCamera->m_fFogNear;
            m_fFogFar = pCamera->Far * pCamera->m_fFogFar;
            if (m_fFogFar == 0.0f)
                m_fFogFar = pCamera->Far;
        }

        for (uint32_t i = 0; i < m_lNrClipPlanes; ++i)
        {
            float* pPlane = &m_ClipPlanes[i * 4];
            pPlane[0] = -pPlane[0];
            pPlane[1] = -pPlane[1];
            pPlane[2] = -pPlane[2];
            pPlane[3] = -pPlane[3];
            const float fLength = std::sqrt(pPlane[0] * pPlane[0] + pPlane[1] * pPlane[1] + pPlane[2] * pPlane[2]);
            if (fLength != 0.0f)
            {
                const float fInvLength = 1.0f / fLength;
                pPlane[0] *= fInvLength;
                pPlane[1] *= fInvLength;
                pPlane[2] *= fInvLength;
                pPlane[3] *= fInvLength;
            }
        }

        m_bIsOrthogonal = (pCamera->CameraCon & 0x4000000) != 0;
        m_bIsMain = (pCamera->CameraCon & 1) != 0;
        m_bGatesEnabled = (pCamera->CameraCon & 0x10000) == 0;
        m_bGeomBoundsEnabled = (pCamera->CameraCon & 0x200000) == 0;
        m_rTopNode = pCamera->m_rCameraRoot ? pCamera->m_rCameraRoot->GetRef() : 0;
        return *this;
    }

    uint32_t ZCameraSpace::CalcRootClipPlanes(float* pPlanes, uint32_t lMaxPlanes) const {
        ZMat3x3 mTransposed;
        tmat(mTransposed, m_CameraToRoot.m0);
        ZVector3 vLocalCamera;
        vmmul(vLocalCamera, m_CameraToRoot.p0, mTransposed);
        vLocalCamera.x = -vLocalCamera.x;
        vLocalCamera.y = -vLocalCamera.y;
        vLocalCamera.z = -vLocalCamera.z;

        const uint32_t lCount = (std::min)(m_lNrClipPlanes, lMaxPlanes);
        ZASSERT(m_lNrClipPlanes <= lMaxPlanes);
        for (uint32_t i = 0; i < lCount; ++i)
        {
            const float* pSource = &m_ClipPlanes[i * 4];
            float* pDest = &pPlanes[i * 4];
            vmtmul(pDest, pSource, mTransposed);
            pDest[3] = vdot(vLocalCamera, pSource) + pSource[3];
        }
        return lCount;
    }

    void ZCameraSpace::SetCameraStartPoint(const float* pPoint) {
        vcpy(m_vCameraStartPoint, pPoint);
    }

    bool ZCameraSpace::IsMirror() const {
        ZVector3 vCross;
        vcross(vCross, m_CameraToRoot.m0.XAxis(), m_CameraToRoot.m0.ZAxis());
        return vdot(vCross, m_CameraToRoot.m0.YAxis()) > 0.0f;
    }

    bool ZCameraSpace::IsMain() const {
        return m_bIsMain;
    }

    void ZCameraSpace::Proj3D(Vector3* pResult, const Vector2* pPoint) const {
        const float fFactor = pResult->z / m_fPers;
        pResult->x = pPoint->x * fFactor;
        pResult->y = -(m_fViewAspect / m_fScaleY) * pPoint->y * fFactor;
    }

    void ZCameraSpace::Proj2D(Vector2* pResult, const Vector3* pPoint) const {
        const float fFactor = m_fPers / (std::max)(pPoint->z, 0.001f);
        pResult->x = pPoint->x * fFactor;
        pResult->y = -pPoint->y * m_fScaleY * fFactor;
    }

    void ZCameraSpace::GetLocalMatPos(Matrix3x3* mat, Vector3* pos) {
        vsub(*pos, m_CameraToRoot.p0);
        vmtmul(*pos, m_CameraToRoot.m0);
        mmtmul(*mat, *mat, m_CameraToRoot.m0);
    }

    void ZCameraSpace::TransformInversMatPos(Matrix3x3* mat, Vector3* pos) const {
        ZVector3 vLocalPos;
        vsub(vLocalPos, *pos, m_CameraToRoot.p0);
        vmtmul(vLocalPos, m_CameraToRoot.m0);
        ZMat3x3 mLocalMat;
        mmtmul(mLocalMat, *mat, m_CameraToRoot.m0);
        *mat = mLocalMat;
        *pos = vLocalPos;
    }

    void ZCameraSpace::GetViewport(Vector4* pViewport) const {
        std::memcpy(pViewport, m_fViewport, sizeof(m_fViewport));
    }
}
