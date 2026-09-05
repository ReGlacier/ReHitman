#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/Prim/SHandleTableEntry.h>
#include <Glacier/Render/Prim/SPrimObjectHeader.h>
#include <Glacier/Render/Prim/SPrimHeaderStripWaterPatch.h>
#include <Glacier/Render/Prim/SPrimVariant.h>
#include <Glacier/Render/Prim/ZPrimAccessMesh.h>
#include <Glacier/Render/Prim/SPrimLightEnvironment.h>
#include <Glacier/Render/Prim/SPrimLightSpotSquare.h>
#include <Glacier/Render/Prim/SPrimObjectScatter.h>
#include <Glacier/Render/Prim/SPrimSpritesArray.h>
#include <Glacier/Render/Prim/SPrimHeaderStrip.h>
#include <Glacier/Render/Prim/SPrimLightSpot.h>
#include <Glacier/Render/Prim/SPrimLightOmni.h>
#include <Glacier/Render/Prim/SPrimLight.h>
#include <Glacier/Render/Prim/ZPrimHandle.h>
#include <Glacier/Render/Prim/SPrimObject.h>
#include <Glacier/Render/Prim/SPrimHeader.h>
#include <Glacier/Render/Prim/SPrimStrips.h>
#include <Glacier/Render/Prim/SPrimMesh.h>
#include <Glacier/Render/Prim/ELightType.h>
#include <Glacier/Render/Prim/EPrimType.h>
#include <Glacier/Render/Prim/SPrims.h>
#include <Glacier/Render/Sprite/SSpriteArrayElementUV.h>
#include <Glacier/Render/Sprite/SSpriteArray.h>
#include <Glacier/Render/Draw/ZRenderDraw.h>
#include <Glacier/Render/Entry/ZRenderEntryGeom.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/ZTextureBase.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/ZSTL/StringUtils.h>
#include <Glacier/ZUniAssert.h>
#include <cstdio>
#include <cstring>


namespace Glacier
{
    ZPrimControlBase::~ZPrimControlBase()
    {
        // Do nothing
    }

    ZPrimControlBase::ZPrimControlBase(bool bUnusedSmth)
    {
        // Do nothing
    }

    void ZPrimControlBase::GetPrimExtraData(uint32_t lPrim, uint32_t lFlag, void* pData, uint32_t lCount)
    {
        // Do nothing
    }

    int ZPrimControlBase::ShouldUseBoundForLightCheck(uint32_t lPrim)
    {
        const SPrimObject* pObject = ZPrimHandle{lPrim};
        return (pObject->lType == EPrimType::PTOBJECTHEADER) ? (pObject->lProperties & 1) : false; // ->lProperites? XYAXIS? Need check in future
    }

    uint8_t ZPrimControlBase::GetPrimPackType(uint32_t lPrim)
    {
        const auto* pData = GetPrimData(lPrim);
        ZASSERT(pData);

        return reinterpret_cast<const SPrimHeader*>(pData)->lPackType;
    }

    const void* ZPrimControlBase::GetPrimData(uint32_t lPrim) const
    {
        if (!lPrim)
        {
            return nullptr;
        }

        ZASSERT(g_pRenderDll && g_pRenderDll->m_pPrimBuffer);
        return g_apPrimHandleToPointerTable[lPrim];
    }

    uint32_t ZPrimControlBase::GetPrimOffset(const void* ptr)
    {
        ZASSERT(false);
        return 0u;
    }

    bool ZPrimControlBase::CheckInPrimBuffer(const void* ptr)
    {
        return ptr >= g_pRenderDll->m_pPrimBuffer && ptr < g_pRenderDll->m_pPrimBufferEnd;
    }

    bool ZPrimControlBase::IsPrimUnique(uint32_t lPrim)
    {
        return lPrim >= g_lPrimHandleToPointerCount;
    }

    bool ZPrimControlBase::CheckPointInsidePrim(uint32_t lPrim, const ZVector3& vPoint, float fThreshold)
    {
        if (!lPrim)
            return true;

        uint32_t lCurrentPrim = lPrim;
        while (lCurrentPrim)
        {
            const SPrims* pPrim = static_cast<const SPrims*>(GetPrimData(lCurrentPrim));

            if (pPrim->lType == PTSTRIP || pPrim->lType == PTDOT3STRIP)
            {
                const auto* pStrip = static_cast<const SPrimStrips*>(pPrim);
                if (pStrip->lInsideCheckData)
                {
                    const uint32_t* pInsideCheck = static_cast<const uint32_t*>(GetPrimData(pStrip->lInsideCheckData));
                    const uint32_t lPlaneCount = pInsideCheck[0];
                    const float* pPlane = reinterpret_cast<const float*>(pInsideCheck + 1);

                    for (uint32_t i = 0; i < lPlaneCount; ++i)
                    {
                        if (pPlane[0] * vPoint.x + pPlane[1] * vPoint.y + pPlane[2] * vPoint.z + pPlane[3] > -fThreshold)
                            return false;

                        pPlane += 4;
                    }
                }
            }
            else if (pPrim->lType == PTOBJECTHEADER)
            {
                const auto* pObjectHeader = static_cast<const SPrimObjectHeader*>(pPrim);

                ZPrimHandle hObjectTable { pObjectHeader->lObjectTable };
                const uint32_t* pObjectTable = hObjectTable;

                for (uint32_t i = 0; i < pObjectHeader->lNumObjects; ++i)
                {
                    const uint32_t lObject = pObjectTable[i];
                    const auto* pObject = static_cast<const SPrims*>(GetPrimData(lObject));
                    if (pObject->lType != PTMESH)
                        continue;

                    ZPrimHandle hObject { lObject };
                    auto* pMesh = static_cast<ZPrimAccessMesh*>(ZPrimAccess::Create(hObject));
                    if (!pMesh)
                        continue;

                    pMesh->Lock(ZPrimAccess::LF_READONLY);

                    const uint32_t lNumTriangles = pMesh->GetNumTriangles();

                    for (uint32_t lTriangleBatchStart = 0; lTriangleBatchStart < lNumTriangles; lTriangleBatchStart += 32)
                    {
                        uint32_t lBatchCount = lNumTriangles - lTriangleBatchStart;
                        if (lBatchCount > 32)
                            lBatchCount = 32;

                        float aVertices[32 * 9];
                        pMesh->GetTriangles(lTriangleBatchStart, lBatchCount, aVertices);

                        for (uint32_t t = 0; t < lBatchCount; ++t)
                        {
                            const float* pV0 = aVertices + t * 9;
                            const float* pV1 = pV0 + 3;
                            const float* pV2 = pV0 + 6;

                            float vE0[3];
                            float vE1[3];
                            vsub(vE0, pV0, pV2);
                            vsub(vE1, pV0, pV1);

                            float vNormal[3];
                            vcross(vNormal, vE0, vE1);
                            vnorm(vNormal);

                            float vToPoint[3];
                            vsub(vToPoint, vPoint, pV0);

                            if (vdot(vNormal, vToPoint) > fThreshold)
                            {
                                pMesh->Unlock();
                                pMesh->Destroy();
                                return false;
                            }
                        }
                    }

                    pMesh->Unlock();
                    pMesh->Destroy();
                }
            }

            lCurrentPrim = pPrim->lNextPrim;
        }

        return true;
    }

