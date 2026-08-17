#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    class REFTAB;

    class ZPrimControlBase
    {
    public:
        // vtbl
        virtual void GetPrimExtraData(uint32_t lPrim, uint32_t lFlag, void* pData, uint32_t lCount);
        virtual void GetPrimTriangles(REFTAB* ppTris, SPrims* pPrims, uint32_t lPrim) = 0;
        virtual void GetPrimTriangles(REFTAB* ppTris, uint32_t lPrim) = 0;
        virtual int ShouldUseBoundForLightCheck(uint32_t lPrim);
        virtual uint8_t GetPrimPackType(uint32_t lPrim);
        virtual const void* GetPrimData(uint32_t lPrim);
        virtual uint32_t GetPrimOffset(const void* ptr);
        virtual bool CheckInPrimBuffer(const void* ptr);
        virtual bool IsPrimUnique(uint32_t lPrim);
        virtual bool CheckPointInsidePrim(uint32_t lPrim, const ZVector3& vPoint, float fThreshold);
        virtual bool CheckBoxInsidePrim(uint32_t lPrim, const float* pvBoxP1, const float* pvBoxP2, const float* pvThreshold);
        virtual uint32_t AddPrimDataUnique(const void* pData, uint32_t lDataSize);
        virtual void FreePrimData(uint32_t lPrim);
        virtual void GetPrimInfoString(char* pInfo, uint32_t lInfoMaxSize, uint32_t lPrim);
        virtual uint32_t CountSubPrims(uint32_t lPrim);
        virtual uint32_t GetPrimType(uint32_t lPrim);
        virtual bool HasKeys(uint32_t lPrim) = 0;
        virtual bool IsRigidBones(uint32_t lPrim);
        virtual uint32_t GetPrimSize(const SPrims* lPrim);
        virtual uint32_t GetSubPrim(uint32_t lPrim, uint32_t lSubPrimNumber);
        virtual uint32_t GetSubPrims(uint32_t lPrim, uint32_t* pSubPrims, uint32_t lMaxSubPrims);
        virtual uint32_t CopySinglePrim(uint32_t lPrim, uint32_t lFlags) = 0;
        virtual uint32_t GetNumEntries(uint32_t lPrim, uint32_t lFlags) = 0;
        virtual uint32_t CopyPrim(uint32_t lPrim, uint32_t lFlags);
        virtual void SetPrimExtraData(uint32_t lPrim, uint32_t lFlag, const void* pData, uint32_t lCount);
        virtual void SetSubPrimOnTop(uint32_t lPrim, uint32_t lSubPrim) = 0;
        virtual bool ExtraDataSupport();
        virtual bool IsVariantAvailable(uint32_t lPrim, uint32_t lVariantId);
        virtual uint32_t GetNumVariants(uint32_t lPrim);
        virtual uint32_t CreateUserLight(uint32_t lType);
        virtual bool GetOmniLightData(const uint32_t lPrimId, SPrimLightOmni* pDst);
        virtual bool GetSpotLightData(const uint32_t lPrimId, SPrimLightSpot* pDst);
        virtual bool GetSpotLightSquareData(const uint32_t lPrimId, SPrimLightSpot* pDst);
        virtual bool GetEnvironmentLightData(const uint32_t lPrimId, SPrimLightEnvironment* pDst);
        virtual void ModifyOmniLight(SPrimLightOmni* pDst, const SPrimLightOmni* pSrc);
        virtual void ModifyOmniLight(uint32_t lPrim, const SPrimLightOmni* pSrc);
        virtual void ModifySpotLight(SPrimLightSpot* pDst, const SPrimLightSpot* pSrc);
        virtual void ModifySpotLight(uint32_t lPrim, const SPrimLightSpot* pSrc);
        virtual void ModifySpotLightSquare(SPrimLightSpotSquare* pDst, const SPrimLightSpotSquare* pSrc);
        virtual void ModifySpotLightSquare(uint32_t lPrim, const SPrimLightSpotSquare* pSrc);
        virtual void ModifyEnvironment(uint32_t lPrim, const SPrimLightEnvironment* pSrc);
        virtual uint32_t GetNrBones(uint32_t lPrim) = 0;
        virtual const float* GetLocalPrimBones(uint32_t lPrim) = 0;
        virtual const float* GetLocalPrimBonesQuats(uint32_t lPrim) = 0;
        virtual const float* GetGlobalPrimBones(uint32_t lPrim) = 0;
        virtual const float* GetInvertGlobalPrimBones(uint32_t lPrim);
        virtual const float* GetConvBones(uint32_t lPrim) = 0;
        virtual const SBoneDefinition* GetBoneDefinitions(uint32_t lPrim) = 0;
        virtual const ZBoneConstraintsHeader* GetBoneConstraints(uint32_t lPrim) = 0;
        virtual const uint8_t* GetBoneIdToIndexLookup(uint32_t lPrim) = 0;
        virtual uint32_t GetVerticesXYZ(uint32_t lPrim, float *pVertices, uint32_t lVerticesNr) = 0;
        virtual void GetVerticesColor(uint32_t lPrim, uint32_t*, uint32_t lColorsNr) = 0;
        virtual uint32_t GetPrimVertices(uint32_t lPrim, float* pVertices, uint32_t lVerticesNr) = 0;
        virtual bool GetPrimVertAnim(uint32_t lPrim, uint32_t* pFrameStart, uint32_t* pFrameStep, uint32_t* pNrKeys) = 0;
        virtual void SetPrimFrameNr(const ZBaseGeom* pBaseGeom, float fFrameNr);
        virtual bool GetPrimTextureAnimData(uint32_t lPrim, uint32_t* pFrameCount);
        virtual void SetPrimTextureFrameNr(const ZBaseGeom* pBaseGeom, float fFrameNr);
        virtual uint32_t GetPrimTextureId(uint32_t lPrim, const char* pTextureName);
        virtual uint32_t GetSubPrimUsingTexture(uint32_t lPrim, const char* pTextureName);
        virtual void CopyPrimDrawMode(uint32_t lDestPrim, uint32_t lSourcePrim);
        virtual void SetActiveNrTriangles(uint32_t lPrim, uint32_t lNrTriangles);
        virtual uint32_t GetActiveNrTriangles(uint32_t lPrim);
        virtual ZTextureBase* GetTextureFromPrim(uint32_t lPrim);
        virtual uint32_t GetMaterialIdFromPrim(uint32_t lPrim);
        virtual const float* GetPrimSplineVertices(uint32_t lPrim) = 0;
        virtual uint32_t* GetPrimSplineIndices(uint32_t lPrim) = 0;
        virtual uint32_t CreateSpriteArray(uint32_t lTextureId, uint32_t lDrawMode, SPRITETYPE lSpriteType, bool bSaveLoadThisPrim);
        virtual uint32_t CopyBasePrim(uint32_t lPrim);
        virtual uint32_t GetBasePrim(uint32_t lPrim);
        virtual bool CalcPrimCenSize(uint32_t lPrim, float* pvCen, float* pvSize, bool bIncludeLights);
        virtual bool CalcPrimCenSizeAlongMat(uint32_t lPrim, float* pvCen, float* pvSize, float* mMat, bool bIncludeLights);
        virtual uint32_t GetSizeOfLightOmni() const;
        virtual uint32_t GetSizeOfLightSpot() const;
        virtual uint32_t GetSizeOfLightSpotSquare() const;
        virtual uint32_t GetSizeOfLightEnvironment() const;
        virtual void ColorIUTOU(uint32_t* pDst, const uint32_t* pSrc);
        virtual void ColorUTOIU(uint32_t* pDst, const uint32_t* pSrc);
        virtual void ColorFVTOIU(uint32_t* pDst, const float* pSrc);
        virtual void ColorIUTOFV(float* pDst, const uint32_t* pSrc);
        virtual void ColorUTOFV(float* pDst, const uint32_t* pSrc);
        virtual float GetLightIntensity(const uint32_t lPrim, const float fDistance);
        virtual uint32_t CombinePrimVariants(const uint32_t* aOriginalPrims, uint32_t lOriginalPrimsCount, const SPrimVariant* aVariants, uint32_t lVariantCount);
        virtual void ModifyLight(SPrimLight* pDst, const SPrimLight* pSrc);
        virtual void ModifyEnvironmentLight(SPrimLightEnvironment* pDst, const SPrimLightEnvironment* pSrc);
        virtual void GetLightData(SPrimLight* pDst, const SPrimLight* pSrc);
        virtual void GetLightOmniData(SPrimLightOmni* pDstOmni, const SPrimLightOmni* pSrcOmni);
        virtual void GetLightSpotData(SPrimLightSpot* pDstSpot, const SPrimLightSpot* pSrcSpot);
        virtual void GetLightSpotSquareData(SPrimLightSpotSquare* pDstSpotSquare, const SPrimLightSpotSquare* pSrcSpotSquare);
        virtual void GetLightEnvironmentData(SPrimLightEnvironment* pDst, const SPrimLightEnvironment* pSrc);
        virtual bool CalcLightCenSize(const SPrims *pPrim, float *vCenter, float *vSize);
        virtual uint32_t CreateVariantPrim(const SPrimVariant *pVariant);
        virtual void AppendPrims(uint32_t lFirstPrim, uint32_t lLastPrim);
        virtual void UpdateStripBounds(uint32_t lPrim);
        virtual ~ZPrimControlBase();

        // methods
        static ZPrimControlBase* Instance();

        ZPrimControlBase(bool bUnusedSmth);
        SSpriteArray* AllocSpriteArrays(uint32_t lNrSpriteArrays);
        SSpriteArrayElementUV* AllocSpriteArrayUV(uint32_t lNrSprites);
        uint32_t* AllocPrimList(uint32_t lNrPrims);
        void FreeSpriteArrays(SSpriteArray* pSpriteArrays, uint32_t lNrSpriteArrays);
        void FreePrimList(uint32_t* pPrimList, uint32_t lNrPrims);
        void FreeSpriteArrayUV(SSpriteArrayElementUV *pSpriteArray, uint32_t lNrSprites);
    };
}