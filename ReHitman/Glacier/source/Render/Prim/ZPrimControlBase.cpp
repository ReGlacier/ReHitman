#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/Render/Prim/SHandleTableEntry.h>
#include <Glacier/Render/Prim/ZPrimHandle.h>
#include <Glacier/Render/Prim/SPrimObject.h>
#include <Glacier/Render/Prim/SPrimHeader.h>
#include <Glacier/Render/Prim/SPrimMesh.h>
#include <Glacier/Render/Prim/EPrimType.h>
#include <Glacier/Render/Prim/SPrims.h>
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
    
    const void* ZPrimControlBase::GetPrimData(uint32_t lPrim)
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
        // TODO: Finish me
        return false;
    }

    bool ZPrimControlBase::CheckBoxInsidePrim(uint32_t lPrim, const float* pvBoxP1, const float* pvBoxP2, const float* pvThreshold)
    {
        // TODO: Finish this place after ZPrimAccessMesh will be reversed
        // See PC 0x46DDE0 / iOS 0x100234F28 / PS2 0x17E478
        return false;
    }

    uint32_t ZPrimControlBase::AddPrimDataUnique(const void* pData, uint32_t lDataSize)
    {
        if (!g_pRenderDll->m_pPrimBuffer)
        {
            ZASSERT(false);
            return 0;
        }

        char* pCurrentPrimBuffer = g_pRenderDll->m_pCurrentPrimBuffer;
        const uint32_t lFreeSpace = static_cast<uint32_t>(g_pRenderDll->m_pCurrentPrimBufferBack - pCurrentPrimBuffer);
        const bool bBufferOverflow = lFreeSpace < lDataSize;
        const bool bHandlesOverflow = g_iCurrentDynamicPrimBuffersCount > static_cast<int32_t>(g_lPrimHandleToPointerFreeBack);

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

        if (bBufferOverflow)
        {
            // TODO: Finish this place after ZRenderDll::InstallPrimBuffer will be reversed.
            // PC 0x46D9BE: dead code in release because bBufferOverflow implies the assert above never returns.
            // pPrimData = g_pRenderDll->m_pCurrentPrimBuffer;
            // memcpy(pPrimData, pData, lDataSize);
            // g_pRenderDll->m_pCurrentPrimBuffer += lDataSize;
            // lNewPrimIdx = g_iCurrentDynamicPrimBuffersCount++;
            return 0;
        }

        g_pRenderDll->m_pCurrentPrimBufferBack -= lDataSize;
        pPrimData = g_pRenderDll->m_pCurrentPrimBufferBack;
        memcpy(pPrimData, pData, lDataSize);
        lNewPrimIdx = g_lPrimHandleToPointerFreeBack--;

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
            // TODO: Finish me
            return 0;
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
        

        // TODO: Finish me
        return lTotalCount;
    }

    uint32_t ZPrimControlBase::CopyPrim(uint32_t lPrim, uint32_t lFlags)
    {
        // TODO: Finish me
        return 0u;
    }

    void ZPrimControlBase::SetPrimExtraData(uint32_t lPrim, uint32_t lFlag, const void* pData, uint32_t lCount)
    {
        // TODO: Finish me
    }

    bool ZPrimControlBase::ExtraDataSupport()
    {
        // TODO: Finish me
        return false;
    }

    bool ZPrimControlBase::IsVariantAvailable(uint32_t lPrim, uint32_t lVariantId)
    {
        // TODO: Finish me
        return false;
    }

    uint32_t ZPrimControlBase::GetNumVariants(uint32_t lPrim)
    {
        // TODO: Finish me
        return 0u;
    }

    uint32_t ZPrimControlBase::CreateUserLight(uint32_t lType)
    {
        // TODO: Finish me
        return 0u;
    }

    bool ZPrimControlBase::GetOmniLightData(const uint32_t lPrimId, SPrimLightOmni* pDst)
    {
        // TODO: Finish me
        return false;
    }

    bool ZPrimControlBase::GetSpotLightData(const uint32_t lPrimId, SPrimLightSpot* pDst)
    {
        // TODO: Finish me
        return false;
    }

    bool ZPrimControlBase::GetSpotLightSquareData(const uint32_t lPrimId, SPrimLightSpot* pDst)
    {
        // TODO: Finish me
        return false;
    }

    bool ZPrimControlBase::GetEnvironmentLightData(const uint32_t lPrimId, SPrimLightEnvironment* pDst)
    {
        // TODO: Finish me
        return false;
    }

    void ZPrimControlBase::ModifyOmniLight(SPrimLightOmni* pDst, const SPrimLightOmni* pSrc)
    {
        // TODO: Finish me
    }

    void ZPrimControlBase::ModifyOmniLight(uint32_t lPrim, const SPrimLightOmni* pSrc)
    {
        // TODO: Finish me
    }

    void ZPrimControlBase::ModifySpotLight(SPrimLightSpot* pDst, const SPrimLightSpot* pSrc)
    {
        // TODO: Finish me
    }

    void ZPrimControlBase::ModifySpotLight(uint32_t lPrim, const SPrimLightSpot* pSrc)
    {
        // TODO: Finish me
    }

    void ZPrimControlBase::ModifySpotLightSquare(SPrimLightSpotSquare* pDst, const SPrimLightSpotSquare* pSrc)
    {
        // TODO: Finish me
    }

    void ZPrimControlBase::ModifySpotLightSquare(uint32_t lPrim, const SPrimLightSpotSquare* pSrc)
    {
        // TODO: Finish me
    }

    void ZPrimControlBase::ModifyEnvironment(uint32_t lPrim, const SPrimLightEnvironment* pSrc)
    {
        // TODO: Finish me
    }

    const float* ZPrimControlBase::GetInvertGlobalPrimBones(uint32_t lPrim)
    {
        // TODO: Finish me
        return nullptr;
    }

    void ZPrimControlBase::SetPrimFrameNr(const ZBaseGeom* pBaseGeom, float fFrameNr)
    {
        // TODO: Finish me
    }

    bool ZPrimControlBase::GetPrimTextureAnimData(uint32_t lPrim, uint32_t* pFrameCount)
    {
        // TODO: Finish me
        return false;
    }

    void ZPrimControlBase::SetPrimTextureFrameNr(const ZBaseGeom* pBaseGeom, float fFrameNr)
    {
        // TODO: Finish me
    }

    uint32_t ZPrimControlBase::GetPrimTextureId(uint32_t lPrim, const char* pTextureName)
    {
        // TODO: Finish me
        return 0u;
    }

    uint32_t ZPrimControlBase::GetSubPrimUsingTexture(uint32_t lPrim, const char* pTextureName)
    {
        // TODO: Finish me
        return 0u;
    }

    void ZPrimControlBase::CopyPrimDrawMode(uint32_t lDestPrim, uint32_t lSourcePrim)
    {
        // TODO: Finish me
    }

    void ZPrimControlBase::SetActiveNrTriangles(uint32_t lPrim, uint32_t lNrTriangles)
    {
        // TODO: Finish me
    }

    uint32_t ZPrimControlBase::GetActiveNrTriangles(uint32_t lPrim)
    {
        // TODO: Finish me
        return 0;
    }

    ZTextureBase* ZPrimControlBase::GetTextureFromPrim(uint32_t lPrim)
    {
        // TODO: Finish me
        return nullptr;
    }

    uint32_t ZPrimControlBase::GetMaterialIdFromPrim(uint32_t lPrim)
    {
        // TODO: Finish me
        return 0;
    }

    uint32_t ZPrimControlBase::CreateSpriteArray(uint32_t lTextureId, uint32_t lDrawMode, SPRITETYPE lSpriteType, bool bSaveLoadThisPrim)
    {
        // TODO: Finish me
        return 0;
    }

    uint32_t ZPrimControlBase::CopyBasePrim(uint32_t lPrim)
    {
        // TODO: Finish me
        return 0;
    }

    uint32_t ZPrimControlBase::GetBasePrim(uint32_t lPrim)
    {
        // TODO: Finish me
        return 0;
    }

    bool ZPrimControlBase::CalcPrimCenSize(uint32_t lPrim, float* pvCen, float* pvSize, bool bIncludeLights)
    {
        // TODO: Finish me
        return false;
    }

    bool ZPrimControlBase::CalcPrimCenSizeAlongMat(uint32_t lPrim, float* pvCen, float* pvSize, float* mMat, bool bIncludeLights)
    {
        // TODO: Finish me
        return false;
    }

    uint32_t ZPrimControlBase::GetSizeOfLightOmni() const
    {
        // TODO: Finish me
        return 0u;
    }

    uint32_t ZPrimControlBase::GetSizeOfLightSpot() const
    {
        // TODO: Finish me
        return 0u;
    }

    uint32_t ZPrimControlBase::GetSizeOfLightSpotSquare() const
    {
        // TODO: Finish me
        return 0u;
    }

    uint32_t ZPrimControlBase::GetSizeOfLightEnvironment() const
    {
        // TODO: Finish me
        return 0u;
    }

    void ZPrimControlBase::ColorIUTOU(uint32_t* pDst, const uint32_t* pSrc)
    {
        // TODO: Finish me
    }

    void ZPrimControlBase::ColorUTOIU(uint32_t* pDst, const uint32_t* pSrc)
    {
        // TODO: Finish me
    }

    void ZPrimControlBase::ColorFVTOIU(uint32_t* pDst, const float* pSrc)
    {
        // TODO: Finish me
    }

    void ZPrimControlBase::ColorIUTOFV(float* pDst, const uint32_t* pSrc)
    {
        // TODO: Finish me
    }

    void ZPrimControlBase::ColorUTOFV(float* pDst, const uint32_t* pSrc)
    {
        // TODO: Finish me
    }

    float ZPrimControlBase::GetLightIntensity(const uint32_t lPrim, const float fDistance)
    {
        // TODO: Finish me
        return 0.f;
    }

    uint32_t ZPrimControlBase::CombinePrimVariants(const uint32_t* aOriginalPrims, uint32_t lOriginalPrimsCount, const SPrimVariant* aVariants, uint32_t lVariantCount)
    {
        // TODO: Finish me
        return 0u;
    }

    void ZPrimControlBase::ModifyLight(SPrimLight* pDst, const SPrimLight* pSrc)
    {
        // TODO: Finish me
    }

    void ZPrimControlBase::ModifyEnvironmentLight(SPrimLightEnvironment* pDst, const SPrimLightEnvironment* pSrc)
    {
        // TODO: Finish me
    }

    void ZPrimControlBase::GetLightData(SPrimLight* pDst, const SPrimLight* pSrc)
    {
        // TODO: Finish me
    }

    void ZPrimControlBase::GetLightOmniData(SPrimLightOmni* pDstOmni, const SPrimLightOmni* pSrcOmni)
    {
        // TODO: Finish me
    }

    void ZPrimControlBase::GetLightSpotData(SPrimLightSpot* pDstSpot, const SPrimLightSpot* pSrcSpot)
    {
        // TODO: Finish me
    }

    void ZPrimControlBase::GetLightSpotSquareData(SPrimLightSpotSquare* pDstSpotSquare, const SPrimLightSpotSquare* pSrcSpotSquare)
    {
        // TODO: Finish me
    }

    void ZPrimControlBase::GetLightEnvironmentData(SPrimLightEnvironment* pDst, const SPrimLightEnvironment* pSrc)
    {
        // TODO: Finish me
    }

    bool ZPrimControlBase::CalcLightCenSize(const SPrims *pPrim, float *vCenter, float *vSize)
    {
        // TODO: Finish me
        return false;
    }

    uint32_t ZPrimControlBase::CreateVariantPrim(const SPrimVariant *pVariant)
    {
        // TODO: Finish me
        return 0u;
    }

    void ZPrimControlBase::AppendPrims(uint32_t lFirstPrim, uint32_t lLastPrim)
    {
        // TODO: Finish me
    }

    void ZPrimControlBase::UpdateStripBounds(uint32_t lPrim)
    {
        // TODO: Finish me
    }

    ZPrimControlBase* ZPrimControlBase::Instance()
    {
        if (!g_pRenderDll) return nullptr;

        return g_pRenderDll->m_pPrimControl;
    }

    SSpriteArray* ZPrimControlBase::AllocSpriteArrays(uint32_t lNrSpriteArrays)
    {
        // TODO: Finish me
        return nullptr;
    }

    SSpriteArrayElementUV* ZPrimControlBase::AllocSpriteArrayUV(uint32_t lNrSprites)
    {
        // TODO: Finish me
        return nullptr;
    }

    uint32_t* ZPrimControlBase::AllocPrimList(uint32_t lNrPrims)
    {
        // TODO: Finish me
        return nullptr;
    }

    void ZPrimControlBase::FreeSpriteArrays(SSpriteArray* pSpriteArrays, uint32_t lNrSpriteArrays)
    {
        // TODO: Finish me
    }

    void ZPrimControlBase::FreePrimList(uint32_t* pPrimList, uint32_t lNrPrims)
    {
        // TODO: Finish me
    }

    void ZPrimControlBase::FreeSpriteArrayUV(SSpriteArrayElementUV *pSpriteArray, uint32_t lNrSprites)
    {
        // TODO: Finish me
    }
}