    bool ZPrimControlBase::CheckBoxInsidePrim(uint32_t lPrim, const float* pvBoxP1, const float* pvBoxP2, const float* pvThreshold)
    {
        float fMaxThreshold = pvThreshold[0];
        if (fMaxThreshold < pvThreshold[1])
            fMaxThreshold = pvThreshold[1];
        if (fMaxThreshold < pvThreshold[2])
            fMaxThreshold = pvThreshold[2];

        if (!lPrim)
            return true;

        uint32_t lCurrentPrim = lPrim;
        while (lCurrentPrim)
        {
            const SPrims* pPrim = static_cast<const SPrims*>(GetPrimData(lCurrentPrim));

            if (pPrim->lType == PTSTRIP || pPrim->lType == PTDOT3STRIP)
            {
                const auto* pStrip = static_cast<const SPrimStrips*>(pPrim);
                if (pStrip->lInsideCheckData)
                {
                    const uint32_t* pInsideCheck = static_cast<const uint32_t*>(GetPrimData(pStrip->lInsideCheckData));
                    const uint32_t lPlaneCount = pInsideCheck[0];
                    const float* pPlane = reinterpret_cast<const float*>(pInsideCheck + 1);

                    for (uint32_t i = 0; i < lPlaneCount; ++i)
                    {
                        const float fDistance = pvBoxP2[0] * pPlane[0] + pPlane[1] * pvBoxP2[1] + pvBoxP2[2] * pPlane[2] + pPlane[3];
                        if (fDistance > fMaxThreshold)
                            return false;

                        const float fProjRadius = fabsf((pvBoxP1[0] * pPlane[0] + pvBoxP1[1] * pPlane[1] + pvBoxP1[2] * pPlane[2]) * pvThreshold[2])
                                                + fabsf((pvBoxP1[3] * pPlane[0] + pvBoxP1[4] * pPlane[1] + pvBoxP1[5] * pPlane[2]) * pvThreshold[1])
                                                + fabsf((pvBoxP1[6] * pPlane[0] + pvBoxP1[7] * pPlane[1] + pvBoxP1[8] * pPlane[2]) * pvThreshold[0]);
                        if (fProjRadius < fDistance)
                            return false;

                        pPlane += 4;
                    }
                }
            }
            else if (pPrim->lType == PTOBJECTHEADER)
            {
                const auto* pObjectHeader = static_cast<const SPrimObjectHeader*>(pPrim);

                ZPrimHandle hObjectTable { pObjectHeader->lObjectTable };
                const uint32_t* pObjectTable = hObjectTable;

                for (uint32_t i = 0; i < pObjectHeader->lNumObjects; ++i)
                {
                    const uint32_t lObject = pObjectTable[i];
                    const auto* pObject = static_cast<const SPrims*>(GetPrimData(lObject));
                    if (pObject->lType != PTMESH)
                        continue;

                    ZPrimHandle hObject { lObject };
                    auto* pMesh = static_cast<ZPrimAccessMesh*>(ZPrimAccess::Create(hObject));
                    if (!pMesh)
                        continue;

                    pMesh->Lock(ZPrimAccess::LF_READONLY);

                    const uint32_t lNumTriangles = pMesh->GetNumTriangles();

                    for (uint32_t lTriangleBatchStart = 0; lTriangleBatchStart < lNumTriangles; lTriangleBatchStart += 32)
                    {
                        uint32_t lBatchCount = lNumTriangles - lTriangleBatchStart;
                        if (lBatchCount > 32)
                            lBatchCount = 32;

                        float aVertices[32 * 9];
                        pMesh->GetTriangles(lTriangleBatchStart, lBatchCount, aVertices);

                        for (uint32_t t = 0; t < lBatchCount; ++t)
                        {
                            const float* pV0 = aVertices + t * 9;
                            const float* pV1 = pV0 + 3;
                            const float* pV2 = pV0 + 6;

                            float vE0[3];
                            float vE1[3];
                            vsub(vE0, pV0, pV2);
                            vsub(vE1, pV0, pV1);

                            float vNormal[3];
                            vcross(vNormal, vE0, vE1);
                            vnorm(vNormal);

                            float vToBox[3];
                            vsub(vToBox, pvBoxP2, pV0);

                            const float fDistance = vdot(vToBox, vNormal);
                            if (fDistance > fMaxThreshold)
                            {
                                pMesh->Unlock();
                                pMesh->Destroy();
                                return false;
                            }

                            TransformRootVector(vNormal, pvBoxP1);

                            const float fProjRadius = fabsf(vNormal[2] * pvThreshold[2])
                                                    + fabsf(vNormal[1] * pvThreshold[1])
                                                    + fabsf(vNormal[0] * pvThreshold[0]);
                            if (fProjRadius < fDistance)
                            {
                                pMesh->Unlock();
                                pMesh->Destroy();
                                return false;
                            }
                        }
                    }

                    pMesh->Unlock();
                    pMesh->Destroy();
                }
            }

            lCurrentPrim = pPrim->lNextPrim;
        }

        return true;
    }

