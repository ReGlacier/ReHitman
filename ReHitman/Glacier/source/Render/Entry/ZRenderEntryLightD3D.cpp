#include <Glacier/Render/Entry/ZRenderEntryLightD3D.h>
#include <Glacier/Geom/ZLIGHT.h>
#include <Glacier/Geom/ZCAMERA.h>
#include <Glacier/Render/Cmd/ZRenderCommands.h>
#include <Glacier/Render/Draw/ZRenderDraw.h>
#include <Glacier/Render/Entry/ZRenderEntryCameraD3D.h>
#include <Glacier/Render/Entry/ZRenderEntryLists.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/Render/Prim/ELightType.h>
#include <Glacier/Render/Prim/SPrimLightSpot.h>
#include <Glacier/Render/Prim/SPrimLightSpotSquare.h>
#include <Glacier/Render/ZSharedResourcesD3D.h>
#include <Glacier/Render/ZViewSpace.h>
#include <Glacier/Render/View/ZRenderView.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZUniMemory.h>
#include <algorithm>
#include <cmath>
#include <cstring>


namespace Glacier
{
    namespace
    {
        struct LightSetupPayload
        {
            uint8_t lShadow;
            uint8_t pad[3];
            const SPrimLight* pLight;
            ZVector3 vMin;
            ZVector3 vMax;
            ZMatrix worldToLight;
            ZMat4x4 projection;
        };
        RE_VERIFY_SIZE(LightSetupPayload, 0x90);

        struct LightShadowPayload
        {
            ZMatrix worldToLight;
            ZMat4x4 projection;
            uint32_t lUnused;
        };
        RE_VERIFY_SIZE(LightShadowPayload, 0x74);

        struct LightPassPayload
        {
            float data[4];
            uint8_t bLight;
            uint8_t pad[3];
        };
        RE_VERIFY_SIZE(LightPassPayload, 0x14);

        struct LightHandlePayload
        {
            const SPrimLight* pLight;
            uint8_t lIndex;
            uint8_t pad[3];
        };
        RE_VERIFY_SIZE(LightHandlePayload, 8);

        ZCmdList::ZCmd* BeginLightCommand(ZCmdList* pCmdList, ZRenderView* pView, ZCmdList::CMD eType)
        {
            auto* pCmd = pCmdList->Current();
            pCmd->m_lType = eType;
            pCmd->m_pRenderEntryGeom = nullptr;
            pCmd->m_pCmdList = pCmdList;
            pCmd->m_pRenderView = pView;
            pCmd->m_lLayer = 0;
            pCmd->m_lNrObjects = 0;
            pCmdList->NextCommand();
            return pCmd;
        }

        ZVector3 TransformPoint(const ZMatrix& matrix, const ZVector3& point)
        {
            ZVector3 result = point;
            TransformRootVector(result, matrix.m0);
            result += matrix.p0;
            return result;
        }

        void BuildPerspective(ZMat4x4& projection, float fov, float aspect, float nearPlane, float farPlane)
        {
            std::memset(projection.data, 0, sizeof(projection.data));
            const float yScale = 1.0f / std::tan(fov * 0.5f);
            projection.data[0] = yScale / aspect;
            projection.data[5] = yScale;
            projection.data[10] = farPlane / (farPlane - nearPlane);
            projection.data[11] = 1.0f;
            projection.data[14] = -farPlane * nearPlane / (farPlane - nearPlane);
        }

        void AddLightDraw(ZCmdList* pCmdList, ZRenderView* pView, ZRenderEntry** ppEntries,
            uint32_t lCount, const ZVector3& observer, float lodScale, uint32_t layer)
        {
            CmdDrawEntries(pCmdList, pView, const_cast<const ZRenderEntry**>(ppEntries), lCount,
                observer, lodScale, layer, 1, 7);
        }

        void MakePlane(float* pPlane, const ZVector3& a, const ZVector3& b, const ZVector3& c,
            const ZVector3& inside)
        {
            const ZVector3 ab = b - a;
            const ZVector3 ac = c - a;
            vcross(pPlane, ab.Get(), ac.Get());
            vnorm(pPlane);
            pPlane[3] = -vdot(pPlane, a.Get());
            if (vdot(pPlane, inside.Get()) + pPlane[3] < 0.0f)
            {
                pPlane[0] = -pPlane[0];
                pPlane[1] = -pPlane[1];
                pPlane[2] = -pPlane[2];
                pPlane[3] = -pPlane[3];
            }
        }

