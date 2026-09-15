#include <Glacier/Render/Decal/ZDecalMarkController.h>
#include <Glacier/Render/Prim/ZPrimAccessMesh.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/Draw/ZRenderDraw.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/Geom/GeomControlMasks.h>
#include <Glacier/Physics/ZCollisionBase.h>
#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/System/ZSysMem.h>
#include <Glacier/ZUniMemory.h>
#include <cstring>
#include <algorithm>
#include <cmath>


namespace Glacier
{
    // ZDecalMarkController
    ZDecalMarkController::ZDecalMarkController() :
        m_BaseGeomToLink(0x100)
    {
        m_pFirst = nullptr;
        m_pFirstCallBack = nullptr;
        m_pFirstCreated = nullptr;
        m_pFirstRemoved = nullptr;
    }

    ZDecalMarkController::~ZDecalMarkController()
    {
        RemoveAllDecals();
    }

    void ZDecalMarkController::Init()
    {
        m_pFirst = nullptr;
    }

    void ZDecalMarkController::End()
    {
        RemoveAllDecals();
    }

    void ZDecalMarkController::Flush()
    {
    }

    void ZDecalMarkController::BeginFrame()
    {
        // Create all queued marks
        ZDecalMark* pCreate = m_pFirstCreated;
        while (pCreate)
        {
            ZDecalMark* pNext = pCreate->m_pNextToCreate;
            Create(pCreate);
            pCreate->m_pNextToCreate = nullptr;
            pCreate = pNext;
        }
        m_pFirstCreated = nullptr;

        // Fire pending callbacks
        ZDecalMark* pCallBack = m_pFirstCallBack;
        while (pCallBack)
        {
            ZDecalCallBack callBack;
            ZDecalCallBack::ZDecalCallBackEntries entries[512]; // PC stack buffer
            callBack.m_lNumEntries = 0;
            callBack.m_pEntries = entries;

            for (const ZLink* pLink = pCallBack->m_pLinks; pLink; pLink = pLink->m_pNext)
            {
                ZASSERT((pLink->m_pBaseGeom->m_lControl & ZCRENDERATTACHED) != 0);
                entries[callBack.m_lNumEntries].m_pBaseGeom = pLink->m_pBaseGeom;
                entries[callBack.m_lNumEntries].m_pStoredUV = pLink->m_pStoredUV;
                entries[callBack.m_lNumEntries].m_pEditableMesh = pLink->m_pPrimAccessMesh;
                ++callBack.m_lNumEntries;
                ZASSERT(callBack.m_lNumEntries != 512);
            }

            if (pCallBack->m_CallBackDecal)
            {
                pCallBack->m_CallBackDecal(&callBack, pCallBack->m_pCallBackData, pCallBack->m_lCallBackValue);
            }
            pCallBack->m_CallBackDecal = nullptr;

            ZDecalMark* pNext = pCallBack->m_pNextCallBack;
            pCallBack->m_pNextCallBack = nullptr;
            pCallBack = pNext;
        }
        m_pFirstCallBack = nullptr;

        // Remove all queued marks
        ZDecalMark* pRemove = m_pFirstRemoved;
        while (pRemove)
        {
            ZDecalMark* pNext = pRemove->m_pNextToRemove;
            RemoveDecal(pRemove);
            pRemove = pNext;
        }
        m_pFirstRemoved = nullptr;
    }