    uint32_t ZPrimControlBase::AddPrimDataUnique(const void* pData, uint32_t lDataSize, bool bFront)
    {
        if (!g_pRenderDll->m_pPrimBuffer)
        {
            ZASSERT(false);
            return 0;
        }

        char* pCurrentPrimBuffer = g_pRenderDll->m_pCurrentPrimBuffer;
        const uint32_t lFreeSpace = static_cast<uint32_t>(g_pRenderDll->m_pCurrentPrimBufferBack - pCurrentPrimBuffer);
        const bool bBufferOverflow = lFreeSpace < lDataSize;
        const bool bHandlesOverflow = g_iCurrentDynamicPrimBuffersCount > g_lPrimHandleToPointerFreeBack;

        if (bBufferOverflow || bHandlesOverflow)
        {
            if (!bBufferOverflow)
            {
                printf("ERROR: Dynamic prim buffer is out of handles! Max of %d has been reached.\n", g_iCurrentDynamicPrimBuffersCount);
            }
            else
            {
                printf("ERROR: Dynamic prim buffer too small! (requested datasize: %d, CpB: %x, PbE: %x)\n",
                       lDataSize,
                       reinterpret_cast<uintptr_t>(pCurrentPrimBuffer),
                       reinterpret_cast<uintptr_t>(g_pRenderDll->m_pPrimBufferEnd));
            }
            ZASSERT(false);
        }

        uint32_t lNewPrimIdx;
        void* pPrimData;

        if (bFront)
        {
            pPrimData = g_pRenderDll->m_pCurrentPrimBuffer;
            memcpy(pPrimData, pData, lDataSize);
            g_pRenderDll->m_pCurrentPrimBuffer += lDataSize;
            lNewPrimIdx = g_iCurrentDynamicPrimBuffersCount++;
        }
        else
        {
            g_pRenderDll->m_pCurrentPrimBufferBack -= lDataSize;
            pPrimData = g_pRenderDll->m_pCurrentPrimBufferBack;
            memcpy(pPrimData, pData, lDataSize);
            lNewPrimIdx = g_lPrimHandleToPointerFreeBack--;
        }

        g_apPrimHandleToPointerTable[lNewPrimIdx] = pPrimData;
        return lNewPrimIdx;
    }

    void ZPrimControlBase::FreePrimData(uint32_t lPrim)
    {
        if (!lPrim)
        {
            return;
        }

        if (g_pPrimHandleTable)
        {
            auto* pToRelease = &g_pPrimHandleTable[lPrim];
            ZASSERT(g_pPrimHandleTable[lPrim].lRefCount);

            if (!g_pPrimHandleTable[lPrim].lRefCount)
            {
                ZASSERT(false && "Somebody triyng to release released prim, watafaaa");
                pToRelease = &g_pPrimHandleTable[0]; // Zero prim is always trash
            }

            --pToRelease->lRefCount;
        }
        else
        {
            ZASSERT(g_lPrimToFreeCount < MAX_NUM_FREEPRIM);

            g_lPrimToFreeList[g_lPrimToFreeCount] = lPrim;
            ++g_lPrimToFreeCount;
        }
    }

    void ZPrimControlBase::GetPrimInfoString(char* pInfo, uint32_t lInfoMaxSize, uint32_t lPrim)
    {
        pInfo = nullptr;
    }

    uint32_t ZPrimControlBase::CountSubPrims(uint32_t lPrim)
    {
        uint32_t lCount = 0;
        for (uint32_t lCurrentPrim = lPrim; lCurrentPrim; ++lCount)
        {
            const auto* pData = static_cast<const SPrims*>(GetPrimData(lCurrentPrim));
            lCurrentPrim = pData->lNextPrim;
        }
        return lCount;
    }

    uint32_t ZPrimControlBase::GetPrimType(uint32_t lPrim)
    {
        return reinterpret_cast<const SPrimObject*>(GetPrimData(lPrim))->lType;
    }

    bool ZPrimControlBase::IsRigidBones(uint32_t lPrim)
    {
        return false;
    }

    uint32_t ZPrimControlBase::GetPrimSize(const SPrims* lPrim)
    {
        return 0u;
    }

    uint32_t ZPrimControlBase::GetSubPrim(uint32_t lPrim, uint32_t lSubPrimNumber)
    {
        if (!lPrim)
        {
            return 0;
        }

        const SPrimObject* pHeader = reinterpret_cast<const SPrimObject*>(GetPrimData(lPrim));
        if (!pHeader)
        {
            return 0;
        }

        if (pHeader->lType == EPrimType::PTOBJECTHEADER)
        {
            const auto* pObjectHeader = reinterpret_cast<const SPrimObjectHeader*>(pHeader);
            if (lSubPrimNumber >= pObjectHeader->lNumObjects)
                return 0;

            return reinterpret_cast<const uint32_t*>(GetPrimData(pObjectHeader->lObjectTable))[lSubPrimNumber];
        }

        // Otherwise
        uint32_t lCounter = lSubPrimNumber;
        uint32_t lNextPrim = lPrim;

        while (true)
        {
            const SPrimObject* pObject = reinterpret_cast<const SPrimObject*>(GetPrimData(lSubPrimNumber));
            if (!lCounter--)
                break;

            lNextPrim = pObject->lNextPrim;
            if (!lNextPrim)
                return 0;
        }

        return lNextPrim;
    }

    uint32_t ZPrimControlBase::GetSubPrims(uint32_t lPrim, uint32_t* pSubPrims, uint32_t lMaxSubPrims)
    {
        uint32_t lTotalCount = 0;
        uint32_t lCurrentPrim = lPrim;

        for (; lCurrentPrim; ++lTotalCount)
        {
            if (lTotalCount >= lMaxSubPrims)
                break;

            const auto* pCurrentPrim = GetPrimitive<const SPrims>(lCurrentPrim);
            pSubPrims[lTotalCount] = lCurrentPrim;
            lCurrentPrim = pCurrentPrim->lNextPrim;
        }

        return lTotalCount;
    }

    uint32_t ZPrimControlBase::CopyPrim(uint32_t lPrim, uint32_t lFlags)
    {
        uint32_t lFirstCopy = 0;
        uint32_t lPrevCopy = 0;
        uint32_t lSource = lPrim;

        while (lSource)
        {
            const SPrims* pSource = static_cast<const SPrims*>(GetPrimData(lSource));
            const uint32_t lCopy = CopySinglePrim(lSource, lFlags);

            if (!lFirstCopy)
            {
                lFirstCopy = lCopy;
            }

            if (lPrevCopy)
            {
                SPrims* pPrevCopy = GetPrimitive<SPrims>(lPrevCopy);
                pPrevCopy->lNextPrim = lCopy;
            }

            lPrevCopy = lCopy;
            lSource = pSource->lNextPrim;
        }

        return lFirstCopy;
    }

