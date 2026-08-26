#include <Glacier/Render/Prim/ZPrimControlWintel.h>
#include <Glacier/Render/Prim/SPrimStripsWintel.h>
#include <Glacier/Render/Prim/SPrimSplinesWintel.h>
#include <Glacier/Render/Prim/SPrimBoneStripsWintel.h>
#include <Glacier/Render/Prim/SVertexWintel.h>
#include <Glacier/Render/Prim/SPrimObjectHeader.h>
#include <Glacier/Render/Prim/SPrimMesh.h>
#include <Glacier/Render/Prim/SPrimSubMesh.h>
#include <Glacier/Render/Prim/SPropertyBones.h>
#include <Glacier/Render/Prim/SBoneDefinition.h>
#include <Glacier/Render/Prim/ZBoneConstraintsHeader.h>
#include <Glacier/Render/Prim/SPrimLight.h>
#include <Glacier/Render/Prim/SPrimLightOmni.h>
#include <Glacier/Render/Prim/SPrimLightSpot.h>
#include <Glacier/Render/Prim/SPrimLightSpotSquare.h>
#include <Glacier/Render/Prim/SPrimLightEnvironment.h>
#include <Glacier/Render/Prim/EPrimType.h>
#include <Glacier/Render/Prim/ELightType.h>
#include <Glacier/Render/Draw/ZRenderDraw.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/Material/ZRenderMaterialBuffer.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/ZUniAssert.h>
#include <cstdio>
#include <cstring>


namespace Glacier
{
    namespace
    {
        const SPropertyBones* GetBonesNr(ZPrimControlWintel* pSelf, uint32_t lPrim)
        {
            const auto* pHeader = static_cast<const SPrimObjectHeader*>(pSelf->GetPrimData(lPrim));
            if (pHeader->lType != EPrimType::PTOBJECTHEADER || (pHeader->lPropertyFlags & SPrimObjectHeader::HAS_BONES) == 0)
            {
                return nullptr;
            }

            const auto* pBones = static_cast<const SPropertyBones*>(pSelf->GetPrimData(pHeader->lPropertyData));
            const auto* pBonesData = static_cast<const SPropertyBones*>(pSelf->GetPrimData(pBones->lBoneDefinitions));
            ZASSERT(pBonesData != nullptr);

            return pBonesData;
        }

        void BuildPrimTriangles(REFTAB* ppTris, const float* pVertices, const uint16_t* pIndices, uint32_t lStride)
        {
            const uint16_t lNrStrips = pIndices[0];
            if (!lNrStrips)
            {
                return;
            }

            const uint16_t* p = pIndices + 1;
            uint16_t lRemaining = lNrStrips;

            while (true)
            {
                const uint16_t lEntryCount = *p;
                const uint16_t* pEntries = p + 1;

                if (lEntryCount != 2)
                {
                    const uint16_t* pTri = pEntries + 2;

                    for (uint32_t i = 0; i < lEntryCount - 2; ++i)
                    {
                        const uint16_t i0 = *(pTri - 2);
                        uint16_t i1;
                        uint16_t i2;
                        if (i & 1)
                        {
                            i1 = *pTri;
                            i2 = *(pTri - 1);
                        }
                        else
                        {
                            i1 = *(pTri - 1);
                            i2 = *pTri;
                        }

                        if (i0 != i1 && i0 != i2 && i1 != i2)
                        {
                            auto* pDst = reinterpret_cast<float*>(ppTris->Add(0));
                            const float* v0 = pVertices + lStride * i0;
                            const float* v1 = pVertices + lStride * i1;
                            const float* v2 = pVertices + lStride * i2;

                            pDst[0] = v0[0]; pDst[1] = v0[1]; pDst[2] = v0[2];
                            pDst[3] = v1[0]; pDst[4] = v1[1]; pDst[5] = v1[2];
                            pDst[6] = v2[0]; pDst[7] = v2[1]; pDst[8] = v2[2];
                        }

                        ++pTri;
                    }
                }

                p = pEntries + lEntryCount;
                if (!--lRemaining)
                {
                    break;
                }
            }
        }
    }