        bool ProjectBounds(ZVector4& bounds, const ZVector3* pCorners, uint32_t count,
            const ZMatrix& cameraToWorld, const ZMat4x4& projection, const uint32_t* viewport)
        {
            float minX = 9.9999997e37f;
            float maxX = -9.9999997e37f;
            float minY = 9.9999997e37f;
            float maxY = -9.9999997e37f;
            const float width = static_cast<float>(viewport[2] - viewport[0]);
            const float height = static_cast<float>(viewport[3] - viewport[1]);
            for (uint32_t i = 0; i < count; ++i)
            {
                ZVector3 view = pCorners[i] - cameraToWorld.p0;
                TransformLocalVector(view, cameraToWorld.m0);
                float ndcX;
                float ndcY;
                if (view.z > 0.0f)
                {
                    const float clipX = projection.data[0] * view.x + projection.data[4] * view.y
                        + projection.data[8] * view.z + projection.data[12];
                    const float clipY = projection.data[1] * view.x + projection.data[5] * view.y
                        + projection.data[9] * view.z + projection.data[13];
                    const float clipW = projection.data[3] * view.x + projection.data[7] * view.y
                        + projection.data[11] * view.z + projection.data[15];
                    const float invW = clipW != 0.0f ? 1.0f / clipW : 1.0f;
                    ndcX = (std::max)(-1.0f, (std::min)(1.0f, clipX * invW));
                    ndcY = (std::max)(-1.0f, (std::min)(1.0f, clipY * invW));
                }
                else
                {
                    ndcX = view.x >= 0.0f ? 1.0f : -1.0f;
                    ndcY = view.y >= 0.0f ? 1.0f : -1.0f;
                }
                const float screenX = ndcX * width * 0.5f + static_cast<float>(viewport[0]) + width * 0.5f;
                const float screenY = ndcY * height * 0.5f + static_cast<float>(viewport[1]) + height * 0.5f;
                minX = (std::min)(minX, screenX);
                maxX = (std::max)(maxX, screenX);
                minY = (std::min)(minY, screenY);
                maxY = (std::max)(maxY, screenY);
            }
            bounds = ZVector4(minX, maxX, height - minY, height - maxY);
            return minX != maxX && minY != maxY;
        }
    }

    ZRenderEntryLightD3D::ZRenderEntryLightD3D(const ZRenderEntryGeomCreateInfo& sInfo)
        : ZRenderEntryLight(sInfo)
    {
    }

    ZRenderEntryLightD3D::~ZRenderEntryLightD3D() = default;