    void ZPrimControlBase::SetPrimExtraData(uint32_t lPrim, uint32_t lFlag, const void* pData, uint32_t lCount)
    {
        // Do nothing
    }

    bool ZPrimControlBase::ExtraDataSupport()
    {
        return true;
    }

    bool ZPrimControlBase::IsVariantAvailable(uint32_t lPrim, uint32_t lVariantId)
    {
        ZPrimHandle hPrim { lPrim };
        if (!hPrim)
            return false;

        const SPrims* pPrim = hPrim;
        if (pPrim->lType != EPrimType::PTOBJECTHEADER)
            return false;

        const SPrimObjectHeader* pObjectHeader = hPrim;
        if (!pObjectHeader->lNumObjects)
            return false;

        ZPrimHandle hObjectTable { pObjectHeader->lObjectTable };
        const uint32_t* pObjectTable = hObjectTable;

        uint32_t lCurrentVariation = 0u;

        for (; lCurrentVariation < pObjectHeader->lNumObjects; ++lCurrentVariation)
        {
            const uint32_t lObject = pObjectTable[lCurrentVariation];
            ZPrimHandle hObject { lObject };
            const SPrimObject* pObject = hObject;

            if (pObject->lVariantId == lVariantId)
                break;
        }

        return true;
    }

    uint32_t ZPrimControlBase::GetNumVariants(uint32_t lPrim)
    {
        ZPrimHandle hPrim { lPrim };
        if (!hPrim)
            return false;

        const SPrims* pPrims = hPrim;
        if (pPrims->lType != EPrimType::PTOBJECTHEADER)
            return 0u;

        const SPrimObjectHeader* pObjectHeader = hPrim;
        ZPrimHandle hObjectTable { pObjectHeader->lObjectTable };
        const uint32_t* pObjectTable = hObjectTable;
        uint32_t lMaxVariationId = 0u;

        for (uint32_t i = 0; i < pObjectHeader->lNumObjects; ++i)
        {
            const uint32_t lObject = pObjectTable[i];
            ZPrimHandle hObject { lObject };
            const SPrimObject* pObject = hObject;

            lMaxVariationId = std::max(lMaxVariationId, static_cast<uint32_t>(pObject->lVariantId));
        }

        return lMaxVariationId;
    }

    uint32_t ZPrimControlBase::CreateUserLight(uint32_t lType)
    {
        uint32_t lSize = 0;

        switch (lType)
        {
            case 0:
                lSize = GetSizeOfLightSpot();
                break;
            case 1:
                lSize = GetSizeOfLightOmni();
                break;
            case 2:
                lSize = GetSizeOfLightSpotSquare();
                break;
            case 3:
            case 4:
                ZASSERT(false);
                break;
            default:
                break;
        }

        void* pMem = alloca(lSize);
        memset(pMem, 0, lSize);

        auto lNewPrim = AddPrimDataUnique(pMem, lSize, true);
        auto* pLD = GetPrimitive<SPrimLightEnvironment>(lNewPrim);
        pLD->lType = PTLIGHT;
        pLD->lLightType = lType;

        return lNewPrim;
    }

    bool ZPrimControlBase::GetOmniLightData(const uint32_t lPrimId, SPrimLightOmni* pDst)
    {
        ZASSERT(lPrimId);
        const auto* pData = reinterpret_cast<const SPrimLightOmni*>(GetPrimData(lPrimId));
        ZASSERT(pData->lType == PTLIGHT);
        ZASSERT(pData->lLightType == LTOMNI);

        GetLightOmniData(pDst, pData);
        return true;
    }

    bool ZPrimControlBase::GetSpotLightData(const uint32_t lPrimId, SPrimLightSpot* pDst)
    {
        ZASSERT(lPrimId);
        const auto* pData = reinterpret_cast<const SPrimLightSpot*>(GetPrimData(lPrimId));
        ZASSERT(pData->lType == PTLIGHT);
        ZASSERT(pData->lLightType == LTSPOT);

        GetLightSpotData(pDst, pData);
        return true;
    }

    bool ZPrimControlBase::GetSpotLightSquareData(const uint32_t lPrimId, SPrimLightSpotSquare* pDst)
    {
        ZASSERT(lPrimId);
        const auto* pData = reinterpret_cast<const SPrimLightSpotSquare*>(GetPrimData(lPrimId));
        ZASSERT(pData->lType == PTLIGHT);
        ZASSERT(pData->lLightType == LTSPOTSQUARE);

        GetLightSpotSquareData(pDst, pData);
        return true;
    }

    bool ZPrimControlBase::GetEnvironmentLightData(const uint32_t lPrimId, SPrimLightEnvironment* pDst)
    {
        ZASSERT(lPrimId);
        const auto* pData = reinterpret_cast<const SPrimLightEnvironment*>(GetPrimData(lPrimId));
        ZASSERT(pData->lType == PTLIGHT);
        if (pData->bIsDirectional)
        {
            ZASSERT(pData->lLightType == LTENVIRONMENT);
        }
        else
        {
            ZASSERT(pData->lLightType == LTENVIRONMENT);
        }
        GetLightEnvironmentData(pDst, pData);
        return true;
    }

    void ZPrimControlBase::ModifyOmniLight(SPrimLightOmni* pDst, const SPrimLightOmni* pSrc)
    {
        pDst->lDiffuseColor = pSrc->lDiffuseColor;
        pDst->lStaticShadowColor = pSrc->lStaticShadowColor;
        pDst->fMultiplier = pSrc->fMultiplier;

        if ((pSrc->fFarRange - 1.0f) >= pSrc->fNearRange)
        {
            pDst->fNearRange = pSrc->fNearRange;
        }
        else
        {
            pDst->fNearRange = pSrc->fFarRange - 1.0f;
        }

        pDst->fFarRange = pSrc->fFarRange;
        pDst->fInverseFarMinusNear = 1.0 / (pSrc->fFarRange - pDst->fNearRange);
    }