    void ZPrimControlWintel::GetPrimExtraData(uint32_t lPrim, uint32_t lFlag, void* pData, uint32_t lCount)
    {
        const auto* pPrim = GetPrimitive<const SPrims>(lPrim);
        if (pPrim->lType != EPrimType::PTSTRIP)
        {
            return;
        }

        const auto* pStrip = static_cast<const SPrimStripsWintel*>(pPrim);

        if (lFlag == 16)
        {
            ZASSERT(pStrip->lIndicesCount >= lCount);

            const auto* pIndices = static_cast<const uint16_t*>(GetPrimData(pStrip->lIndices));
            auto* pOut = static_cast<uint16_t*>(pData);

            for (uint32_t i = 0; i < lCount; ++i)
            {
                pOut[i] = pIndices[i];
            }

            return;
        }

        const auto* pVertices = static_cast<const SVertexWintel*>(GetPrimData(pStrip->lVertices));
        ZASSERT(pStrip->lNrKeys * pStrip->lNrEntries >= lCount);

        auto* pOut = static_cast<float*>(pData);
        switch (lFlag)
        {
            case 1: // position
                for (uint32_t i = 0; i < lCount; ++i)
                {
                    pOut[0] = pVertices[i].p.x;
                    pOut[1] = pVertices[i].p.y;
                    pOut[2] = pVertices[i].p.z;
                    pOut += 3;
                }
                break;
            case 2: // color
                for (uint32_t i = 0; i < lCount; ++i)
                {
                    *pOut++ = *reinterpret_cast<const float*>(&pVertices[i].c);
                }
                break;
            case 4: // normal
                for (uint32_t i = 0; i < lCount; ++i)
                {
                    pOut[0] = pVertices[i].n.x;
                    pOut[1] = pVertices[i].n.y;
                    pOut[2] = pVertices[i].n.z;
                    pOut += 3;
                }
                break;
            default: // uv
                for (uint32_t i = 0; i < lCount; ++i)
                {
                    pOut[0] = pVertices[i].t.x;
                    pOut[1] = pVertices[i].t.y;
                    pOut += 2;
                }
                break;
        }
    }

    void ZPrimControlWintel::GetPrimTriangles(REFTAB* ppTris, SPrims* pPrims, uint32_t lPrim)
    {
        if (pPrims->lType == EPrimType::PTSTRIPBONES)
        {
            const auto* pStrip = static_cast<const SPrimStripsWintel*>(pPrims);
            const auto* pVertices = static_cast<const float*>(GetPrimData(pStrip->lVertices));
            const auto* pIndices = static_cast<const uint16_t*>(GetPrimData(pStrip->lIndices));
            BuildPrimTriangles(ppTris, pVertices, pIndices, 13);
        }
        else if (pPrims->lType == EPrimType::PTSTRIP)
        {
            const auto* pStrip = static_cast<const SPrimStripsWintel*>(pPrims);
            const auto* pVertices = static_cast<const float*>(GetPrimData(pStrip->lVertices));
            const auto* pIndices = static_cast<const uint16_t*>(GetPrimData(pStrip->lIndices));
            BuildPrimTriangles(ppTris, pVertices, pIndices, 9);
        }
    }

    void ZPrimControlWintel::GetPrimTriangles(REFTAB* ppTris, uint32_t lPrim)
    {
        if (!lPrim)
        {
            return;
        }

        SPrims* pPrim;
        while (true)
        {
            pPrim = GetPrimitive<SPrims>(lPrim);
            if (pPrim->lType == EPrimType::PTOBJECTHEADER)
            {
                break;
            }

            GetPrimTriangles(ppTris, pPrim, lPrim);

            lPrim = pPrim->lNextPrim;
            if (!lPrim)
            {
                return;
            }
        }

        GetPrimTriangles(ppTris, pPrim, lPrim);
    }

    void ZPrimControlWintel::GetPrimInfoString(char* pInfo, uint32_t lInfoMaxSize, uint32_t lPrim)
    {
        pInfo[0] = 0;

        const auto* pPrim = GetPrimitive<const SPrims>(lPrim);
        if (pPrim->lType != EPrimType::PTOBJECTHEADER)
        {
            return;
        }

        const auto* pHeader = static_cast<const SPrimObjectHeader*>(pPrim);
        const auto* pObjectTable = static_cast<const uint32_t*>(GetPrimData(pHeader->lObjectTable));

        uint32_t lVerticesCount = 0;
        for (uint32_t i = 0; i < pHeader->lNumObjects; ++i)
        {
            const auto* pObject = GetPrimitive<const SPrims>(pObjectTable[i]);
            if (pObject->lType != EPrimType::PTMESH)
            {
                continue;
            }

            const auto* pMesh = static_cast<const SPrimMesh*>(pObject);
            const auto* pSubMeshTable = static_cast<const uint32_t*>(GetPrimData(pMesh->lSubMeshTable));
            const auto* pSubMesh = GetPrimitive<const SPrimSubMesh>(pSubMeshTable[0]);
            lVerticesCount += pSubMesh->lNumVertices;
        }

        const auto* pBones = GetBonesNr(this, lPrim);

        char szSource[512];
        if (pBones)
        {
            std::snprintf(szSource, sizeof(szSource), "Bones %d Vertices %d", pBones->lNumBones, lVerticesCount);
        }
        else
        {
            std::snprintf(szSource, sizeof(szSource), "Vertices %d", lVerticesCount);
        }

        strncpy(pInfo, szSource, lInfoMaxSize);
        if (strlen(szSource) >= lInfoMaxSize)
        {
            pInfo[lInfoMaxSize - 1] = 0;
        }
    }

