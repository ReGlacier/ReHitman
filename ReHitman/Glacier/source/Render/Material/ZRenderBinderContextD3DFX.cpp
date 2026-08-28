#include <Glacier/Render/Material/ZRenderBinderContextD3DFX.h>
#include <Glacier/Render/Material/ZRenderMaterialEffectD3DFX.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/Render/Object/ZRenderObject.h>
#include <Glacier/Render/Object/ZRenderObjectInstance.h>
#include <Glacier/Render/Prim/EPrimType.h>
#include <Glacier/Render/Prim/SPrimMesh.h>
#include <Glacier/Render/Prim/SPrimMeshWeighted.h>
#include <Glacier/Render/Prim/SPrimLightOmni.h>
#include <Glacier/Render/Prim/SPrimLightSpot.h>
#include <Glacier/Render/Prim/ZPrimHandle.h>
#include <Glacier/Render/ZDirect3DDevice.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/ZRenderContext.h>
#include <Glacier/Render/ZSharedResourcesD3D.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniAssert.h>
#include <cstring>


namespace Glacier
{
    namespace
    {
        // PC 0x489740
        // Initialise Mat4x4 for DirectX renderer with Mat3x3 object transform and Vec3F object position vector
        void Transform3x3To4x4Matrix(D3DXMATRIX& pDstMtx, const ZMat3x3& pSrcMtx, const ZVector3& pVPosition)
        {
            pDstMtx._11 = pSrcMtx.data[6];
            pDstMtx._12 = pSrcMtx.data[7];
            pDstMtx._13 = pSrcMtx.data[8];
            pDstMtx._14 = 0.0f;
            pDstMtx._21 = pSrcMtx.data[3];
            pDstMtx._22 = pSrcMtx.data[4];
            pDstMtx._23 = pSrcMtx.data[5];
            pDstMtx._24 = 0.0f;
            pDstMtx._31 = pSrcMtx.data[0];
            pDstMtx._32 = pSrcMtx.data[1];
            pDstMtx._33 = pSrcMtx.data[2];
            pDstMtx._34 = 0.0f;
            pDstMtx._41 = pVPosition.x;
            pDstMtx._42 = pVPosition.y;
            pDstMtx._43 = pVPosition.z;
            pDstMtx._44 = 1.0f;
        }

        // PC: UnpackD3DCOLORToFloats (D3DCOLOR 0xAABBGGRR -> RGBA floats)
        void UnpackD3DCOLORToFloats(float& r, float& g, float& b, float& a, uint32_t lColor)
        {
            a = static_cast<float>((lColor >> 24) & 0xFF) * (1.0f / 255.0f);
            r = static_cast<float>((lColor >> 16) & 0xFF) * (1.0f / 255.0f);
            g = static_cast<float>((lColor >> 8) & 0xFF) * (1.0f / 255.0f);
            b = static_cast<float>(lColor & 0xFF) * (1.0f / 255.0f);
        }

        // ---- Unreversed globals used as Execute caches (bind to real addresses later) ----
        // TODO: Finish this place after the render globals will be reversed
        static uint32_t s_lDeformBoneCache = 0;       // dword_90D540
        static uint32_t s_lBoneLightCache = 0;        // dword_90D05C
        static uint32_t s_lBonesLight2Cache = 0;      // dword_90D058
        static bool     s_bBlurDropShadowActive = false; // dword_90DE5C
        static bool     s_bDisableMPS = false;        // dword_90D04C
        static float    s_aDeformBoneMatrices[312] = {}; // unk_90D060
    }

    ZRenderBinderContextD3DFX::ZRenderBinderContextD3DFX(const char* pszName, uint32_t lContextType, ZRenderMaterialEffectD3DFX* pEffect, D3DXHANDLE hParamter)
    {
        m_pszName = pszName;
        m_lBinderType = 3;
        m_lContextType = lContextType;
        m_pEffect = pEffect;
        m_hParamter = hParamter;
    }

    ZRenderBinderContextD3DFX::~ZRenderBinderContextD3DFX() = default;