    void ZPrimControlBase::ModifyOmniLight(uint32_t lPrim, const SPrimLightOmni* pSrc)
    {
        auto* pData = const_cast<SPrimLightOmni*>(reinterpret_cast<const SPrimLightOmni*>(GetPrimData(lPrim)));
        ZASSERT(pData->lType == PTLIGHT);

        ModifyOmniLight(pData, pSrc);
    }

    void ZPrimControlBase::ModifySpotLight(SPrimLightSpot* pDst, const SPrimLightSpot* pSrc)
    {
        pDst->lDiffuseColor = pSrc->lDiffuseColor;
        pDst->lStaticShadowColor = pSrc->lStaticShadowColor;
        pDst->fMultiplier = pSrc->fMultiplier;
        if ((pSrc->fFarRange - 1.0f) >= pSrc->fNearRange)
        {
            pDst->fNearRange = pSrc->fNearRange;
        }
        else
        {
            pDst->fNearRange = pSrc->fFarRange - 1.0f;
        }

        pDst->fFarRange = pSrc->fFarRange;
        pDst->fInverseFarMinusNear = 1.0f / (pSrc->fFarRange - pDst->fNearRange);

        ZASSERT(pSrc->fHotSpot >= 0.0f && pSrc->fFallOff >= 0.0f);
        ZASSERT(pSrc->fHotSpot <= 1.5707964f && pSrc->fFallOff <= 1.5707964f);

        pDst->fHotSpot = pSrc->fHotSpot;
        pDst->fFallOff = pSrc->fFallOff;
        pDst->fCosHotSpot = cosf(pSrc->fHotSpot);
        pDst->fCosFallOff = cosf(pSrc->fFallOff);

        if (pSrc->fFallOff - pSrc->fHotSpot == 0.0f)
        {
            pDst->fLConst1 = 1.0f;
        }
        else
        {
            pDst->fLConst1 = 1.0f / (pDst->fCosHotSpot - pDst->fCosFallOff);
        }
    }

    void ZPrimControlBase::ModifySpotLight(uint32_t lPrim, const SPrimLightSpot* pSrc)
    {
        auto* pData = const_cast<SPrimLightSpot*>(reinterpret_cast<const SPrimLightSpot*>(GetPrimData(lPrim)));
        ZASSERT(pData->lType == PTLIGHT);

        ModifySpotLight(pData, pSrc);
    }

    void ZPrimControlBase::ModifySpotLightSquare(SPrimLightSpotSquare* pDst, const SPrimLightSpotSquare* pSrc)
    {
        ModifySpotLight(pDst, pSrc);
        pDst->fAspect = pSrc->fAspect;
    }

    void ZPrimControlBase::ModifySpotLightSquare(uint32_t lPrim, const SPrimLightSpotSquare* pSrc)
    {
        auto* pData = const_cast<SPrimLightSpotSquare*>(reinterpret_cast<const SPrimLightSpotSquare*>(GetPrimData(lPrim)));
        ZASSERT(pData->lType == PTLIGHT);

        ModifySpotLightSquare(pData, pSrc);
    }

    void ZPrimControlBase::ModifyEnvironment(uint32_t lPrim, const SPrimLightEnvironment* pSrc)
    {
        auto* pData = const_cast<SPrimLightEnvironment*>(reinterpret_cast<const SPrimLightEnvironment*>(GetPrimData(lPrim)));
        ZASSERT(pData->lType == PTLIGHT);

        ModifyEnvironmentLight(pData, pSrc);
    }

    const float* ZPrimControlBase::GetInvertGlobalPrimBones(uint32_t lPrim)
    {
        return nullptr;
    }

    void ZPrimControlBase::SetPrimFrameNr(const ZBaseGeom* pBaseGeom, float fFrameNr)
    {
        if (!pBaseGeom->DrawId())
            return;

        auto* pDraw = IDraw::Instance<ZRenderDraw>();
        auto* pEntry = static_cast<ZRenderEntryGeom*>(pDraw->m_apRenderEntryLookup[pBaseGeom->DrawId()]);
        pEntry->m_fVertexFrameNumber = fFrameNr;
    }

    bool ZPrimControlBase::GetPrimTextureAnimData(uint32_t lPrim, uint32_t* pFrameCount)
    {
        return false;
    }

    void ZPrimControlBase::SetPrimTextureFrameNr(const ZBaseGeom* pBaseGeom, float fFrameNr)
    {
        // Do nothing
    }

    uint32_t ZPrimControlBase::GetPrimTextureId(uint32_t lPrim, const char* pTextureName)
    {
        const auto iSubPrim = GetSubPrimUsingTexture(lPrim, pTextureName);
        if (iSubPrim)
        {
            return reinterpret_cast<const SPrims*>(GetPrimData(iSubPrim))->lTextureId;
        }

        return 0;
    }

    uint32_t ZPrimControlBase::GetSubPrimUsingTexture(uint32_t lPrim, const char* pTextureName)
    {
        while (lPrim)
        {
            const auto* pData = reinterpret_cast<const SPrims*>(lPrim);
            if ((pData->lTextureId & 0x7FF) != 0)
            {
                if (!pTextureName)
                    break;

                if (!_stricmp(pTextureName, "*"))
                    break;

                auto* pTexture = g_pRenderDll->GetTexture(pData->lTextureId);
                if (pTexture)
                {
                    if (!striwcmp(pTexture->m_szName, pTextureName))
                        break;
                }
            }

            lPrim = pData->lNextPrim;
        }

        return 0u;
    }

    void ZPrimControlBase::CopyPrimDrawMode(uint32_t lDestPrim, uint32_t lSourcePrim)
    {
        // Do nothing
    }

    void ZPrimControlBase::SetActiveNrTriangles(uint32_t lPrim, uint32_t lNrTriangles)
    {
        // Do nothing
    }

    uint32_t ZPrimControlBase::GetActiveNrTriangles(uint32_t lPrim)
    {
        return 0;
    }

    ZTextureBase* ZPrimControlBase::GetTextureFromPrim(uint32_t lPrim)
    {
        const auto* pData = reinterpret_cast<const SPrims*>(lPrim);
        if (!pData)
            return nullptr;

        return g_pRenderDll->GetTexture(pData->lTextureId);
    }