    ZDecalMarkController::ZDecalMark* ZDecalMarkController::Add(const float* pvPosition, const float* pvDirection, uint32_t lSourcePrim, float fRadiusX, float fRadiusY, const float* pvExtraTextureSize, bool bStoreUV, float fRotation)
    {
        ZASSERT(lSourcePrim);

        ZDecalMark* pDecalMark = m_DecalMarks.Add();
        if (!pDecalMark)
        {
            return nullptr;
        }

        memset(pDecalMark, 0, sizeof(ZDecalMark));
        pDecalMark->m_vPosition = pvPosition;
        pDecalMark->m_vDirection = pvDirection;
        pDecalMark->m_fRotation = fRotation;
        pDecalMark->m_lSourcePrim = lSourcePrim;
        pDecalMark->m_fRadius.x = fRadiusX;
        pDecalMark->m_fRadius.y = fRadiusY;
        pDecalMark->m_fOpacity = 1.0f;
        memcpy(&pDecalMark->m_fExtraTextureSize, pvExtraTextureSize, sizeof(ZVector4));
        pDecalMark->m_bStoreUV = bStoreUV;

        // Link into the active marks list
        pDecalMark->m_pNext = m_pFirst;
        if (m_pFirst)
        {
            m_pFirst->m_pPrev = pDecalMark;
        }
        pDecalMark->m_pPrev = nullptr;
        m_pFirst = pDecalMark;

        // Queue for creation on the next BeginFrame
        pDecalMark->m_pNextToCreate = m_pFirstCreated;
        m_pFirstCreated = pDecalMark;

        return pDecalMark;
    }

    void ZDecalMarkController::Modify(ZDecalMarkController::ZDecalMark* pDecalMark, float fOpacity)
    {
        // Reference (PC 0x47CFE0):
        uint32_t lAlpha = static_cast<uint32_t>((1.0f - fOpacity) * 255.0f);
        if (lAlpha > 0xFF)
        {
            lAlpha = 0xFF;
        }
        const uint32_t lColor = (lAlpha << 24) | 0xFFFFFF;

        for (ZLink* pLink = pDecalMark->m_pLinks; pLink; pLink = pLink->m_pNext)
        {
            ZPrimAccessMesh* pMesh = pLink->m_pPrimAccessMesh;
            pMesh->Lock(2);
            const uint32_t lNumVertices = pMesh->GetNumVertices();
            uint32_t* pColors = static_cast<uint32_t*>(_alloca(sizeof(uint32_t) * lNumVertices));
            pMesh->GetColors(0, lNumVertices, pColors);
            memset(pColors, lColor, lNumVertices);
            pMesh->SetColors(0, lNumVertices, pColors);
            pMesh->Unlock();
        }
    }

    void ZDecalMarkController::Remove(ZDecalMarkController::ZDecalMark* pDecalMark)
    {
        if (!pDecalMark->m_bRemoved)
        {
            pDecalMark->m_bRemoved = true;
            pDecalMark->m_pNextToRemove = m_pFirstRemoved;
            m_pFirstRemoved = pDecalMark;
        }
    }

    void ZDecalMarkController::SetCallBack(ZDecalMarkController::ZDecalMark* pDecalMark, DECALCALLBACK pCallBackDecal, void* pCallBackData, uint32_t lCallBackValue)
    {
        ZASSERT(pCallBackDecal);

        if (!pDecalMark->m_CallBackDecal)
        {
            pDecalMark->m_CallBackDecal = pCallBackDecal;
            pDecalMark->m_pCallBackData = pCallBackData;
            pDecalMark->m_lCallBackValue = lCallBackValue;
            pDecalMark->m_pNextCallBack = m_pFirstCallBack;
            m_pFirstCallBack = pDecalMark;
        }
    }

    uint32_t ZDecalMarkController::GetAttachedObjects(ZBaseGeom* pBaseGeom, ZPrimHandle* pObjects, uint32_t lMaxNumObjects)
    {
        uint32_t lCount = 0;
        const int lGeomKey = static_cast<int>(reinterpret_cast<uintptr_t>(pBaseGeom));

        if (auto* pNode = m_BaseGeomToLink.Find(&lGeomKey))
        {
            for (ZLink* pLink = pNode->m_Value; pLink && lCount != lMaxNumObjects; pLink = pLink->m_pNextSameGeom)
            {
                pObjects[lCount] = pLink->m_pPrimAccessMesh->m_hPrim;
                ++lCount;
            }
        }

        return lCount;
    }

