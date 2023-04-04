#include <Glacier/ZCameraSpace.h>
#include <G1ConfigurationService.h>
#include <cassert>

namespace Glacier {
    ZCameraSpace& ZCameraSpace::operator=(ZCAMERA* pCamera) {
        if (!pCamera) {
            return *this;
        }

        assert(G1ConfigurationService::G1API_FunctionAddress_ZCameraSpace_AssignOperator != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZCameraSpace_AssignOperator != G1ConfigurationService::kNotConfiguredOption)
        {
            ((void(__thiscall*)(ZCameraSpace*,ZCAMERA*))(G1ConfigurationService::G1API_FunctionAddress_ZCameraSpace_AssignOperator))(this, pCamera);
        }
        return *this;
    }

    bool ZCameraSpace::IsMirror() {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZCameraSpace_IsMirror  != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZCameraSpace_IsMirror != G1ConfigurationService::kNotConfiguredOption)
        {
            return ((bool(__thiscall*)(ZCameraSpace*))(G1ConfigurationService::G1API_FunctionAddress_ZCameraSpace_IsMirror))(this);
        }
        return false;
    }

    bool ZCameraSpace::IsMain() {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZCameraSpace_IsMain  != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZCameraSpace_IsMain != G1ConfigurationService::kNotConfiguredOption)
        {
            return ((bool(__thiscall*)(ZCameraSpace*))(G1ConfigurationService::G1API_FunctionAddress_ZCameraSpace_IsMain))(this);
        }
        return false;
    }

    void ZCameraSpace::Proj3D(Vector3* pResult, const Vector3* pPoint) {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZCameraSpace_Proj3D != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZCameraSpace_Proj3D != G1ConfigurationService::kNotConfiguredOption)
        {
            ((void(__thiscall*)(ZCameraSpace*,Vector3*,const Vector3*))(G1ConfigurationService::G1API_FunctionAddress_ZCameraSpace_Proj3D))(this, pResult, pPoint);
        }
    }

    void ZCameraSpace::Proj2D(Vector2* pResult, const Vector3* pPoint) {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZCameraSpace_Proj2D != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZCameraSpace_Proj2D != G1ConfigurationService::kNotConfiguredOption)
        {
            ((void(__thiscall*)(ZCameraSpace*,Vector2*,const Vector3*))(G1ConfigurationService::G1API_FunctionAddress_ZCameraSpace_Proj2D))(this, pResult, pPoint);
        }
    }

    void ZCameraSpace::GetLocalMatPos(Matrix3x3* mat, Vector3* pos) {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZCameraSpace_GetLocalMatPos != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZCameraSpace_GetLocalMatPos != G1ConfigurationService::kNotConfiguredOption)
        {
            ((void(__thiscall*)(ZCameraSpace*,Matrix3x3*,Vector3*))(G1ConfigurationService::G1API_FunctionAddress_ZCameraSpace_GetLocalMatPos))(this, mat, pos);
        }
    }

    void ZCameraSpace::TransformInversMatPos(Matrix3x3* mat, Vector3* pos) {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZCameraSpace_TransformInversMatPos != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZCameraSpace_TransformInversMatPos != G1ConfigurationService::kNotConfiguredOption)
        {
            ((void(__thiscall*)(ZCameraSpace*,Matrix3x3*,Vector3*))(G1ConfigurationService::G1API_FunctionAddress_ZCameraSpace_TransformInversMatPos))(this, mat, pos);
        }
    }

    void ZCameraSpace::GetViewport(Vector4* pViewport) {
        assert(pViewport != nullptr);
        assert(G1ConfigurationService::G1API_FunctionAddress_ZCameraSpace_GetViewport != G1ConfigurationService::kNotConfiguredOption);

        if (pViewport && G1ConfigurationService::G1API_FunctionAddress_ZCameraSpace_GetViewport != G1ConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(ZCameraSpace*,Vector4*))G1ConfigurationService::G1API_FunctionAddress_ZCameraSpace_GetViewport)(this, pViewport);
        }
    }
}