    void ZRenderEntryLightD3D::GetVisible(ZCmdList* pCmdList, ZRenderEntryGeom* pGeomEntry, ZViewSpace* pViewSpace, ZRenderView* pView, ZRenderEntryLists* pEntryList)
    {
        if (!pCmdList || !pGeomEntry || !pViewSpace || !pView || !pEntryList
            || pGeomEntry->GetType() != RT_CAMERA)
            return;

        auto* pCameraEntry = static_cast<ZRenderEntryCameraD3D*>(pGeomEntry);
        if ((pCameraEntry->m_lDrawConForbid & 2u) != 0)
            return;

        const auto* pLight = this->m_hPrimLight.Get<SPrimLight>();
        if (!pLight || (pLight->lType & 2u) == 0 || !ZSharedResourcesD3D::g_pInstance
            || ZSharedResourcesD3D::g_pInstance->m_lShaderQuality < 2u)
            return;

        auto* pCameraBase = pGeomEntry->GetBaseGeom();
        auto* pCamera = pCameraBase ? static_cast<ZCAMERA*>(pCameraBase->GetGeom()) : nullptr;
        auto* pDraw = static_cast<ZRenderDraw*>(IDraw::Instance());
        if (!pCamera || !pDraw)
            return;

        const auto lightType = pLight->lLightType;
        const float farRange = (lightType <= LTSPOTSQUARE)
            ? static_cast<const SPrimLightOmni*>(pLight)->fFarRange : 100000.0f;
        const float nearRange = (lightType <= LTSPOTSQUARE)
            ? static_cast<const SPrimLightOmni*>(pLight)->fNearRange : 5.0f;
        const float cone = (lightType == LTOMNI)
            ? 1.5707964f : static_cast<const SPrimLightSpot*>(pLight)->fFallOff * 2.0f;
        const float aspect = (lightType == LTSPOTSQUARE)
            ? static_cast<const SPrimLightSpotSquare*>(pLight)->fAspect : 1.0f;

        ZMatrix lightToWorld = m_ObjectToWorldMatrix;
        if (lightType == LTENVIRONMENT || lightType == LTDIRECTIONAL)
            lightToWorld.p0 = pCameraEntry->m_ObjectToWorldMatrix.p0 - lightToWorld.m0.ZAxis() * 50000.0f;
        ZMatrix worldToLight;
        tmat(worldToLight.m0, lightToWorld.m0);
        worldToLight.p0 = lightToWorld.p0;
        TransformLocalVector(worldToLight.p0, worldToLight.m0);
        worldToLight.p0.x = -worldToLight.p0.x;
        worldToLight.p0.y = -worldToLight.p0.y;
        worldToLight.p0.z = -worldToLight.p0.z;

        ZMat4x4 projection;
        BuildPerspective(projection, cone, aspect, (std::max)(nearRange, 5.0f), farRange);

        // The PC tests the light volume against all six camera planes before
        // it mutates the view space. Reject only when every volume vertex is
        // outside a plane, which preserves intersection at the frustum edge.
        ZVector3 half(farRange * std::tan(cone * 0.5f) * aspect,
            farRange * std::tan(cone * 0.5f), farRange);
        if (lightType == LTOMNI)
            half = ZVector3(farRange, farRange, farRange);
        ZVector3 corners[8];
        uint32_t cornerCount = 0;
        if (lightType <= LTSPOTSQUARE)
        {
            for (int x = -1; x <= 1; x += 2)
                for (int y = -1; y <= 1; y += 2)
                {
                    if (lightType == LTOMNI)
                        corners[cornerCount++] = TransformPoint(lightToWorld, ZVector3(x * half.x, y * half.y, -half.z));
                    else if (cornerCount == 0)
                        corners[cornerCount++] = lightToWorld.p0;
                    corners[cornerCount++] = TransformPoint(lightToWorld, ZVector3(x * half.x, y * half.y, half.z));
                }
            for (uint32_t plane = 0; plane < 6; ++plane)
            {
                float p[4];
                pViewSpace->GetClipPlane(plane, p);
                bool outside = true;
                for (uint32_t i = 0; i < cornerCount; ++i)
                    outside &= vdot(p, corners[i].Get()) + p[3] < 0.0f;
                if (outside)
                    return;
            }
        }

        ZVector3 oldPosition, oldInnerRoom;
        pViewSpace->GetPosition(oldPosition);
        pViewSpace->GetPositionInnerRoom(oldInnerRoom);
        pViewSpace->SetPosition(lightToWorld.p0);
        pViewSpace->SetPositionInnerRoom(oldInnerRoom);

        float lightPlanes[6][4] {};
        uint32_t lightPlaneCount = 0;
        float oldPlanes[12][4] {};
        bool oldPlaneEnabled[12] {};
        for (uint32_t i = 0; i < 12; ++i)
        {
            pViewSpace->GetClipPlane(i, oldPlanes[i]);
            oldPlaneEnabled[i] = pViewSpace->SetClipPlaneEnabled(i, true);
            pViewSpace->SetClipPlaneEnabled(i, oldPlaneEnabled[i]);
        }
        if (lightType == LTOMNI)
        {
            static constexpr int faces[6][3] = {{0, 1, 3}, {4, 6, 7}, {0, 4, 5}, {2, 3, 7}, {0, 2, 6}, {1, 5, 7}};
            const ZVector3 inside = lightToWorld.p0;
            for (uint32_t i = 0; i < 6; ++i)
                MakePlane(lightPlanes[i], corners[faces[i][0]], corners[faces[i][1]], corners[faces[i][2]], inside);
            lightPlaneCount = 6;
        }
        else if (lightType == LTSPOT || lightType == LTSPOTSQUARE)
        {
            const ZVector3 inside = TransformPoint(lightToWorld, ZVector3(0.0f, 0.0f, farRange * 0.5f));
            MakePlane(lightPlanes[0], corners[0], corners[1], corners[2], inside);
            MakePlane(lightPlanes[1], corners[0], corners[2], corners[4], inside);
            MakePlane(lightPlanes[2], corners[0], corners[4], corners[3], inside);
            MakePlane(lightPlanes[3], corners[0], corners[3], corners[1], inside);
            MakePlane(lightPlanes[4], corners[1], corners[3], corners[4], inside);
            lightPlaneCount = 5;
        }
        for (uint32_t i = 0; i < 6; ++i)
        {
            if (i < lightPlaneCount)
                pViewSpace->SetClipPlane(6 + i, lightPlanes[i]);
            pViewSpace->SetClipPlaneEnabled(6 + i, i < lightPlaneCount);
        }

        ZVolumeList volumeList;
        pViewSpace->GetVisibleVolumes(&volumeList, false, nullptr);
        ZRenderEntry* entries[ZVolumeList::MAX_NUM_VISIBLE_RENDER_ENTRIES] {};
        ZRenderEntryLists lists;
        const ZVector3 observer = pGeomEntry->m_ObjectToWorldMatrix.p0;
        const float lodScale = pCameraEntry->m_matProjection._11;
        const uint32_t count = pDraw->CreateRenderEntries(entries, ZVolumeList::MAX_NUM_VISIBLE_RENDER_ENTRIES,
            &volumeList, &lists, this, const_cast<float*>(observer.Get()), lodScale);

        ZVector4 scissor;
        if (!ProjectBounds(scissor, corners, cornerCount, pCameraEntry->m_ObjectToWorldMatrix,
            *reinterpret_cast<const ZMat4x4*>(&pCameraEntry->m_matProjection), pView->Viewport()))
        {
            pViewSpace->SetPosition(oldPosition);
            pViewSpace->SetPositionInnerRoom(oldInnerRoom);
            for (uint32_t i = 0; i < 12; ++i)
            {
                pViewSpace->SetClipPlane(i, oldPlanes[i]);
                pViewSpace->SetClipPlaneEnabled(i, oldPlaneEnabled[i]);
            }
            return;
        }

        LightSetupPayload setup {};
        setup.lShadow = g_pSysInterface && g_pSysInterface->m_lShadowDetail > 0 ? 1 : 0;
        setup.pLight = pLight;
        setup.vMin = ZVector3(-half.x, -half.y, nearRange);
        setup.vMax = ZVector3(half.x, half.y, farRange);
        setup.worldToLight = worldToLight;
        setup.projection = projection;
        LightPassPayload pass {{ scissor.x, scissor.y, scissor.z, scissor.w }, 1, { 0, 0, 0 }};
        auto* pPass = BeginLightCommand(pCmdList, pView, static_cast<ZCmdList::CMD>(16));
        pPass->AddData(&pass, sizeof(pass));
        auto* pLightSetup = BeginLightCommand(pCmdList, pView, static_cast<ZCmdList::CMD>(3));
        pLightSetup->AddData(&setup, sizeof(setup));
        if (setup.lShadow)
        {
            auto* pShadow = BeginLightCommand(pCmdList, pView, static_cast<ZCmdList::CMD>(6));
            LightShadowPayload shadow { worldToLight, projection, 0 };
            pShadow->AddData(&shadow, sizeof(shadow));
            auto* pHandle = BeginLightCommand(pCmdList, nullptr, static_cast<ZCmdList::CMD>(8));
            LightHandlePayload handle { pLight, 0, { 0, 0, 0 } };
            pHandle->AddData(&handle, sizeof(handle));
            AddLightDraw(pCmdList, pView, entries, count, observer, lodScale, 2);
            BeginLightCommand(pCmdList, pView, static_cast<ZCmdList::CMD>(10));
        }
        else
        {
            auto* pShadow = BeginLightCommand(pCmdList, pView, static_cast<ZCmdList::CMD>(6));
            LightShadowPayload shadow { worldToLight, projection, 0 };
            pShadow->AddData(&shadow, sizeof(shadow));
        }
        AddLightDraw(pCmdList, pView, entries, count, observer, lodScale, 5);
        BeginLightCommand(pCmdList, pView, static_cast<ZCmdList::CMD>(11));
        LightPassPayload reset {};
        auto* pReset = BeginLightCommand(pCmdList, pView, static_cast<ZCmdList::CMD>(16));
        pReset->AddData(&reset, sizeof(reset));
        pViewSpace->SetPosition(oldPosition);
        pViewSpace->SetPositionInnerRoom(oldInnerRoom);
        for (uint32_t i = 0; i < 12; ++i)
        {
            pViewSpace->SetClipPlane(i, oldPlanes[i]);
            pViewSpace->SetClipPlaneEnabled(i, oldPlaneEnabled[i]);
        }
    }

    ZRenderEntryLightD3D* ZRenderEntryLightD3D::Create(const ZRenderEntryGeomCreateInfo& sInfo)
    {
        if (!sInfo.m_pBaseGeom || !sInfo.m_pBaseGeom->IsDerivedFrom<ZLIGHT>())
            return nullptr;

        return ZUniMemory::New<ZRenderEntryLightD3D>(sInfo);
    }
}