    void ZDecalMarkController::RemoveBaseGeom(ZBaseGeom* pBaseGeom)
    {
        if ((pBaseGeom->m_lControl & ZCRENDERATTACHED) == 0)
            return;

        if (pBaseGeom->IsDerivedFrom<ZLNKOBJ>())
        {
            RemoveBoneDecalBaseGeom(static_cast<ZLNKOBJ*>(pBaseGeom->m_pExtraGeom));
            return;
        }

        RecreateObjects(pBaseGeom);
        ZLink* pLink = RemoveLink(pBaseGeom);
        if (!pLink)
        {
            ZASSERT(false);
            pBaseGeom->SetControl(0, ZCRENDERATTACHED);
            return;
        }

        while (pLink)
        {
            ZLink* pNext = pLink->m_pNextSameGeom;
            ZASSERT(pLink->m_pBaseGeom == pBaseGeom);
            pLink->m_pPrimAccessMesh->Destroy();
            if (pLink->m_pStoredUV)
                ISysMem::Instance().Delete(pLink->m_pStoredUV);

            if (pLink->m_pPrev)
                pLink->m_pPrev->m_pNext = pLink->m_pNext;
            else
                pLink->m_pDecalMark->m_pLinks = pLink->m_pNext;
            if (pLink->m_pNext)
                pLink->m_pNext->m_pPrev = pLink->m_pPrev;

            m_Links.Remove(pLink);
            pLink = pNext;
        }

        pBaseGeom->SetControl(0, ZCRENDERATTACHED);
    }

    void ZDecalMarkController::RemoveAllDecals()
    {
        ZDecalMark* pDecalMark = m_pFirst;
        while (pDecalMark)
        {
            ZDecalMark* pNext = pDecalMark->m_pNext;
            RemoveDecal(pDecalMark);
            pDecalMark = pNext;
        }

        m_pFirst = nullptr;
        m_pFirstCallBack = nullptr;
        m_pFirstCreated = nullptr;
        m_pFirstRemoved = nullptr;

        m_DecalMarks.Reset();
        m_Links.Reset();
    }

    void ZDecalMarkController::Create(ZDecalMarkController::ZDecalMark* pDecalMark)
    {
        if (!ZCollisionBase::s_pCollisionBase)
            return;

        const float fMaxRadius = (std::max)(1.0f, (std::max)(pDecalMark->m_fRadius.x, pDecalMark->m_fRadius.y));
        const ZVector3 vExtents(fMaxRadius, fMaxRadius, fMaxRadius);
        ZMat3x3 mIdentity;
        mIdentity.Reset();
        ZBaseGeom* aGeoms[ZCollisionBase::MAX_GEOMS_NR];
        const uint32_t lNumGeoms = ZCollisionBase::s_pCollisionBase->GetGeomsInBox(
            aGeoms, aGeoms + ZCollisionBase::MAX_GEOMS_NR, GT_StdObjs,
            mIdentity.Get(), pDecalMark->m_vPosition.Get(), vExtents.Get(), 6, true, true, true);

        uint32_t lNumRegistered = 0;
        for (uint32_t i = 0; i < lNumGeoms && lNumRegistered < 8; ++i)
        {
            ZBaseGeom* pGeom = aGeoms[i];
            if (!pGeom || pGeom->IsDerivedFrom<ZLNKOBJ>())
                continue;

            ZVector3 vLocalPos = pDecalMark->m_vPosition;
            ZVector3 vLocalDir = pDecalMark->m_vDirection;
            pGeom->GetLocalPoint(vLocalPos);
            pGeom->GetLocalVect(vLocalDir);
            vnorm(vLocalDir.Get());
            ZPrimAccessMesh* pMesh = CreateDecal(pGeom, pDecalMark->m_lSourcePrim,
                vLocalPos.Get(), vLocalDir.Get(), fMaxRadius, pDecalMark->m_fRotation,
                &pDecalMark->m_fExtraTextureSize.x);
            if (!pMesh)
                continue;
            if (!RegisterDecalMesh(pDecalMark, pGeom, pMesh))
                pMesh->Destroy();
            else
                ++lNumRegistered;
        }
    }

