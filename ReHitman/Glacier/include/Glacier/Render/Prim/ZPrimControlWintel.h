#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>


namespace Glacier
{
    class ZPrimControlWintel : public ZPrimControlBase
    {
    public:
        // vtbl
        void GetPrimExtraData(uint32_t lPrim, uint32_t lFlag, void* pData, uint32_t lCount) override;
        void GetPrimTriangles(REFTAB* ppTris, SPrims* pPrims, uint32_t lPrim) override;
        void GetPrimTriangles(REFTAB* ppTris, uint32_t lPrim) override;
        void GetPrimInfoString(char* pInfo, uint32_t lInfoMaxSize, uint32_t lPrim) override;
        bool HasKeys(uint32_t lPrim) override;
        uint32_t GetPrimSize(const SPrims* lPrim) override;
        uint32_t CopySinglePrim(uint32_t lPrim, uint32_t lFlags) override;
        uint32_t GetNumEntries(uint32_t lPrim, uint32_t lFlags) override;
        uint32_t GetNrBones(uint32_t lPrim) override;
        const float* GetLocalPrimBones(uint32_t lPrim) override;
        const float* GetLocalPrimBonesQuats(uint32_t lPrim) override;
        const float* GetGlobalPrimBones(uint32_t lPrim) override;
        const float* GetConvBones(uint32_t lPrim) override;
        const SBoneDefinition* GetBoneDefinitions(uint32_t lPrim) override;
        const ZBoneConstraintsHeader* GetBoneConstraints(uint32_t lPrim) override;
        const uint8_t* GetBoneIdToIndexLookup(uint32_t lPrim) override;
        uint32_t GetVerticesXYZ(uint32_t lPrim, float *pVertices, uint32_t lVerticesNr) override;
        void GetVerticesColor(uint32_t lPrim, uint32_t*, uint32_t lColorsNr) override;
        uint32_t GetPrimVertices(uint32_t lPrim, float* pVertices, uint32_t lVerticesNr) override;
        bool GetPrimVertAnim(uint32_t lPrim, uint32_t* pFrameStart, uint32_t* pFrameStep, uint32_t* pNrKeys) override;
        bool GetPrimTextureAnimData(uint32_t lPrim, uint32_t* pFrameCount) override;
        void SetPrimTextureFrameNr(const ZBaseGeom* pBaseGeom, float fFrameNr) override;
        void CopyPrimDrawMode(uint32_t lDestPrim, uint32_t lSourcePrim) override;
        void SetActiveNrTriangles(uint32_t lPrim, uint32_t lNrTriangles) override;
        uint32_t GetActiveNrTriangles(uint32_t lPrim) override;
        const float* GetPrimSplineVertices(uint32_t lPrim) override;
        uint32_t* GetPrimSplineIndices(uint32_t lPrim) override;

        ~ZPrimControlWintel() override;

        // methods
        using ZPrimControlBase::ZPrimControlBase;

        // members
    };
}
