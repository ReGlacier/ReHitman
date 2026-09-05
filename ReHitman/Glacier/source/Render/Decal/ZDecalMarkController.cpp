#include <Glacier/Render/Decal/ZDecalMarkController.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/Geom/GeomControlMasks.h>
#include <cstring>


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
        // TODO: Finish this place after ZPrimAccessMesh will be reversed
        // Reference (PC 0x47CFE0):
        // uint32_t lAlpha = static_cast<uint32_t>((1.0f - fOpacity) * 255.0f);
        // if (lAlpha > 0xFF)
        // {
        //     lAlpha = 0xFF;
        // }
        // const uint32_t lColor = (lAlpha << 24) | 0xFFFFFF;
        //
        // for (ZLink* pLink = pDecalMark->m_pLinks; pLink; pLink = pLink->m_pNext)
        // {
        //     ZPrimAccessMesh* pMesh = pLink->m_pPrimAccessMesh;
        //     pMesh->Lock(2);
        //     const uint32_t lNumVertices = pMesh->GetNumVertices();
        //     uint32_t* pColors = static_cast<uint32_t*>(_alloca(sizeof(uint32_t) * lNumVertices));
        //     pMesh->GetVertexData(0, lNumVertices, pColors);
        //     memset32(pColors, lColor, lNumVertices);
        //     pMesh->SetVertexData(0, lNumVertices, pColors);
        //     pMesh->Unlock();
        // }
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
                // TODO: Finish this place after ZPrimAccessMesh will be reversed
                // pObjects[lCount] = pLink->m_pPrimAccessMesh->m_hPrim;
                ++lCount;
            }
        }

        return lCount;
    }

    void ZDecalMarkController::RemoveBaseGeom(ZBaseGeom* pBaseGeom)
    {
        // TODO: Finish this place after ZPrimAccessMesh and IDraw render entries will be reversed
        // Reference (PC 0x47DC50):
        // if ((pBaseGeom->m_lControl & ZCRENDERATTACHED) == 0)
        // {
        //     return;
        // }
        //
        // if (pBaseGeom->IsDerivedFrom<ZLNKOBJ>()) // via m_pExtraGeom class id check
        // {
        //     RemoveBoneDecalBaseGeom((ZLNKOBJ*)pBaseGeom->m_pExtraGeom);
        //     return;
        // }
        //
        // RecreateObjects(pBaseGeom);
        //
        // ZLink* pLink = RemoveLink(pBaseGeom);
        // if (!pLink)
        // {
        //     ZASSERT(false); // geom has ZCRENDERATTACHED but no registered links
        // }
        // while (pLink)
        // {
        //     ZLink* pNextSameGeom = pLink->m_pNextSameGeom;
        //     ZASSERT(pLink->m_pBaseGeom == pBaseGeom);
        //     pLink->m_pPrimAccessMesh->Destroy();
        //     if (pLink->m_pStoredUV)
        //     {
        //         operator delete(pLink->m_pStoredUV);
        //     }
        //
        //     // Unlink from the mark's list
        //     if (pLink->m_pPrev)
        //     {
        //         pLink->m_pPrev->m_pNext = pLink->m_pNext;
        //         if (pLink->m_pNext)
        //         {
        //             pLink->m_pNext->m_pPrev = pLink->m_pPrev;
        //         }
        //     }
        //     else
        //     {
        //         pLink->m_pDecalMark->m_pLinks = pLink->m_pNext;
        //         if (pLink->m_pNext)
        //         {
        //             pLink->m_pNext->m_pPrev = nullptr;
        //         }
        //     }
        //
        //     m_Links.Remove(pLink);
        //     pLink = pNextSameGeom;
        // }
        //
        // pBaseGeom->SetControl(0, ZCRENDERATTACHED);
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
        // TODO: Finish this place after ZCollisionBase will be reversed
        // Reference (PC 0x47E2E0):
        // float fMaxRadius = std::max(std::max(pDecalMark->m_fRadius.x, 1.0f), std::max(pDecalMark->m_fRadius.y, 1.0f));
        // ZVector3 vExtents(fMaxRadius, fMaxRadius, fMaxRadius);
        //
        // Query ZCollisionBase::s_pCollisionBase for the geoms inside the box around m_vPosition,
        // then for each geom (skip ZLNKOBJ-derived):
        //     ZVector3 vLocalPos = pDecalMark->m_vPosition;
        //     ZVector3 vLocalDir = pDecalMark->m_vDirection;
        //     pGeom->GetLocalPoint(vLocalPos);
        //     pGeom->GetLocalVect(vLocalDir);
        //     vnorm(vLocalDir.Get(), vLocalDir.Get());
        //
        //     ZPrimAccessMesh* pMesh = CreateDecal(pGeom, pDecalMark->m_lSourcePrim, vLocalPos.Get(), vLocalDir.Get(), fMaxRadius, pDecalMark->m_fRotation, pDecalMark->m_fExtraTextureSize.Get());
        //     if (pMesh)
        //     {
        //         if (!RegisterDecalMesh(pDecalMark, pGeom, pMesh))
        //         {
        //             pMesh->Destroy();
        //         }
        //         else if (++lNumRegistered == 8)
        //         {
        //             return; // at most 8 meshes per mark
        //         }
        //     }
    }

    ZPrimAccessMesh* ZDecalMarkController::CreateDecal(ZBaseGeom* pBaseGeom, uint32_t lSourcePrim, const float* pvPosition, const float* pvDirection, float fRadius, float fRotation, const float* pvExtraTextureSize)
    {
        // TODO: Finish this place after ZPrimAccessMesh will be reversed
        // Reference (PC 0x47D150): clips the source prim polygons against the decal
        // box (MakeInclusivePolygon/ClipLinePlane), projects UVs with ZUVProject and
        // bakes the result into a new dynamic prim created via ZPrimAccess::Create
        // (at most 32 triangles per decal). Returns nullptr when no decal mesh was built.
        return nullptr;
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
            // TODO: Finish this place after ZSysMem::AllocateSpecificMem will be reversed
            // const uint32_t lPrevColor = SetMemColor(0xFF);
            // RenderMemAllocator::SetFileAndLine(__FILE__, __LINE__);
            // const uint32_t lNumVertices = pMesh->GetNumVertices();
            // pLink->m_pStoredUV = static_cast<float*>(ZSysMem::AllocateSpecificMem(sizeof(float) * 2 * lNumVertices, RENDERPRIMACCESS_MEM));
            // if (pLink->m_pStoredUV)
            // {
            //     pMesh->Lock(1);
            //     pMesh->GetVertexData(0, lNumVertices, pLink->m_pStoredUV);
            //     pMesh->Unlock();
            // }
            // SetMemColor(lPrevColor);
            pLink->m_pStoredUV = nullptr;
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

        // TODO: Finish this place after IDraw render entries will be reversed (inlined RecreateObjects)
        // if (pBaseGeom->m_lDrawId)
        // {
        //     IDraw::Instance()-><render entry>[pBaseGeom->m_lDrawId].m_lFlags |= 0x400;
        // }

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

            // TODO: Finish this place after ZPrimAccessMesh will be reversed
            // pLink->m_pPrimAccessMesh->Destroy();

            if (pLink->m_pStoredUV)
            {
                operator delete(pLink->m_pStoredUV);
            }

            // TODO: Finish this place after IDraw render entries will be reversed
            // if (pLink->m_pBaseGeom->m_lDrawId)
            // {
            //     IDraw::Instance()-><render entry>[pLink->m_pBaseGeom->m_lDrawId].m_lFlags |= 0x400;
            // }

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
        // TODO: Finish this place after IDraw render entries will be reversed
        // Reference (XBOX 0x82134270):
        // if (pBaseGeom->m_lDrawId)
        // {
        //     IDraw::Instance()-><render entry>[pBaseGeom->m_lDrawId].m_lFlags |= 0x400;
        // }
    }
}