    ZPrimAccessMesh* ZDecalMarkController::CreateDecal(ZBaseGeom* pBaseGeom, uint32_t lSourcePrim, const float* pvPosition, const float* pvDirection, float fRadius, float fRotation, const float* pvExtraTextureSize)
    {
        const ZPrimHandle hSource{ pBaseGeom->m_lPrim };
        auto* pSourceAccess = ZPrimAccess::Create(hSource);
        auto* pSourceMesh = pSourceAccess ? dynamic_cast<ZPrimAccessMesh*>(pSourceAccess) : nullptr;
        if (!pSourceMesh)
        {
            if (pSourceAccess)
                pSourceAccess->Destroy();
            return nullptr;
        }

        ZMat3x3 mBasis;
        createmat(mBasis.Get(), pvDirection, nullptr);
        ZMat3x3 mRotation;
        mrotaxis(mRotation.Get(), fRotation + 90.0f, mBasis.data[0], mBasis.data[1], mBasis.data[2]);
        mBasis *= mRotation;

        struct Vertex { ZVector3 p; ZVector3 n; ZVector2 uv; };
        Vertex aVertices[96];
        uint16_t aIndices[96];
        uint32_t lVertices = 0;
        uint32_t lTriangles = 0;
        const float fDepth = fRadius;
        const float fU = pvExtraTextureSize[0] + 1.0f;
        const float fV = pvExtraTextureSize[1] + 1.0f;

        pSourceMesh->Lock(ZPrimAccess::LF_READONLY);
        const uint32_t lSourceTriangles = pSourceMesh->GetNumTriangles();
        const uint16_t* pIndices = pSourceMesh->GetIndicesConst();
        for (uint32_t triangle = 0; triangle < lSourceTriangles && lTriangles < 32; ++triangle)
        {
            Vertex polygon[8];
            uint32_t lPolygon = 3;
            for (uint32_t corner = 0; corner < 3; ++corner)
            {
                float position[3];
                pSourceMesh->GetPositions(pIndices[3 * triangle + 2 + corner], 1, position);
                polygon[corner].p = position;
            }
            ZVector3 edge0 = polygon[1].p - polygon[0].p;
            ZVector3 edge1 = polygon[2].p - polygon[0].p;
            vcross(polygon[0].n.Get(), edge1.Get(), edge0.Get());
            if (vnorm(polygon[0].n.Get()) < 0.0001f || vdot(polygon[0].n.Get(), pvDirection) > -0.2f)
                continue;
            polygon[1].n = polygon[0].n;
            polygon[2].n = polygon[0].n;

            for (int axis = 0; axis < 3 && lPolygon >= 3; ++axis)
            {
                for (int side = -1; side <= 1 && lPolygon >= 3; side += 2)
                {
                    Vertex clipped[8];
                    uint32_t lClipped = 0;
                    for (uint32_t j = 0; j < lPolygon; ++j)
                    {
                        const Vertex& a = polygon[j];
                        const Vertex& b = polygon[(j + 1) % lPolygon];
                        ZVector3 localA = a.p - ZVector3(pvPosition);
                        ZVector3 localB = b.p - ZVector3(pvPosition);
                        vmtmul(localA.Get(), mBasis.Get());
                        vmtmul(localB.Get(), mBasis.Get());
                        const float da = side * localA.Get()[axis] - fDepth;
                        const float db = side * localB.Get()[axis] - fDepth;
                        const bool ina = da <= 0.0f;
                        const bool inb = db <= 0.0f;
                        if (ina)
                            clipped[lClipped++] = a;
                        if (ina != inb)
                        {
                            const float t = da / (da - db);
                            clipped[lClipped].p = a.p + (b.p - a.p) * t;
                            clipped[lClipped].n = a.n;
                            ++lClipped;
                        }
                    }
                    memcpy(polygon, clipped, sizeof(Vertex) * lClipped);
                    lPolygon = lClipped;
                }
            }
            for (uint32_t j = 1; j + 1 < lPolygon && lTriangles < 32; ++j)
            {
                const Vertex* corners[3] = { &polygon[0], &polygon[j], &polygon[j + 1] };
                for (const Vertex* corner : corners)
                {
                    ZVector3 local = corner->p - ZVector3(pvPosition);
                    vmtmul(local.Get(), mBasis.Get());
                    aVertices[lVertices] = *corner;
                    aVertices[lVertices].uv = { local.x / (2.0f * fRadius) + 0.5f,
                        local.y / (2.0f * fRadius) + 0.5f };
                    aVertices[lVertices].uv.x *= fU;
                    aVertices[lVertices].uv.y *= fV;
                    aIndices[3 * lTriangles + (lVertices % 3)] = static_cast<uint16_t>(lVertices++);
                }
                ++lTriangles;
            }
        }
        pSourceMesh->Unlock();
        pSourceAccess->Destroy();
        if (!lTriangles)
            return nullptr;

        const ZPrimHandle hTarget{ g_pRenderDll->m_pPrimControl->GetSubPrim(lSourcePrim, 0) };
        auto* pTargetAccess = ZPrimAccess::Create(hTarget);
        auto* pTargetMesh = pTargetAccess ? dynamic_cast<ZPrimAccessMesh*>(pTargetAccess) : nullptr;
        if (!pTargetMesh)
        {
            if (pTargetAccess)
                pTargetAccess->Destroy();
            return nullptr;
        }
        auto* pResult = pTargetMesh->CreateEditable(lTriangles, lVertices);
        if (!pResult)
        {
            pTargetAccess->Destroy();
            return nullptr;
        }
        pResult->Lock(ZPrimAccess::LF_WRITEONLY);
        for (uint32_t i = 0; i < lVertices; ++i)
        {
            pResult->SetPositions(i, 1, aVertices[i].p.Get());
            pResult->SetNormals(i, 1, aVertices[i].n.Get());
            pResult->SetTexCoords(i, 1, &aVertices[i].uv.x);
            const uint32_t color = 0xFFFFFFFFu;
            pResult->SetColors(i, 1, &color);
        }
        pResult->SetTriangles(0, lTriangles, aIndices);
        pResult->Unlock();
        pTargetAccess->Destroy();
        return pResult;
    }