    uint32_t ZPrimControlBase::GetMaterialIdFromPrim(uint32_t lPrim, uint32_t lVariation)
    {
        if (!lPrim)
            return 0;

        const auto* hdr = reinterpret_cast<const SPrimObjectHeader*>(GetPrimData(lPrim));
        if (!hdr || hdr->lType != PTOBJECTHEADER)
            return 0;

        if (lVariation < hdr->lNumObjects)
            return 0;

        const uint32_t* pObjTable = reinterpret_cast<const uint32_t*>(GetPrimData(hdr->lObjectTable));
        ZPrimHandle hPrim { pObjTable[lVariation] };
        const SPrimMesh* pMesh = hPrim;

        return pMesh->lMaterialId;
    }

    uint32_t ZPrimControlBase::CreateSpriteArray(uint32_t lTextureId, uint32_t lDrawMode, SPRITETYPE lSpriteType, bool bSaveLoadThisPrim)
    {
        SPrimSpritesArray sData;
        sData.lType = 0;
        sData.lSpriteType = lSpriteType;
        sData.lTextureId = lTextureId;
        sData.lDrawMode = lDrawMode;
        sData.lPackType = 1;
        sData.lDrawDestination = 1;
        sData.lNextPrim = 0;
        sData.lDrawEntryId = 0;
        sData.lType = 2;

        return AddPrimDataUnique(&sData, sizeof(SPrimSpritesArray), bSaveLoadThisPrim);
    }

    uint32_t ZPrimControlBase::CopyBasePrim(uint32_t lPrim)
    {
        ZASSERT(false);
        return 0;
    }

    uint32_t ZPrimControlBase::GetBasePrim(uint32_t lPrim)
    {
        ZASSERT(false);
        return 0;
    }

    bool ZPrimControlBase::CalcPrimCenSize(uint32_t lPrim, float* pvCen, float* pvSize, bool bIncludeLights)
    {
        ZPrimHandle hPrim { lPrim };
        if (!hPrim)
            return false;

        const void* pData = GetPrimData(lPrim);
        if (!pData)
            return false;

        const SPrims* pPrims = hPrim;
        if (pPrims->lType == PTLIGHT)
        {
            if (bIncludeLights)
            {
                return CalcLightCenSize(pPrims, pvCen, pvSize);
            }

            return false;
        }

        if (pPrims->lType == PTOBJECTHEADER)
        {
            const SPrimObjectHeader* pHeader = hPrim;

            const ZVector3 vMin = pHeader->vMin, vMax = pHeader->vMax;
            ZASSERT(vMax.x >= vMin.x && vMax.y >= vMin.y && vMax.z >= vMin.z);

            vsub(pvSize, vMax, vMin);
            vscalar(pvSize, .5f);
            vsub(pvCen, vMax, pvSize);
            return true;
        }

        if (pPrims->lType < PTSPRITES || pPrims->lType >= PTDOT3STRIP)
        {
            const SPrimObjectHeader* pHeader = hPrim;
            ZASSERT(pHeader->lObjectTable);

            // It's weird (and probably incorrect logically) code, but should works fine
            const SPrimObjectHeader* pObj = reinterpret_cast<const SPrimObjectHeader*>(GetPrimData(pHeader->lObjectTable));
            if (pObj)
            {
                const ZVector3 vMin = pObj->vMin, vMax = pObj->vMax;
                ZASSERT(vMax.x >= vMin.x && vMax.y >= vMin.y && vMax.z >= vMin.z);

                vsub(pvSize, vMax, vMin);
                vscalar(pvSize, .5f);
                vsub(pvCen, vMax, pvSize);
                return true;
            }

            return false;
        }

        if (pPrims->lType == PTOBJECTSCATTER)
        {
            const SPrimObjectScatter* pScatter = hPrim;

            pvCen[0] = pScatter->vCen.x;
            pvCen[1] = pScatter->vCen.y;
            pvCen[2] = pScatter->vCen.z;
            pvSize[0] = pScatter->vSize.x;
            pvSize[1] = pScatter->vSize.y;
            pvSize[2] = pScatter->vSize.z;

            return true;
        }

        if (pPrims->lType == PTWATERPATCH)
        {
            const SPrimObjectHeader* pHeader = hPrim;
            ZASSERT(pHeader->lObjectTable);

            const auto* pWaterPatch = reinterpret_cast<const SPrimHeaderStripWaterPatch*>(GetPrimData(pHeader->lObjectTable));
            if (pWaterPatch)
            {
                const ZVector3 vMin = pWaterPatch->vMin, vMax = pWaterPatch->vMax;
                ZASSERT(vMax.x >= vMin.x && vMax.y >= vMin.y && vMax.z >= vMin.z);

                vsub(pvSize, vMax, vMin);
                vscalar(pvSize, .5f);
                vsub(pvCen, vMax, pvSize);
                return true;
            }

            return false;
        }

        ZVector3 vMin { 0.f }, vMax { 1.f };
        vsub(pvSize, vMax, vMin);
        vscalar(pvSize, .5f);
        vsub(pvCen, vMax, pvSize);
        return true;
    }

    bool ZPrimControlBase::CalcPrimCenSizeAlongMat(uint32_t lPrim, float* pvCen, float* pvSize, float* mMat, bool bIncludeLights)
    {
        if (!CalcPrimCenSize(lPrim, pvCen, pvSize, bIncludeLights))
            return false;

        TransformRootVector(pvCen, mMat);
        TransformBox(mMat, pvSize);

        return true;
    }

    uint32_t ZPrimControlBase::GetSizeOfLightOmni() const
    {
        return sizeof(SPrimLightOmni);
    }

    uint32_t ZPrimControlBase::GetSizeOfLightSpot() const
    {
        return sizeof(SPrimLightSpot);
    }

    uint32_t ZPrimControlBase::GetSizeOfLightSpotSquare() const
    {
        return sizeof(SPrimLightSpotSquare);
    }

    uint32_t ZPrimControlBase::GetSizeOfLightEnvironment() const
    {
        return sizeof(SPrimLightEnvironment);
    }

    void ZPrimControlBase::ColorIUTOU(uint32_t* pDst, const uint32_t* pSrc)
    {
        *pDst = *pSrc;
    }

    void ZPrimControlBase::ColorUTOIU(uint32_t* pDst, const uint32_t* pSrc)
    {
        *pDst = *pSrc;
    }