    bool ZPrimControlWintel::HasKeys(uint32_t lPrim)
    {
        const auto* pPrim = GetPrimitive<const SPrims>(lPrim);
        if (pPrim->lType != EPrimType::PTSTRIP)
        {
            return false;
        }

        return static_cast<const SPrimStripsWintel*>(pPrim)->lNrKeys != 1;
    }

    uint32_t ZPrimControlWintel::GetPrimSize(const SPrims* lPrim)
    {
        switch (lPrim->lType)
        {
            case EPrimType::PTSTRIP:
                return sizeof(SPrimStripsWintel); // 124
            case EPrimType::PTSTRIPBONES:
                return sizeof(SPrimBoneStripsWintel); // 164
            case EPrimType::PTSPLINES:
                return sizeof(SPrimSplinesWintel); // 132
            case EPrimType::PTOBJECTHEADER:
                return sizeof(SPrimObjectHeader); // 60
            case EPrimType::PTLIGHT:
            {
                const auto* pLight = reinterpret_cast<const SPrimLight*>(lPrim);
                switch (pLight->lLightType)
                {
                    case ELightType::LTSPOT:
                        return sizeof(SPrimLightSpot); // 64
                    case ELightType::LTOMNI:
                        return sizeof(SPrimLightOmni); // 44
                    case ELightType::LTSPOTSQUARE:
                        return sizeof(SPrimLightSpotSquare); // 68
                    case ELightType::LTENVIRONMENT:
                    case ELightType::LTDIRECTIONAL:
                        return sizeof(SPrimLightEnvironment); // 40
                    default:
                        ZASSERT(false);
                        return sizeof(SPrimObjectHeader); // 60
                }
            }
            default:
                ZASSERT(false);
                return 0;
        }
    }

    uint32_t ZPrimControlWintel::CopySinglePrim(uint32_t lPrim, uint32_t lFlags)
    {
        const auto* pPrim = GetPrimitive<const SPrims>(lPrim);
        const uint32_t lNewPrim = AddPrimDataUnique(pPrim, GetPrimSize(pPrim), true);

        if ((lFlags & 0xF) != 0)
        {
            auto* pNewPrim = GetPrimitive<SPrimStripsWintel>(lNewPrim);
            if (pNewPrim->lType != EPrimType::PTSTRIP)
            {
                return 0;
            }

            const auto* pVertices = static_cast<const SVertexWintel*>(GetPrimData(pNewPrim->lVertices));
            const uint32_t lVertexBytes = pNewPrim->lNrEntries * pNewPrim->lNrKeys * sizeof(SVertexWintel);
            pNewPrim->lVertices = AddPrimDataUnique(pVertices, lVertexBytes, true);
        }

        return lNewPrim;
    }

    uint32_t ZPrimControlWintel::GetNumEntries(uint32_t lPrim, uint32_t lFlags)
    {
        if (!lPrim)
        {
            return 0;
        }

        const auto* pPrim = GetPrimitive<const SPrims>(lPrim);
        if (pPrim->lType != EPrimType::PTSTRIP)
        {
            if (pPrim->lType != EPrimType::PTMESH)
            {
                ZASSERT(false);
            }

            return 0;
        }

        const auto* pStrip = static_cast<const SPrimStripsWintel*>(pPrim);
        switch (lFlags)
        {
            case 1:
            case 2:
            case 4:
            case 8:
                return pStrip->lNrEntries * pStrip->lNrKeys;
            case 16:
                return pStrip->lIndicesCount;
            default:
                ZASSERT(false);
                return 0;
        }
    }