    bool ZDecalMarkController::RegisterDecalMesh(ZDecalMarkController::ZDecalMark* pDecalMark, ZBaseGeom* pBaseGeom, ZPrimAccessMesh* pMesh)
    {
        const int lGeomKey = static_cast<int>(reinterpret_cast<uintptr_t>(pBaseGeom));

        ZLink* pSameGeomLink = nullptr;
        if (auto* pNode = m_BaseGeomToLink.Find(&lGeomKey))
        {
            pSameGeomLink = pNode->m_Value;
        }

        if (m_Links.Count() == m_Links.TotalNrEntries())
        {
            return false;
        }

        ZLink* pLink = m_Links.Add();
        pLink->m_pPrev = nullptr;
        pLink->m_pNext = nullptr;
        pLink->m_pPrevSameGeom = nullptr;
        pLink->m_pNextSameGeom = nullptr;
        pLink->m_pBaseGeom = pBaseGeom;
        pLink->m_pDecalMark = pDecalMark;
        pLink->m_pPrimAccessMesh = pMesh;

        if (pDecalMark->m_bStoreUV)
        {
            const uint32_t lNumVertices = pMesh->GetNumVertices();
            pLink->m_pStoredUV = static_cast<float*>(ISysMem::Instance().New(
                RENDERPRIMACCESS_MEM, sizeof(float) * 2 * lNumVertices));
            if (pLink->m_pStoredUV)
            {
                pMesh->Lock(ZPrimAccess::LF_READONLY);
                pMesh->GetTexCoords(0, lNumVertices, pLink->m_pStoredUV);
                pMesh->Unlock();
            }
        }
        else
        {
            pLink->m_pStoredUV = nullptr;
        }

        // Link into the mark's list
        ZLink* pFirst = pDecalMark->m_pLinks;
        if (pFirst)
        {
            pLink->m_pNext = pFirst->m_pNext;
            pLink->m_pPrev = pFirst;
            if (pFirst->m_pNext)
            {
                pFirst->m_pNext->m_pPrev = pLink;
            }
            pFirst->m_pNext = pLink;
        }
        else
        {
            pDecalMark->m_pLinks = pLink;
        }

        // Link into the per-geom chain or register as the first link of the geom
        if (pSameGeomLink)
        {
            pLink->m_pNextSameGeom = pSameGeomLink->m_pNextSameGeom;
            pLink->m_pPrevSameGeom = pSameGeomLink;
            if (pSameGeomLink->m_pNextSameGeom)
            {
                pSameGeomLink->m_pNextSameGeom->m_pPrevSameGeom = pLink;
            }
            pSameGeomLink->m_pNextSameGeom = pLink;
        }
        else
        {
            m_BaseGeomToLink.Put(&lGeomKey, pLink, false);
        }

        ZASSERT(pLink->m_pBaseGeom == pBaseGeom);

        pBaseGeom->SetControl(ZCRENDERATTACHED, 0);

        RecreateObjects(pBaseGeom);

        return true;
    }