    void ZPrimControlBase::ColorFVTOIU(uint32_t* pDst, const float* pSrc)
    {
        const uint32_t r = static_cast<uint32_t>(pSrc[0] * 255.0f);
        const uint32_t g = static_cast<uint32_t>(pSrc[1] * 255.0f);
        const uint32_t b = static_cast<uint32_t>(pSrc[2] * 255.0f);
        const uint32_t a = static_cast<uint32_t>(pSrc[3] * 255.0f);
        *pDst = (a << 24) | (r << 16) | (g << 8) | b;
    }

    void ZPrimControlBase::ColorIUTOFV(float* pDst, const uint32_t* pSrc)
    {
        const uint32_t color = *pSrc;
        const float fScale = 1.0f / 255.0f;
        pDst[0] = static_cast<float>((color >> 16) & 0xFF) * fScale;
        pDst[1] = static_cast<float>((color >> 8)  & 0xFF) * fScale;
        pDst[2] = static_cast<float>(color          & 0xFF) * fScale;
        pDst[3] = static_cast<float>((color >> 24) & 0xFF) * fScale;
    }

    void ZPrimControlBase::ColorUTOFV(float* pDst, const uint32_t* pSrc)
    {
        ColorIUTOFV(pDst, pSrc);
    }

    float ZPrimControlBase::GetLightIntensity(const uint32_t lPrim, const float fDistance)
    {
        const SPrimLight* pLight = GetPrimitive<const SPrimLight>(lPrim);
        ZASSERT(pLight->lType == EPrimType::PTLIGHT);
        float fMultiplier = 1.0f;

        switch (pLight->lLightType)
        {
            case ELightType::LTSPOT: // 0
            case ELightType::LTOMNI: // 1
            case ELightType::LTSPOTSQUARE: // 2
            {
                const auto* pOmni = reinterpret_cast<const SPrimLightOmni*>(pLight);
                float fMultiplier2 = (pOmni->fFarRange - fDistance) * pOmni->fInverseFarMinusNear;
                if (fMultiplier2 >= 0.0f)
                {
                    fMultiplier2 = std::min(1.0f, fMultiplier2);
                    fMultiplier = fMultiplier2 * pOmni->fMultiplier;
                }
                else
                {
                    fMultiplier = 0.0f * pOmni->fMultiplier; // Yep, same in PC build lol
                }
            }
            break;
            case ELightType::LTENVIRONMENT:
            case ELightType::LTDIRECTIONAL:
            {
                fMultiplier = pLight->fMultiplier;
            }
            break;
            default:
            {
                // Impossible case: all lLightType are processed before
                fMultiplier = fDistance;
            }
            break;
        }

        const uint32_t lDiffuseColor = pLight->lDiffuseColor;
        const uint32_t lColorSum = (lDiffuseColor & 0xFFu)
                                 + ((lDiffuseColor >> 8) & 0xFFu)
                                 + ((lDiffuseColor >> 16) & 0xFFu);

        return fMultiplier * static_cast<float>(lColorSum);
    }

    uint32_t ZPrimControlBase::CombinePrimVariants(const uint32_t* aOriginalPrims, uint32_t lOriginalPrimsCount, const SPrimVariant* aVariants, uint32_t lVariantCount)
    {
        if (!lOriginalPrimsCount)
            return 0u;

        uint32_t lLastPrimVariant = 0u;
        if (lVariantCount)
            lLastPrimVariant = CreateVariantPrim(&aVariants[lVariantCount - 1]);

        for (int i = static_cast<int>(lVariantCount) - 2; i >= 0; --i)
        {
            const uint32_t lVariantPrim = CreateVariantPrim(&aVariants[i]);
            AppendPrims(lVariantPrim, lLastPrimVariant);
            lLastPrimVariant = lVariantPrim;
        }

        for (int i = static_cast<int>(lOriginalPrimsCount) - 1; i >= 0; --i)
        {
            const uint32_t lOriginalPrim = aOriginalPrims[i];
            AppendPrims(lOriginalPrim, lLastPrimVariant);
            lLastPrimVariant = lOriginalPrim;
        }

        UpdateStripBounds(lLastPrimVariant);
        return lLastPrimVariant;
    }

    void ZPrimControlBase::ModifyLight(SPrimLight* pDst, const SPrimLight* pSrc)
    {
        pDst->lDiffuseColor = pSrc->lDiffuseColor;
        pDst->fMultiplier = pSrc->fMultiplier;
    }

    void ZPrimControlBase::ModifyEnvironmentLight(SPrimLightEnvironment* pDst, const SPrimLightEnvironment* pSrc)
    {
        pDst->lDiffuseColor = pSrc->lDiffuseColor;
        pDst->lStaticShadowColor = pSrc->lStaticShadowColor;
        pDst->fMultiplier = pSrc->fMultiplier;
        pDst->bIsDirectional = pSrc->bIsDirectional;
        pDst->lDiffuseColorBack = pSrc->lDiffuseColorBack;
    }

    void ZPrimControlBase::GetLightData(SPrimLight* pDst, const SPrimLight* pSrc)
    {
        pDst->lLightType = pSrc->lLightType;
        pDst->lLightControl = pSrc->lLightControl;
        pDst->lDiffuseColor = pSrc->lDiffuseColor;
        pDst->fMultiplier = pSrc->fMultiplier;
        pDst->lStaticShadowColor = pSrc->lStaticShadowColor;
    }

    void ZPrimControlBase::GetLightOmniData(SPrimLightOmni* pDstOmni, const SPrimLightOmni* pSrcOmni)
    {
        GetLightData(pDstOmni, pSrcOmni);
        pDstOmni->fNearRange = pSrcOmni->fNearRange;
        pDstOmni->fFarRange = pSrcOmni->fFarRange;
        pDstOmni->fInverseFarMinusNear = pSrcOmni->fInverseFarMinusNear;
    }

    void ZPrimControlBase::GetLightSpotData(SPrimLightSpot* pDstSpot, const SPrimLightSpot* pSrcSpot)
    {
        GetLightOmniData(pDstSpot, pSrcSpot);
        pDstSpot->fFallOff = pSrcSpot->fFallOff;
        pDstSpot->fHotSpot = pSrcSpot->fHotSpot;
        pDstSpot->fCosFallOff = pSrcSpot->fCosFallOff;
        pDstSpot->fCosHotSpot = pSrcSpot->fCosHotSpot;
        pDstSpot->fLConst1 = pSrcSpot->fLConst1;
    }