    uint32_t ZPrimControlWintel::GetNrBones(uint32_t lPrim)
    {
        const auto* pBonesData = GetBonesNr(this, lPrim);
        if (pBonesData)
        {
            return pBonesData->lNumBones;
        }

        const auto* pPrim = GetPrimitive<const SPrimBoneStripsWintel>(lPrim);
        ZASSERT(pPrim->lType == EPrimType::PTSTRIPBONES);

        return pPrim->lNrBones;
    }

    const float* ZPrimControlWintel::GetLocalPrimBones(uint32_t lPrim)
    {
        const auto* pBonesData = GetBonesNr(this, lPrim);
        if (pBonesData)
        {
            return static_cast<const float*>(GetPrimData(pBonesData->lLocalBones));
        }

        const auto* pPrim = GetPrimitive<const SPrimBoneStripsWintel>(lPrim);
        ZASSERT(pPrim->lType == EPrimType::PTSTRIPBONES);

        return static_cast<const float*>(GetPrimData(pPrim->lLocalBones));
    }

    const float* ZPrimControlWintel::GetLocalPrimBonesQuats(uint32_t lPrim)
    {
        const auto* pBonesData = GetBonesNr(this, lPrim);
        if (pBonesData)
        {
            return static_cast<const float*>(GetPrimData(pBonesData->lLocalBonesQuats));
        }

        const auto* pPrim = GetPrimitive<const SPrimBoneStripsWintel>(lPrim);
        ZASSERT(pPrim->lType == EPrimType::PTSTRIPBONES);

        return static_cast<const float*>(GetPrimData(pPrim->lLocalBonesQuats));
    }

    const float* ZPrimControlWintel::GetGlobalPrimBones(uint32_t lPrim)
    {
        const auto* pBonesData = GetBonesNr(this, lPrim);
        if (pBonesData)
        {
            return static_cast<const float*>(GetPrimData(pBonesData->lGlobalBones));
        }

        const auto* pPrim = GetPrimitive<const SPrimBoneStripsWintel>(lPrim);
        ZASSERT(pPrim->lType == EPrimType::PTSTRIPBONES);

        return static_cast<const float*>(GetPrimData(pPrim->lGlobalBones));
    }

    const float* ZPrimControlWintel::GetConvBones(uint32_t lPrim)
    {
        const auto* pBonesData = GetBonesNr(this, lPrim);
        if (pBonesData)
        {
            return static_cast<const float*>(GetPrimData(pBonesData->lConvBones));
        }

        const auto* pPrim = GetPrimitive<const SPrimBoneStripsWintel>(lPrim);
        ZASSERT(pPrim->lType == EPrimType::PTSTRIPBONES);

        return static_cast<const float*>(GetPrimData(pPrim->lConvBones));
    }

    const SBoneDefinition* ZPrimControlWintel::GetBoneDefinitions(uint32_t lPrim)
    {
        const auto* pBonesData = GetBonesNr(this, lPrim);
        if (pBonesData)
        {
            return static_cast<const SBoneDefinition*>(GetPrimData(pBonesData->lBoneDefinitions));
        }

        const auto* pPrim = GetPrimitive<const SPrimBoneStripsWintel>(lPrim);
        ZASSERT(pPrim->lType == EPrimType::PTSTRIPBONES);

        return static_cast<const SBoneDefinition*>(GetPrimData(pPrim->lBoneDefinitions));
    }

    const ZBoneConstraintsHeader* ZPrimControlWintel::GetBoneConstraints(uint32_t lPrim)
    {
        const auto* pBonesData = GetBonesNr(this, lPrim);
        if (pBonesData)
        {
            return static_cast<const ZBoneConstraintsHeader*>(GetPrimData(pBonesData->lBoneConstraintsHeader));
        }

        const auto* pPrim = GetPrimitive<const SPrimBoneStripsWintel>(lPrim);
        ZASSERT(pPrim->lType == EPrimType::PTSTRIPBONES);

        return static_cast<const ZBoneConstraintsHeader*>(GetPrimData(pPrim->lBoneConstraintsHeader));
    }

    const uint8_t* ZPrimControlWintel::GetBoneIdToIndexLookup(uint32_t lPrim)
    {
        const auto* pBonesData = GetBonesNr(this, lPrim);
        if (pBonesData)
        {
            return static_cast<const uint8_t*>(GetPrimData(pBonesData->lBoneIdToIndexLookup));
        }

        const auto* pPrim = GetPrimitive<const SPrimBoneStripsWintel>(lPrim);
        ZASSERT(pPrim->lType == EPrimType::PTSTRIPBONES);

        return static_cast<const uint8_t*>(GetPrimData(pPrim->lBoneIdToIndexLookup));
    }