    void ZRenderBinderContextD3DFX::Execute(const ZRenderContext* pContext)
    {
        const SPrimLight* pLight = pContext->m_pCurrentLight;
        const float fShaderResolution = static_cast<float>(ZSharedResourcesD3D::g_pInstance->m_iShaderResolution);

        switch (m_lContextType)
        {
        case 1:
        {
            // Object-to-world * view * scale * clip matrix
            D3DXMATRIX mObjToWorld, mWorldToView, mScaling, mTemp, mResult;
            Transform3x3To4x4Matrix(mObjToWorld, pContext->m_ObjectToWorldMatrix.m0, pContext->m_ObjectToWorldMatrix.p0);
            Transform3x3To4x4Matrix(mWorldToView, pContext->m_WorldToViewMatrix.m0, pContext->m_WorldToViewMatrix.p0);

            const float fScale = pContext->m_fZBias;
            D3DXMatrixScaling(&mScaling, fScale, fScale, fScale);

            D3DXMatrixMultiply(&mTemp, &mObjToWorld, &mWorldToView);
            D3DXMatrixMultiply(&mResult, &mTemp, &mScaling);
            D3DXMatrixMultiply(&mTemp, &mResult, reinterpret_cast<const D3DXMATRIX*>(&pContext->m_ProjectionMatrix));

            SetFloatArray(reinterpret_cast<const float*>(&mTemp), 16);
            break;
        }

        case 2:
        {
            // View * clip matrix
            D3DXMATRIX mView, mTemp, mResult;
            Transform3x3To4x4Matrix(mView, pContext->m_WorldToViewMatrix.m0, pContext->m_WorldToViewMatrix.p0);
            D3DXMatrixMultiply(&mTemp, &mView, reinterpret_cast<const D3DXMATRIX*>(&pContext->m_ProjectionMatrix));
            D3DXMatrixTranspose(&mResult, &mTemp);
            SetFloatArray(reinterpret_cast<const float*>(&mResult), 16);
            break;
        }

        case 3:
        {
            // View matrix
            D3DXMATRIX mView, mResult;
            Transform3x3To4x4Matrix(mView, pContext->m_WorldToViewMatrix.m0, pContext->m_WorldToViewMatrix.p0);
            D3DXMatrixTranspose(&mResult, &mView);
            SetFloatArray(reinterpret_cast<const float*>(&mResult), 16);
            break;
        }

        case 4:
        {
            // Object-to-world matrix
            D3DXMATRIX mObjToWorld, mResult;
            Transform3x3To4x4Matrix(mObjToWorld, pContext->m_ObjectToWorldMatrix.m0, pContext->m_ObjectToWorldMatrix.p0);
            D3DXMatrixTranspose(&mResult, &mObjToWorld);
            SetFloatArray(reinterpret_cast<const float*>(&mResult), 16);
            break;
        }

        case 5:
        {
            // Object-to-world * view
            D3DXMATRIX mObjToWorld, mWorldToView, mTemp, mResult;
            Transform3x3To4x4Matrix(mObjToWorld, pContext->m_ObjectToWorldMatrix.m0, pContext->m_ObjectToWorldMatrix.p0);
            Transform3x3To4x4Matrix(mWorldToView, pContext->m_WorldToViewMatrix.m0, pContext->m_WorldToViewMatrix.p0);
            D3DXMatrixMultiply(&mTemp, &mObjToWorld, &mWorldToView);
            D3DXMatrixTranspose(&mResult, &mTemp);
            SetFloatArray(reinterpret_cast<const float*>(&mResult), 16);
            break;
        }

        case 6:
        {
            // Prebuilt matrix (projection)
            SetFloatArray(reinterpret_cast<const float*>(&pContext->m_ProjectionMatrix), 16);
            break;
        }

        case 7:
        {
            // Object-to-world * view * clip * screen (viewport) matrix
            D3DXMATRIX mObjToWorld, mWorldToView, mProj, mScreen, mViewportRatio, mTemp, mResult;
            Transform3x3To4x4Matrix(mObjToWorld, pContext->m_ObjectToWorldMatrix.m0, pContext->m_ObjectToWorldMatrix.p0);
            Transform3x3To4x4Matrix(mWorldToView, pContext->m_WorldToViewMatrix.m0, pContext->m_WorldToViewMatrix.p0);

            memcpy(&mProj, &pContext->m_ProjectionMatrix, sizeof(mProj));

            // NDC -> texture space transform
            ZeroMemory(&mScreen, sizeof(mScreen));
            mScreen._11 = 0.5f;
            mScreen._22 = -0.5f;
            mScreen._41 = 0.5f;
            mScreen._42 = 0.5f;
            mScreen._43 = 1.0f;
            mScreen._44 = 1.0f;

            // Viewport ratio matrix
            ZeroMemory(&mViewportRatio, sizeof(mViewportRatio));
            mViewportRatio._11 = 1.0f;
            mViewportRatio._22 = 1.0f;
            mViewportRatio._33 = 1.0f;
            mViewportRatio._44 = 1.0f;

            D3DVIEWPORT9 viewport;
            g_pd3dDevice->GetViewport(&viewport);

            IDirect3DSurface9* pSurface = nullptr;
            g_pd3dDevice->GetRenderTarget(0, &pSurface);

            D3DSURFACE_DESC surfaceDesc;
            if (pSurface)
            {
                pSurface->GetDesc(&surfaceDesc);
                pSurface->Release();
            }

            mViewportRatio._11 = static_cast<float>(viewport.Width) / static_cast<float>(surfaceDesc.Width);
            mViewportRatio._22 = static_cast<float>(viewport.Height) / static_cast<float>(surfaceDesc.Height);
            mViewportRatio._41 = static_cast<float>(viewport.X) / static_cast<float>(surfaceDesc.Width);
            mViewportRatio._42 = static_cast<float>(viewport.Y) / static_cast<float>(surfaceDesc.Height);

            D3DXMatrixMultiply(&mTemp, &mObjToWorld, &mWorldToView);
            D3DXMatrixMultiply(&mResult, &mTemp, &mProj);
            D3DXMatrixMultiply(&mTemp, &mResult, &mScreen);
            D3DXMatrixMultiply(&mResult, &mTemp, &mViewportRatio);
            D3DXMatrixTranspose(&mTemp, &mResult);
            SetFloatArray(reinterpret_cast<const float*>(&mTemp), 16);
            break;
        }

        case 8:
        {
            // Object-to-world * object-to-view
            D3DXMATRIX mObjToWorld, mOther, mTemp, mResult;
            Transform3x3To4x4Matrix(mObjToWorld, pContext->m_ObjectToWorldMatrix.m0, pContext->m_ObjectToWorldMatrix.p0);
            Transform3x3To4x4Matrix(mOther, pContext->m_ObjectToViewMatrix.m0, pContext->m_ObjectToViewMatrix.p0);
            D3DXMatrixMultiply(&mTemp, &mObjToWorld, &mOther);
            D3DXMatrixTranspose(&mResult, &mTemp);
            SetFloatArray(reinterpret_cast<const float*>(&mResult), 16);
            break;
        }

        case 9:
        {
            // Object-to-world * light view (light 0)
            D3DXMATRIX mObjToWorld, mOther, mTemp, mResult;
            Transform3x3To4x4Matrix(mObjToWorld, pContext->m_ObjectToWorldMatrix.m0, pContext->m_ObjectToWorldMatrix.p0);
            Transform3x3To4x4Matrix(mOther, pContext->m_WorldToLightMatrix[0].m0, pContext->m_WorldToLightMatrix[0].p0);
            D3DXMatrixMultiply(&mTemp, &mObjToWorld, &mOther);
            D3DXMatrixTranspose(&mResult, &mTemp);
            SetFloatArray(reinterpret_cast<const float*>(&mResult), 16);
            break;
        }

        case 10:
        {
            // Object-to-world * object-to-view * light clip/projection
            D3DXMATRIX mObjToWorld, mObjToView, mTemp, mResult;
            Transform3x3To4x4Matrix(mObjToWorld, pContext->m_ObjectToWorldMatrix.m0, pContext->m_ObjectToWorldMatrix.p0);
            Transform3x3To4x4Matrix(mObjToView, pContext->m_ObjectToViewMatrix.m0, pContext->m_ObjectToViewMatrix.p0);
            D3DXMatrixMultiply(&mTemp, &mObjToWorld, &mObjToView);
            D3DXMatrixMultiply(&mResult, &mTemp, reinterpret_cast<const D3DXMATRIX*>(&pContext->m_aLightClipMatrix[0]));
            D3DXMatrixTranspose(&mTemp, &mResult);
            SetFloatArray(reinterpret_cast<const float*>(&mTemp), 16);
            break;
        }

        case 11:
        {
            // Object-to-world * light view * light clip/projection
            D3DXMATRIX mObjToWorld, mLightView, mTemp, mResult;
            Transform3x3To4x4Matrix(mObjToWorld, pContext->m_ObjectToWorldMatrix.m0, pContext->m_ObjectToWorldMatrix.p0);
            Transform3x3To4x4Matrix(mLightView, pContext->m_WorldToLightMatrix[1].m0, pContext->m_WorldToLightMatrix[1].p0);
            D3DXMatrixMultiply(&mTemp, &mObjToWorld, &mLightView);
            D3DXMatrixMultiply(&mResult, &mTemp, reinterpret_cast<const D3DXMATRIX*>(&pContext->m_aLightClipMatrix[2]));
            D3DXMatrixTranspose(&mTemp, &mResult);
            SetFloatArray(reinterpret_cast<const float*>(&mTemp), 16);
            break;
        }

        case 12:
        {
            // Object-to-world * bone view (light 2)
            D3DXMATRIX mObjToWorld, mOther, mTemp, mResult;
            Transform3x3To4x4Matrix(mObjToWorld, pContext->m_ObjectToWorldMatrix.m0, pContext->m_ObjectToWorldMatrix.p0);
            Transform3x3To4x4Matrix(mOther, pContext->m_WorldToLightMatrix[2].m0, pContext->m_WorldToLightMatrix[2].p0);
            D3DXMatrixMultiply(&mTemp, &mObjToWorld, &mOther);
            D3DXMatrixTranspose(&mResult, &mTemp);
            SetFloatArray(reinterpret_cast<const float*>(&mResult), 16);
            break;
        }

        case 13:
        {
            // Object-to-world * light view (light 1)
            D3DXMATRIX mObjToWorld, mOther, mTemp, mResult;
            Transform3x3To4x4Matrix(mObjToWorld, pContext->m_ObjectToWorldMatrix.m0, pContext->m_ObjectToWorldMatrix.p0);
            Transform3x3To4x4Matrix(mOther, pContext->m_WorldToLightMatrix[1].m0, pContext->m_WorldToLightMatrix[1].p0);
            D3DXMatrixMultiply(&mTemp, &mObjToWorld, &mOther);
            D3DXMatrixTranspose(&mResult, &mTemp);
            SetFloatArray(reinterpret_cast<const float*>(&mResult), 16);
            break;
        }

        case 14:
        {
            // Light clip/projection matrix (or identity when light type == 1)
            D3DXMATRIX mTemp, mResult;
            if (pLight->lLightType == 1)
            {
                D3DXMatrixIdentity(&mResult);
            }
            else
            {
                D3DXMATRIX mObjToWorld, mObjToView, mProj, mScreen;
                Transform3x3To4x4Matrix(mObjToWorld, pContext->m_ObjectToWorldMatrix.m0, pContext->m_ObjectToWorldMatrix.p0);
                Transform3x3To4x4Matrix(mObjToView, pContext->m_ObjectToViewMatrix.m0, pContext->m_ObjectToViewMatrix.p0);

                memcpy(&mProj, &pContext->m_aLightClipMatrix[0], sizeof(mProj));

                ZeroMemory(&mScreen, sizeof(mScreen));
                mScreen._11 = 0.5f;
                mScreen._22 = -0.5f;
                mScreen._41 = 0.5f / fShaderResolution + 0.5f;
                mScreen._42 = mScreen._41;
                mScreen._44 = 1.0f;

                D3DXMatrixMultiply(&mTemp, &mObjToWorld, &mObjToView);
                D3DXMatrixMultiply(&mResult, &mTemp, &mProj);
                D3DXMatrixMultiply(&mTemp, &mResult, &mScreen);
                mResult = mTemp;
            }
            D3DXMatrixTranspose(&mTemp, &mResult);
            SetFloatArray(reinterpret_cast<const float*>(&mTemp), 16);
            break;
        }

        case 15:
        {
            // Object-to-world matrix
            D3DXMATRIX mObjToWorld, mResult;
            Transform3x3To4x4Matrix(mObjToWorld, pContext->m_ObjectToWorldMatrix.m0, pContext->m_ObjectToWorldMatrix.p0);
            D3DXMatrixTranspose(&mResult, &mObjToWorld);
            SetFloatArray(reinterpret_cast<const float*>(&mResult), 16);
            break;
        }

        case 16:
        {
            // Camera position in object space
            float aCameraPos[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            aCameraPos[0] = -pContext->m_WorldToViewMatrix.p0.x;
            aCameraPos[1] = -pContext->m_WorldToViewMatrix.p0.y;
            aCameraPos[2] = -pContext->m_WorldToViewMatrix.p0.z;
            vmtmul(aCameraPos, pContext->m_WorldToViewMatrix.m0);

            vsub(aCameraPos, &pContext->m_ObjectToWorldMatrix.p0.x);
            vmtmul(aCameraPos, pContext->m_ObjectToWorldMatrix.m0);

            SetFloatArray(aCameraPos, 4);
            break;
        }

        case 17:
        {
            // View position in object space
            float aCameraPos[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
            aCameraPos[0] = -pContext->m_WorldToViewMatrix.p0.x;
            aCameraPos[1] = -pContext->m_WorldToViewMatrix.p0.y;
            aCameraPos[2] = -pContext->m_WorldToViewMatrix.p0.z;
            vmtmul(aCameraPos, pContext->m_WorldToViewMatrix.m0);
            SetFloatArray(aCameraPos, 4);
            break;
        }

        case 18:
        {
            // Camera position in object space (from object-to-view)
            float aCameraPos[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            aCameraPos[0] = -pContext->m_ObjectToViewMatrix.p0.x;
            aCameraPos[1] = -pContext->m_ObjectToViewMatrix.p0.y;
            aCameraPos[2] = -pContext->m_ObjectToViewMatrix.p0.z;
            vmtmul(aCameraPos, pContext->m_ObjectToViewMatrix.m0);

            vsub(aCameraPos, &pContext->m_ObjectToWorldMatrix.p0.x);
            vmtmul(aCameraPos, pContext->m_ObjectToWorldMatrix.m0);

            SetFloatArray(aCameraPos, 4);
            break;
        }

        case 19:
        {
            // TODO: Finish this place after sub_491270 (spotlight parameters helper) will be reversed
            // float aSpotParams[4]; float aDistances[3];
            // sub_491270(aSpotParams, aDistances);
            // float aResult[4];
            // aResult[0] = -(aDistances[0] * (1.0f / (aDistances[1] - aDistances[0])));
            // aResult[1] = 1.0f / (aDistances[1] - aDistances[0]);
            // aResult[2] = 2.5f / (aDistances[1] - aDistances[0]);
            // aResult[3] = aDistances[0];
            // SetFloatArray(aResult, 4);
            break;
        }

        case 20:
        {
            // Light attributes
            switch (pLight->lLightType)
            {
            case 0:
            case 1:
            case 2:
            {
                // TODO: Finish this place after sub_490CB0 (light attributes helper) will be reversed
                const auto* pLightOmni = static_cast<const SPrimLightOmni*>(pLight);
                // float aAttr[2];
                // sub_490CB0(aAttr[0], aAttr[1], pLightOmni->fNearRange, pLightOmni->fFarRange);
                // float aResult[4];
                // aResult[0] = aAttr[0];
                // aResult[1] = aAttr[1];
                // aResult[2] = pLight->fMultiplier;
                // aResult[3] = 1.0f / pLightOmni->fFarRange;
                // SetFloatArray(aResult, 4);
                break;
            }
            case 3:
            case 4:
            {
                const float aResult[4] =
                {
                    1.0f,
                    0.0f,
                    pLight->fMultiplier,
                    0.000016666667f,
                };
                SetFloatArray(aResult, 4);
                break;
            }
            default:
                ZASSERT(false);
                {
                    const float aZero[4] = {};
                    SetFloatArray(aZero, 4);
                }
                break;
            }
            break;
        }

        case 21:
        {
            // Light color
            switch (pLight->lLightType)
            {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            {
                float aColor[4];
                UnpackD3DCOLORToFloats(aColor[0], aColor[1], aColor[2], aColor[3], pLight->lDiffuseColor);
                SetFloatArray(aColor, 4);
                break;
            }
            default:
                ZASSERT(false);
                {
                    const float aZero[4] = {};
                    SetFloatArray(aZero, 4);
                }
                break;
            }
            break;
        }

        case 22:
        {
            const float aValue[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
            SetFloatArray(aValue, 4);
            break;
        }

        case 29:
        {
            // Shadow map color texture
            // TODO: Finish this place after g_texShadowMapColor storage will be reversed
            const uint32_t lShadowMapIndex = *reinterpret_cast<const uint32_t*>(&pContext->m_vFogNearPlane[1]);
            const uint32_t lTexture = *reinterpret_cast<const uint32_t*>(&g_texShadowMapColor[lShadowMapIndex].m_usSize);
            SetTexture(reinterpret_cast<IDirect3DBaseTexture9*>(lTexture));
            break;
        }

        case 31:
        {
            // Shadow map color texture (possibly blurred)
            // TODO: Finish this place after g_texShadowMapColor storage / sub_490AD0 will be reversed
            const uint32_t lShadowMapIndex = *reinterpret_cast<const uint32_t*>(&pContext->m_vFogNearPlane[1]);
            uint32_t lTexture = *reinterpret_cast<const uint32_t*>(&g_texShadowMapColor[lShadowMapIndex].m_usSize);

            if (ZSharedResourcesD3D::g_pInstance->m_bBlurDropShadow && !s_bBlurDropShadowActive)
            {
                // lTexture = sub_490AD0(lTexture);
            }

            SetTexture(reinterpret_cast<IDirect3DBaseTexture9*>(lTexture));
            break;
        }

        case 34:
        case 35:
        case 36:
        case 37:
        {
            // Environment texture slots
            SetTexture(ZSharedResourcesD3D::g_pInstance->m_pEnvTextures[m_lContextType - 34]);
            break;
        }

        case 38:
        {
            // TODO: Finish this place after ZRender will be reversed (field at m_pRender + 0x16EC)
            const uint8_t* pRender = reinterpret_cast<const uint8_t*>(pContext->m_pRender);
            const uint32_t lTexture = *reinterpret_cast<const uint32_t*>(pRender + 0x16EC);
            SetTexture(reinterpret_cast<IDirect3DBaseTexture9*>(lTexture));
            break;
        }

        case 39:
        {
            // TODO: Finish this place after ZRender will be reversed (field at m_pRender + 0x16F0)
            const uint8_t* pRender = reinterpret_cast<const uint8_t*>(pContext->m_pRender);
            const uint32_t lTexture = *reinterpret_cast<const uint32_t*>(pRender + 0x16F0);
            SetTexture(reinterpret_cast<IDirect3DBaseTexture9*>(lTexture));
            break;
        }

        case 40:
        {
            // Spot attenuation map
            const uint32_t lTextureId = pLight->lProjectorMap;
            if (lTextureId)
            {
                ZTextureD3D* pTexture = g_pRenderDll->m_pTexCon->GetTexture(lTextureId, 0);
                if (!pTexture)
                {
                    pTexture = &g_texWhite;
                }
                SetTexture(reinterpret_cast<IDirect3DBaseTexture9*>(pTexture->m_pUserData));
            }
            else
            {
                switch (pLight->lLightType)
                {
                case 0:
                {
                    const auto* pLightSpot = static_cast<const SPrimLightSpot*>(pLight);
                    const float fAspect = pLightSpot->fHotSpot / pLightSpot->fFallOff;
                    const uint32_t lMapIndex = (fAspect >= 0.5f) ? 1 : 0;
                    const uint32_t lTexture = *reinterpret_cast<const uint32_t*>(&g_pSpotAttenuationMap[lMapIndex].m_usSize);
                    SetTexture(reinterpret_cast<IDirect3DBaseTexture9*>(lTexture));
                    break;
                }
                case 1:
                case 3:
                case 4:
                {
                    const uint32_t lTexture = *reinterpret_cast<const uint32_t*>(&g_texWhite.m_usSize);
                    SetTexture(reinterpret_cast<IDirect3DBaseTexture9*>(lTexture));
                    break;
                }
                case 2:
                {
                    const auto* pLightSpot = static_cast<const SPrimLightSpot*>(pLight);
                    const float fAspect = pLightSpot->fHotSpot / pLightSpot->fFallOff;
                    const uint32_t lMapIndex = (fAspect >= 0.5f) ? 1 : 0;
                    const uint32_t lTexture = *reinterpret_cast<const uint32_t*>(&g_pSpotAttenuationMap2[lMapIndex].m_usSize);
                    SetTexture(reinterpret_cast<IDirect3DBaseTexture9*>(lTexture));
                    break;
                }
                default:
                    ZASSERT(false);
                    SetTexture(nullptr);
                    break;
                }
            }
            break;
        }

        case 41:
        {
            // Deform (bone) matrices, 26 * 3x4 -> 312 floats
            if (!pContext->m_nCurrentPass)
            {
                s_lDeformBoneCache = 0;
            }

            const uintptr_t lDeformBones = reinterpret_cast<uintptr_t>(pContext->m_pDeformBones);
            if (s_lDeformBoneCache != lDeformBones)
            {
                // The handle resolves to a weighted mesh (SPrimInfo::GetBoneDeclChunk == ZPrimHandle::Get<T>)
                const ZPrimHandle& hPrim = pContext->m_pRenderObjectInstance->m_pRenderObject->m_hPrim;
                const auto* pMesh = hPrim.Get<SPrimMeshWeighted>();

                s_lDeformBoneCache = lDeformBones;
                uint32_t lNumFloats = 0;

                if (lDeformBones == 0 || s_bDisableMPS)
                {
                    // Fill with 26 identity 3x4 matrices
                    for (uint32_t i = 0; i < 26; ++i)
                    {
                        float* pMtx = &s_aDeformBoneMatrices[i * 12];
                        pMtx[0] = 1.0f;  pMtx[1] = 0.0f;  pMtx[2] = 0.0f;  pMtx[3] = 0.0f;
                        pMtx[4] = 0.0f;  pMtx[5] = 1.0f;  pMtx[6] = 0.0f;  pMtx[7] = 0.0f;
                        pMtx[8] = 0.0f;  pMtx[9] = 0.0f;  pMtx[10] = 1.0f; pMtx[11] = 0.0f;
                    }
                    lNumFloats = 312;
                }
                else
                {
                    ZASSERT(pMesh->lType == EPrimType::PTMESH);

                    // Copy bone segments: {numBones, deformBoneIndex} pairs from the copy-bones chunk
                    const uint32_t lNumCopyBones = pMesh->lNumCopyBones;
                    if (lNumCopyBones != 0)
                    {
                        const uint32_t* pCopyBones = ZPrimHandle{pMesh->lCopyBones}.Get<uint32_t>();
                        const float* pDeformBones = reinterpret_cast<const float*>(lDeformBones);
                        uint32_t lRemaining = lNumCopyBones;

                        while (lRemaining != 0)
                        {
                            const uint32_t lNumBones = pCopyBones[0];
                            const uint32_t lBoneIndex = pCopyBones[1];
                            pCopyBones += 2;

                            if (lNumBones > 312u - lNumFloats)
                            {
                                ZASSERT(false);
                            }

                            memcpy(&s_aDeformBoneMatrices[lNumFloats], &pDeformBones[lBoneIndex], lNumBones * sizeof(float));
                            lNumFloats += lNumBones;
                            --lRemaining;
                        }
                    }
                }

                SetFloatArray(s_aDeformBoneMatrices, lNumFloats);
            }
            break;
        }

        case 44:
        {
            // Bone-light matrix array (28 floats)
            // TODO: Finish this place after the bone light data will be reversed
            const uint8_t* pBonesLight = reinterpret_cast<const uint8_t*>(pContext->m_pBonesLight);
            s_lBoneLightCache = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(pContext->m_pBonesLight));

            float aResult[28] = {};
            if (pBonesLight == reinterpret_cast<const uint8_t*>(32))
            {
                std::memset(aResult, 1, sizeof(aResult));
            }
            else if (pBonesLight)
            {
                // Bone light transform: interleaved bone data (3x4 matrices) transformed by object basis
                const float* pBonesLightF = reinterpret_cast<const float*>(pBonesLight);
                vmtmul(&aResult[0], &pBonesLightF[0], pContext->m_ObjectToWorldMatrix.m0);
                pcpy(&aResult[4], &pBonesLightF[4]);
                vmtmul(&aResult[8], &pBonesLightF[8], pContext->m_ObjectToWorldMatrix.m0);
                pcpy(&aResult[12], &pBonesLightF[12]);
                vmtmul(&aResult[16], &pBonesLightF[16], pContext->m_ObjectToWorldMatrix.m0);
                pcpy(&aResult[20], &pBonesLightF[20]);
                pcpy(&aResult[24], &pBonesLightF[24]);
            }

            SetFloatArray(aResult, 28);
            break;
        }

        case 46:
        {
            // Bone-light 3x4 matrix (12 floats)
            // TODO: Finish this place after the bone light data will be reversed
            const uint8_t* pBonesLight = reinterpret_cast<const uint8_t*>(pContext->m_pBonesLight);
            if (s_lBonesLight2Cache != static_cast<uint32_t>(reinterpret_cast<uintptr_t>(pContext->m_pBonesLight)))
            {
                s_lBonesLight2Cache = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(pContext->m_pBonesLight));

                float aResult[12] = {};
                if (pBonesLight == reinterpret_cast<const uint8_t*>(32))
                {
                    std::memset(aResult, 1, sizeof(aResult));
                }
                else if (pBonesLight)
                {
                    // TODO: Finish this place after the bone light data will be reversed
                    // const float* pBonesLightF = reinterpret_cast<const float*>(pBonesLight);
                    // pcpy(&aResult[0], &pBonesLightF[24]);
                    // vmtmul(&aResult[4], &pBonesLightF[0], pContext->m_ObjectToWorldMatrix.m0);
                    // pcpy(&aResult[8], &pBonesLightF[4]);
                }

                SetFloatArray(aResult, 12);
            }
            break;
        }

        case 47:
        {
            // Highlight/outline flag or object fade
            const float fValue = (pLight->lLightControl & 8) != 0 ? 1.0f : pContext->m_fObjectFade;
            SetFloatArray(&fValue, 1);
            break;
        }

        case 48:
        {
            // Texcoord scroll frame fraction
            const ZPrimHandle& hPrim = pContext->m_pRenderObjectInstance->m_pRenderObject->m_hPrim;
            const auto* pMesh = hPrim.Get<SPrimMesh>();
            // TODO: Finish this place after ZRenderEntry will be reversed (m_pRenderEntry[1].m_PAD4)
            // const float fFrame = *reinterpret_cast<const float*>(<m_pRenderEntry[1].m_PAD4>);
            // const float fFraction = (fFrame - static_cast<float>(static_cast<uint16_t>(pMesh->lNumFrames)))
            //     / static_cast<float>(static_cast<uint16_t>(pMesh->lNumFrames >> 16));
            // float fFrac = fFraction - std::floor(fFraction);
            // SetFloatArray(&fFrac, 1);
            break;
        }

        case 49:
        {
            // Scaled game time
            const float fTime = static_cast<float>(static_cast<double>(g_pSysInterface->FrameTime.secs) * 0.0009765625);
            SetFloatArray(&fTime, 1);
            break;
        }

        case 51:
        {
            // Shadow cast flag
            const float fValue = (pContext->m_lShadowId[3] & 0xFF) != 0 ? 1.0f : 0.0f;
            SetFloatArray(&fValue, 1);
            break;
        }

        case 52:
        {
            // Camera position (object space) plane / distance to shadow plane
            D3DXMATRIX mInvObj;
            tmat(&mInvObj.m[0][0], pContext->m_ObjectToWorldMatrix.m0);
            float* pInvObj = &mInvObj.m[0][0];

            vmtmul(&pInvObj[9], &pContext->m_ObjectToWorldMatrix.p0.x, pContext->m_ObjectToWorldMatrix.m0);
            pInvObj[9] = -pInvObj[9];
            pInvObj[10] = -pInvObj[10];
            pInvObj[11] = -pInvObj[11];

            float aResult[4];
            vmmul(&aResult[0], &pContext->m_vFogNearPlane[2], pInvObj);
            aResult[3] = *reinterpret_cast<const float*>(&pContext->m_lShadowId[1])
                - pInvObj[11] * aResult[2]
                - pInvObj[10] * aResult[1]
                - pInvObj[9] * aResult[0];
            SetFloatArray(aResult, 4);
            break;
        }

        case 54:
        {
            // TODO: Finish this place after ZRender will be reversed (dword at m_pRender + 0x1334)
            const uint8_t* pRender = reinterpret_cast<const uint8_t*>(pContext->m_pRender);
            const float fValue = ((*reinterpret_cast<const uint32_t*>(pRender + 0x1334) & 2) == 0) ? 1.0f : 0.0f;
            SetFloatArray(&fValue, 1);
            break;
        }

        case 55:
        {
            // Vertex color with override flag
            const ZPrimHandle& hPrim = pContext->m_pRenderObjectInstance->m_pRenderObject->m_hPrim;
            const auto* pMesh = hPrim.Get<SPrimMesh>();
            if (pMesh->lType == EPrimType::PTMESH)
            {
                uint32_t lWireColor = pMesh->lWireColor;
                // TODO: Finish this place after ZBaseGeom::m_mTransform / sub_431270 will be reversed
                // const uint32_t* pTransform = pContext->m_pRenderObjectInstance->m_pBaseGeom->m_mTransform.data;
                // if (pTransform && (sub_431270(pTransform) & 0x10) != 0)
                //     lWireColor = 0xFF1F1F1F;

                float aColor[4];
                aColor[0] = static_cast<float>((lWireColor >> 16) & 0xFF) * (1.0f / 255.0f);
                aColor[1] = static_cast<float>((lWireColor >> 8) & 0xFF) * (1.0f / 255.0f);
                aColor[2] = static_cast<float>(lWireColor & 0xFF) * (1.0f / 255.0f);
                // TODO: Finish this place after ZRender will be reversed (render flags, see decompile)
                aColor[3] = 0.0f;
                SetFloatArray(aColor, 4);
            }
            break;
        }

        case 56:
        {
            // Static shadow vertex color
            const ZPrimHandle& hPrim = pContext->m_pRenderObjectInstance->m_pRenderObject->m_hPrim;
            const auto* pMesh = hPrim.Get<SPrimMesh>();
            if (pMesh->lType != EPrimType::PTMESH)
            {
                break;
            }
            // TODO: Finish this place after IsShadowMeshObj will be reversed
            // IsShadowMeshObj(pContext->m_pRenderObjectInstance->m_pBaseGeom->m_pGeom);
            // TODO: Finish this place after the mesh prim layout will be reversed (color at +0x78)
            // const uint32_t lColor = *reinterpret_cast<const uint32_t*>(reinterpret_cast<const uint8_t*>(pMesh) + 0x78);
            // float aColor[4];
            // aColor[0] = static_cast<float>((lColor >> 16) & 0xFF) * (1.0f / 255.0f);
            // aColor[1] = static_cast<float>((lColor >> 8) & 0xFF) * (1.0f / 255.0f);
            // aColor[2] = static_cast<float>(lColor & 0xFF) * (1.0f / 255.0f);
            // aColor[3] = 1.0f;
            // SetFloatArray(aColor, 4);
            break;
        }

        case 57:
        {
            // UV/texture projection matrix copied raw from prim data
            const ZPrimHandle& hPrim = pContext->m_pRenderObjectInstance->m_pRenderObject->m_hPrim;
            const auto* pMesh = hPrim.Get<SPrimMesh>();
            if (pMesh->lType != EPrimType::PTMESH)
            {
                break;
            }
            // TODO: Finish this place after the mesh prim layout will be reversed (matrix pointer at +0x38)
            // float aResult[16];
            // memcpy(aResult, *reinterpret_cast<const void* const*>(reinterpret_cast<const uint8_t*>(pMesh) + 0x38), sizeof(aResult));
            // SetFloatArray(aResult, 16);
            break;
        }

        case 58:
        {
            // Fog color
            float aColor[4];
            UnpackD3DCOLORToFloats(
                aColor[0], aColor[1], aColor[2], aColor[3],
                *reinterpret_cast<const uint32_t*>(&pContext->m_vFogNearPlane[0]));
            SetFloatArray(aColor, 4);
            break;
        }

        case 59:
        {
            // Bone weight blend table (64 floats)
            const ZPrimHandle& hPrim = pContext->m_pRenderObjectInstance->m_pRenderObject->m_hPrim;
            const auto* pMesh = hPrim.Get<SPrimMesh>();
            // TODO: Finish this place after the mesh prim layout will be reversed (bone count at +0x28)
            // const uint32_t lNumBones = <HIBYTE of dword at +0x28>;
            // ... build 16 pairs of {i/count, 0.96, i/count, 0.04} blend weights ...
            // SetFloatArray(aResult, 64);
            (void)pMesh;
            break;
        }

        case 60:
        {
            // Texture scroll/scale data (16 floats)
            const ZPrimHandle& hPrim = pContext->m_pRenderObjectInstance->m_pRenderObject->m_hPrim;
            const auto* pMesh = hPrim.Get<SPrimMesh>();
            // TODO: Finish this place after the mesh prim layout will be reversed
            (void)pMesh;
            break;
        }

        case 63:
        {
            // Shadow projection matrix (256^2 atlas space)
            D3DXMATRIX mObjToWorld, mLightView, mProj, mShadow, mTemp, mResult;
            Transform3x3To4x4Matrix(mObjToWorld, pContext->m_ObjectToWorldMatrix.m0, pContext->m_ObjectToWorldMatrix.p0);
            Transform3x3To4x4Matrix(mLightView, pContext->m_WorldToLightMatrix[1].m0, pContext->m_WorldToLightMatrix[1].p0);

            memcpy(&mProj, &pContext->m_aLightClipMatrix[2], sizeof(mProj));

            ZeroMemory(&mShadow, sizeof(mShadow));
            mShadow._11 = 256.0f;
            mShadow._22 = -256.0f;
            mShadow._33 = 65535.0f;
            mShadow._41 = 256.5f;
            mShadow._42 = 256.5f;
            mShadow._44 = 1.0f;

            D3DXMatrixMultiply(&mTemp, &mObjToWorld, &mLightView);
            D3DXMatrixMultiply(&mResult, &mTemp, &mProj);
            D3DXMatrixMultiply(&mTemp, &mResult, &mShadow);
            D3DXMatrixTranspose(&mResult, &mTemp);
            SetFloatArray(reinterpret_cast<const float*>(&mResult), 16);
            break;
        }

        case 64:
        {
            // Matrix from drawable data
            // TODO: Finish this place after sub_473700 (drawable lookup) will be reversed
            // if (!pContext->m_pRenderObjectInstance->m_pRenderEntry) break;
            // const void* pData = sub_473700(<render stats>);
            // float aResult[16]; memcpy(aResult, pData, sizeof(aResult));
            // SetFloatArray(aResult, 16);
            break;
        }

        case 65:
        {
            // Color from drawable data
            // TODO: Finish this place after sub_473700 / sub_473720 will be reversed
            break;
        }

        case 66:
        {
            // Light position (object space) from drawable data
            // TODO: Finish this place after sub_473700 will be reversed
            // if (!pContext->m_pRenderObjectInstance->m_pRenderEntry) break;
            // const float* pData = static_cast<const float*>(sub_473700(<render stats>));
            // float aLightPos[4] = { pData[17], pData[18], pData[19], 1.0f };
            // vsub(aLightPos, &pContext->m_ObjectToWorldMatrix.p0.x);
            // vmtmul(aLightPos, pContext->m_ObjectToWorldMatrix.m0);
            // SetFloatArray(aLightPos, 4);
            break;
        }

        case 67:
        {
            // Decal texture with address mode
            // TODO: Finish this place after sub_473700 (render stats) will be reversed
            // if (pContext->m_pRenderObjectInstance->m_pRenderEntry)
            // {
            //     const uint32_t lValue = *reinterpret_cast<const uint32_t*>(static_cast<const uint8_t*>(sub_473700(<render stats>)) + 64);
            //     g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, ~((lValue & 0xC0000000) >> 30) & 2 | 1);
            //     g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, ~((lValue & 0xC0000000) >> 29) & 2 | 1);
            //     ZTextureD3D* pTexture = g_pRenderDll->m_pTexCon->GetTexture(static_cast<uint16_t>(lValue), 0);
            //     SetTexture(reinterpret_cast<IDirect3DBaseTexture9*>(pTexture->m_pUserData));
            // }
            break;
        }

        case 68:
        {
            // View * clip * screen (viewport) matrix
            D3DXMATRIX mWorldToView, mProj, mScreen, mViewportRatio, mTemp, mResult;
            Transform3x3To4x4Matrix(mWorldToView, pContext->m_WorldToViewMatrix.m0, pContext->m_WorldToViewMatrix.p0);

            memcpy(&mProj, &pContext->m_ProjectionMatrix, sizeof(mProj));

            ZeroMemory(&mScreen, sizeof(mScreen));
            mScreen._11 = 0.5f;
            mScreen._22 = -0.5f;
            mScreen._41 = 0.5f;
            mScreen._42 = 0.5f;
            mScreen._43 = 1.0f;
            mScreen._44 = 1.0f;

            ZeroMemory(&mViewportRatio, sizeof(mViewportRatio));
            mViewportRatio._11 = 1.0f;
            mViewportRatio._22 = 1.0f;
            mViewportRatio._33 = 1.0f;
            mViewportRatio._44 = 1.0f;

            D3DVIEWPORT9 viewport;
            g_pd3dDevice->GetViewport(&viewport);

            IDirect3DSurface9* pSurface = nullptr;
            g_pd3dDevice->GetRenderTarget(0, &pSurface);

            D3DSURFACE_DESC surfaceDesc;
            if (pSurface)
            {
                pSurface->GetDesc(&surfaceDesc);
                pSurface->Release();
            }

            mViewportRatio._11 = static_cast<float>(viewport.Width) / static_cast<float>(surfaceDesc.Width);
            mViewportRatio._22 = static_cast<float>(viewport.Height) / static_cast<float>(surfaceDesc.Height);
            mViewportRatio._41 = static_cast<float>(viewport.X) / static_cast<float>(surfaceDesc.Width);
            mViewportRatio._42 = static_cast<float>(viewport.Y) / static_cast<float>(surfaceDesc.Height);

            D3DXMatrixMultiply(&mTemp, &mWorldToView, &mProj);
            D3DXMatrixMultiply(&mResult, &mTemp, &mScreen);
            D3DXMatrixMultiply(&mTemp, &mResult, &mViewportRatio);
            D3DXMatrixTranspose(&mResult, &mTemp);
            SetFloatArray(reinterpret_cast<const float*>(&mResult), 16);
            break;
        }

        default:
            return;
        }
    }

    void ZRenderBinderContextD3DFX::SetFloatArray(const float* pFloats, uint32_t lNumFloats)
    {
        m_pEffect->m_pD3DXEffect->SetFloatArray(m_hParamter, pFloats, lNumFloats);
    }

    void ZRenderBinderContextD3DFX::SetTexture(IDirect3DBaseTexture9* pTexture)
    {
        m_pEffect->m_pD3DXEffect->SetTexture(m_hParamter, pTexture);
    }
}