    void ZPrimControlBase::GetLightSpotSquareData(SPrimLightSpotSquare* pDstSpotSquare, const SPrimLightSpotSquare* pSrcSpotSquare)
    {
        GetLightSpotData(pDstSpotSquare, pSrcSpotSquare);
        pDstSpotSquare->fAspect = pSrcSpotSquare->fAspect;
    }

    void ZPrimControlBase::GetLightEnvironmentData(SPrimLightEnvironment* pDst, const SPrimLightEnvironment* pSrc)
    {
        GetLightData(pDst, pSrc);
        pDst->lDiffuseColorBack = pSrc->lDiffuseColorBack;
    }

    bool ZPrimControlBase::CalcLightCenSize(const SPrims *pPrim, float *vCenter, float *vSize)
    {
        float fRange = 0.0f;
        vCenter[0] = 0.0f;
        vCenter[1] = 0.0f;
        vCenter[2] = 0.0f;

        const SPrimLightOmni* pOmni = reinterpret_cast<const SPrimLightOmni*>(pPrim);

        if (pOmni->lLightType <= ELightType::LTSPOTSQUARE)
        {
            fRange = pOmni->fFarRange;
        }

        vSize[0] = fRange;
        vSize[1] = fRange;
        vSize[2] = fRange;

        return true;
    }

    uint32_t ZPrimControlBase::CreateVariantPrim(const SPrimVariant *pVariant)
    {
        const uint32_t lFirstPrim = CopyPrim(pVariant->m_lPrim, 0);

        uint32_t lCurrentPrim = lFirstPrim;
        while (lCurrentPrim)
        {
            auto* pPrim = GetPrimitive<SPrimDrawSetup>(lCurrentPrim);
            if (pPrim->lType == PTSTRIP || pPrim->lType == PTSTRIPBONES)
            {
                pPrim->lVariantId = static_cast<uint16_t>(pVariant->m_lVariantId);
                pPrim->lTextureId = static_cast<uint16_t>(pVariant->m_lTextureId);
            }
            lCurrentPrim = pPrim->lNextPrim;
        }

        return lFirstPrim;
    }

    void ZPrimControlBase::AppendPrims(uint32_t lFirstPrim, uint32_t lLastPrim)
    {
        if (!lFirstPrim)
            return;

        ZASSERT(lFirstPrim != lLastPrim);

        SPrims* pPrim = GetPrimitive<SPrims>(lFirstPrim);
        while (pPrim->lNextPrim)
        {
            pPrim = GetPrimitive<SPrims>(pPrim->lNextPrim);
        }

        pPrim->lNextPrim = lLastPrim;
    }

    void ZPrimControlBase::UpdateStripBounds(uint32_t lPrim)
    {
        if (!lPrim)
            return;

        ZPrimHandle hPrim { lPrim };

        const SPrims* pPrim = hPrim;
        ZASSERT(pPrim->lType==PTSTRIP || pPrim->lType==PTSTRIPBONES || pPrim->lType==PTDOT3STRIP); // original assert

        const SPrimStrips* pStripPrim = hPrim;
        ZASSERT(pStripPrim->lTempHeader != 0); // original assert

        auto* pRootHeader = GetPrimitive<SPrimHeaderStrip>(pStripPrim->lTempHeader);

        for (uint32_t i = 0; i; i = pPrim->lNextPrim)
        {
            ZPrimHandle hChild { i };
            const SPrims* pChild = hChild;

            if (!(pPrim->lType==PTSTRIP || pPrim->lType==PTSTRIPBONES || pPrim->lType==PTDOT3STRIP))
            {
                continue;
            }

            const SPrimStrips* pChildStrips = hChild;
            if (!pChildStrips->lTempHeader)
            {
                continue;
            }

            ZPrimHandle hChildStrips { pChildStrips->lTempHeader };
            const SPrimHeaderStrip* pChildStripHeader = hChildStrips;
            vmin(pRootHeader->vMax, pChildStripHeader->vMax);
            vmax(pRootHeader->vMin, pChildStripHeader->vMin);
        }
    }

    ZPrimControlBase* ZPrimControlBase::Instance()
    {
        if (!g_pRenderDll) return nullptr;

        return g_pRenderDll->m_pPrimControl;
    }

    SSpriteArray* ZPrimControlBase::AllocSpriteArrays(uint32_t lNrSpriteArrays)
    {
        const uint32_t lMemSize = sizeof(SSpriteArray) * lNrSpriteArrays;
        auto* pSprites = reinterpret_cast<SSpriteArray*>(AllocSprites(lMemSize));
        memset(pSprites, 0, lMemSize);

        return pSprites;
    }

    SSpriteArrayElementUV* ZPrimControlBase::AllocSpriteArrayUV(uint32_t lNrSprites)
    {
        return reinterpret_cast<SSpriteArrayElementUV*>(AllocSprites(sizeof(SSpriteArrayElementUV) * lNrSprites));
    }

    uint32_t* ZPrimControlBase::AllocPrimList(uint32_t lNrPrims)
    {
        return reinterpret_cast<uint32_t*>(AllocSprites(sizeof(uint32_t) * lNrPrims));
    }

    void ZPrimControlBase::FreeSpriteArrays(SSpriteArray* pSpriteArrays, uint32_t lNrSpriteArrays)
    {
        if (!lNrSpriteArrays)
            return;

        IDraw::Instance()->FreeSprites(pSpriteArrays, sizeof(SSpriteArray) * lNrSpriteArrays);
    }

    void ZPrimControlBase::FreePrimList(uint32_t* pPrimList, uint32_t lNrPrims)
    {
        if (!lNrPrims)
            return;

        IDraw::Instance()->FreeSprites(pPrimList, sizeof(uint32_t) * lNrPrims);
    }

    void ZPrimControlBase::FreeSpriteArrayUV(SSpriteArrayElementUV *pSpriteArray, uint32_t lNrSprites)
    {
        if (!lNrSprites)
            return;

        IDraw::Instance()->FreeSprites(pSpriteArray, sizeof(SSpriteArrayElementUV) * lNrSprites);
    }

    void* ZPrimControlBase::AllocSprites(uint32_t lSize)
    {
        if (!lSize)
            return nullptr;

        return IDraw::Instance()->AllocSprites(lSize);
    }
}