    uint32_t ZPrimControlWintel::GetVerticesXYZ(uint32_t lPrim, float* pVertices, uint32_t lVerticesNr)
    {
        if (!lPrim)
        {
            return 0;
        }

        const auto* pPrim = GetPrimitive<const SPrims>(lPrim);
        const SVertexWintel* pVerticesData;
        const uint16_t* pIndexLookup;

        if (pPrim->lType == EPrimType::PTOBJECTHEADER)
        {
            // Object headers are passed through as raw pointers in the PC build.
            pVerticesData = reinterpret_cast<const SVertexWintel*>(lPrim);
            pIndexLookup = reinterpret_cast<const uint16_t*>(lPrim);
        }
        else
        {
            ZASSERT(pPrim->lType == EPrimType::PTSTRIP);

            const auto* pStrip = static_cast<const SPrimStripsWintel*>(pPrim);
            pVerticesData = static_cast<const SVertexWintel*>(GetPrimData(pStrip->lVertices));
            pIndexLookup = static_cast<const uint16_t*>(GetPrimData(pStrip->lIndices));
        }

        const uint16_t lCount = pIndexLookup[0];
        ZASSERT(lCount != 0);

        const uint16_t lMax = pIndexLookup[1];
        const uint16_t* pIndexList = pIndexLookup + 2;

        uint32_t lCopy = lVerticesNr;
        if (lCopy > lMax)
        {
            lCopy = lMax;
        }

        for (uint32_t i = 0; i < lCopy; ++i)
        {
            const SVertexWintel& v = pVerticesData[pIndexList[i]];
            pVertices[0] = v.p.x;
            pVertices[1] = v.p.y;
            pVertices[2] = v.p.z;
            pVertices += 3;
        }

        return lCopy;
    }

    void ZPrimControlWintel::GetVerticesColor(uint32_t lPrim, uint32_t* pColors, uint32_t lColorsNr)
    {
        const auto* pStrip = GetPrimitive<const SPrimStripsWintel>(lPrim);
        ZASSERT(lColorsNr <= pStrip->lNrEntries);

        const auto* pVertices = static_cast<const SVertexWintel*>(GetPrimData(pStrip->lVertices));
        for (uint32_t i = 0; i < lColorsNr; ++i)
        {
            ColorIUTOU(&pColors[i], &pVertices[i].c);
        }
    }

    uint32_t ZPrimControlWintel::GetPrimVertices(uint32_t lPrim, float* pVertices, uint32_t lVerticesNr)
    {
        const auto* pStrip = GetPrimitive<const SPrimStripsWintel>(lPrim);
        ZASSERT(pStrip->lType == EPrimType::PTSTRIP);

        const auto* pSrc = static_cast<const SVertexWintel*>(GetPrimData(pStrip->lVertices));

        const uint32_t lMax = pStrip->lNrEntries * pStrip->lNrKeys;
        uint32_t lCount = lVerticesNr;
        if (lCount > lMax)
        {
            lCount = lMax;
        }

        for (uint32_t i = 0; i < lCount; ++i)
        {
            pVertices[0] = pSrc[i].p.x;
            pVertices[1] = pSrc[i].p.y;
            pVertices[2] = pSrc[i].p.z;
            pVertices += 3;
        }

        return lCount;
    }

    bool ZPrimControlWintel::GetPrimVertAnim(uint32_t lPrim, uint32_t* pFrameStart, uint32_t* pFrameStep, uint32_t* pNrKeys)
    {
        const auto* pPrim = GetPrimitive<const SPrims>(lPrim);
        if (!pPrim)
        {
            return false;
        }

        if (pPrim->lType != EPrimType::PTSTRIP)
        {
            if (pPrim->lType != EPrimType::PTOBJECTHEADER)
            {
                return false;
            }

            const auto* pHeader = static_cast<const SPrimObjectHeader*>(pPrim);
            if ((pHeader->lPropertyFlags & SPrimObjectHeader::HAS_FRAMES) == 0 || pHeader->lNumObjects == 0)
            {
                return false;
            }

            const auto* pObjectTable = static_cast<const uint32_t*>(GetPrimData(pHeader->lObjectTable));

            const SPrimMesh* pMesh = nullptr;
            for (uint32_t i = 0; i < pHeader->lNumObjects; ++i)
            {
                const auto* pObject = GetPrimitive<const SPrims>(pObjectTable[i]);
                if (pObject->lType == EPrimType::PTMESH)
                {
                    pMesh = static_cast<const SPrimMesh*>(pObject);
                    break;
                }
            }

            if (!pMesh)
            {
                return false;
            }

            *pFrameStart = pMesh->lFrameStart;
            *pFrameStep = pMesh->lFrameStep;
            *pNrKeys = pMesh->lNumFrames;
            return true;
        }

        const auto* pStrip = static_cast<const SPrimStripsWintel*>(pPrim);
        *pFrameStart = pStrip->lFrameStart;
        *pFrameStep = pStrip->lFrameStep;
        *pNrKeys = pStrip->lNrKeys;
        return true;
    }