    void ZDecalMarkController::RemoveDecal(ZDecalMarkController::ZDecalMark* pDecalMark)
    {
        // Unlink from the active marks list
        if (pDecalMark->m_pPrev)
        {
            pDecalMark->m_pPrev->m_pNext = pDecalMark->m_pNext;
        }
        else
        {
            m_pFirst = pDecalMark->m_pNext;
        }
        if (pDecalMark->m_pNext)
        {
            pDecalMark->m_pNext->m_pPrev = pDecalMark->m_pPrev;
        }

        // Release all links of the mark
        ZLink* pLink = pDecalMark->m_pLinks;
        pDecalMark->m_pLinks = nullptr;
        while (pLink)
        {
            ZLink* pNext = pLink->m_pNext;

            pLink->m_pPrimAccessMesh->Destroy();

            if (pLink->m_pStoredUV)
            {
                ISysMem::Instance().Delete(pLink->m_pStoredUV);
            }

            RecreateObjects(pLink->m_pBaseGeom);

            // Unlink from the per-geom chain
            const int lGeomKey = static_cast<int>(reinterpret_cast<uintptr_t>(pLink->m_pBaseGeom));
            if (pLink->m_pPrevSameGeom)
            {
                pLink->m_pPrevSameGeom->m_pNextSameGeom = pLink->m_pNextSameGeom;
                if (pLink->m_pNextSameGeom)
                {
                    pLink->m_pNextSameGeom->m_pPrevSameGeom = pLink->m_pPrevSameGeom;
                }
            }
            else if (pLink->m_pNextSameGeom)
            {
                m_BaseGeomToLink.Put(&lGeomKey, pLink->m_pNextSameGeom, false);
                pLink->m_pNextSameGeom->m_pPrevSameGeom = nullptr;
            }
            else
            {
                m_BaseGeomToLink.Remove(&lGeomKey);
                pLink->m_pBaseGeom->SetControl(0, ZCRENDERATTACHED);
            }

            memset(pLink, 0x5A, sizeof(ZLink));
            m_Links.Remove(pLink);
            pLink = pNext;
        }

        memset(pDecalMark, 0x4F, sizeof(ZDecalMark));
        m_DecalMarks.Remove(pDecalMark);
    }

    ZDecalMarkController::ZLink* ZDecalMarkController::RemoveLink(ZBaseGeom* pBaseGeom)
    {
        const int lGeomKey = static_cast<int>(reinterpret_cast<uintptr_t>(pBaseGeom));

        if (auto* pNode = m_BaseGeomToLink.Find(&lGeomKey))
        {
            ZLink* pLink = pNode->m_Value;
            if (pLink)
            {
                m_BaseGeomToLink.Remove(&lGeomKey);
            }
            return pLink;
        }

        return nullptr;
    }

    void ZDecalMarkController::RecreateObjects(ZBaseGeom* pBaseGeom)
    {
        pBaseGeom->SetAttachUpdate();
    }
}