    bool ZPrimControlWintel::GetPrimTextureAnimData(uint32_t lPrim, uint32_t* pFrameCount)
    {
        *pFrameCount = 0;

        const uint32_t lSubPrims = CountSubPrims(lPrim);
        if (!lSubPrims)
        {
            return false;
        }

        for (uint32_t i = 0; i < lSubPrims; ++i)
        {
            const uint32_t lMaterialId = GetMaterialIdFromPrim(lPrim, i);
            if (lMaterialId)
            {
                const uint32_t lFrameCount = g_pRenderDll->m_pMaterialBuffer->GetTextureAnimData(lMaterialId);
                if (*pFrameCount < lFrameCount)
                {
                    *pFrameCount = lFrameCount;
                }
            }
        }

        return *pFrameCount != 0;
    }

    void ZPrimControlWintel::SetPrimTextureFrameNr(const ZBaseGeom* pBaseGeom, float fFrameNr)
    {
        if (pBaseGeom->m_lPrim && pBaseGeom->m_lDrawId)
        {
            IDraw::Instance<ZRenderDraw>()->SetTextureFrameNumber(pBaseGeom, fFrameNr);
        }
    }

    void ZPrimControlWintel::CopyPrimDrawMode(uint32_t lDestPrim, uint32_t lSourcePrim)
    {
        auto* pDest = GetPrimitive<SPrimDrawSetup>(lDestPrim);
        const auto* pSource = GetPrimitive<const SPrimDrawSetup>(lSourcePrim);

        if (pSource->lType == EPrimType::PTOBJECTHEADER)
        {
            return;
        }

        if (pDest->lType == EPrimType::PTOBJECTHEADER)
        {
            return;
        }

        ZASSERT(pSource->lType == EPrimType::PTSTRIP && pDest->lType == EPrimType::PTSTRIP);

        pDest->lDrawMode = pSource->lDrawMode;
        pDest->lTextureId = pSource->lTextureId;
    }

    void ZPrimControlWintel::SetActiveNrTriangles(uint32_t lPrim, uint32_t lNrTriangles)
    {
        auto* pStrip = GetPrimitive<SPrimStripsWintel>(lPrim);
        ZASSERT(pStrip->lType == EPrimType::PTSTRIP);

        auto* pIndices = GetPrimitive<uint16_t>(pStrip->lIndices);
        pIndices[0] = static_cast<uint16_t>(lNrTriangles);
        pStrip->lNrEntries = static_cast<uint16_t>(3 * lNrTriangles);
    }

    uint32_t ZPrimControlWintel::GetActiveNrTriangles(uint32_t lPrim)
    {
        const auto* pStrip = GetPrimitive<const SPrimStripsWintel>(lPrim);
        ZASSERT((pStrip->lNrEntries % 3) == 0);
        ZASSERT(pStrip->lType == EPrimType::PTSTRIP);

        return pStrip->lNrEntries / 3;
    }

    const float* ZPrimControlWintel::GetPrimSplineVertices(uint32_t lPrim)
    {
        const auto* pPrim = GetPrimitive<const SPrimSplinesWintel>(lPrim);
        ZASSERT(pPrim->lType == EPrimType::PTSPLINES);

        return static_cast<const float*>(GetPrimData(pPrim->lSplineVertices));
    }

    uint32_t* ZPrimControlWintel::GetPrimSplineIndices(uint32_t lPrim)
    {
        const auto* pPrim = GetPrimitive<const SPrimSplinesWintel>(lPrim);
        ZASSERT(pPrim->lType == EPrimType::PTSPLINES);

        return GetPrimitive<uint32_t>(pPrim->lSplineIndices);
    }

    ZPrimControlWintel::~ZPrimControlWintel() = default;
}
