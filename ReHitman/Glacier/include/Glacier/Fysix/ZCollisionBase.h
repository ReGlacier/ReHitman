#pragma once

#include <Glacier/Fysix/ZCollisionBox.h>
#include <Glacier/Fysix/ZOctreeCompiled.h>
#include <Glacier/Fysix/eGlobalTreeType.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/STempStripsUniqueId.h>
#include <Glacier/GlacierFWD.h>


namespace Glacier
{
    struct ZCollisionBase;

    struct ZRawStrip
    {
        uint32_t m_lLength;
        uint32_t m_lTriangleStartNr;
        STempStripsUniqueId m_HitCache;
        ZBaseGeom* m_pBaseGeom;
    };
    RE_VERIFY_SIZE(ZRawStrip, 0x18);

    struct ZCollisionPackBase
    {
        // vtbl
        virtual ~ZCollisionPackBase();
        virtual void* CreateRoomTree(int*, bool, bool);

        // data
        ZCollisionBase* m_pCollisionBase;
    };

    struct STempStrips
    {
        struct STempStrips *pNext;
        unsigned int lNrEntries;
        unsigned int lNrStrips;
        unsigned int *pRawEntryIndices;
        float *pVertices;
        float *pNormals;
        float *pUVCds;
        unsigned int *pColors;
        unsigned int *pLengths;
        float *pTangents;
        float *pBinormals;
        unsigned int lTextureId;
        unsigned int lSpecularMapId;
        unsigned int lSpecularColor;
        unsigned int lNormalMapId;
        unsigned int lDrawMode;
        STempStripsUniqueId *pUniqueId;
        unsigned int lColiBits;
        float fAngle1;
        float fOpacity1;
        float fAngle2;
        float fOpacity2;
        float fNear1;
        float fNear2;
        float fFar1;
        float fFar2;
        float fDisableDot3AtDist;
        float fDisableSpecularAtDist;
    };
    RE_VERIFY_SIZE(STempStrips, 0x70);

    class ZTreeGroup;

    struct ZCollisionBase
    {
        // vtbl
        virtual ~ZCollisionBase(); 
        virtual ZCollisionPackBase* GetCollisionPack(); 
        virtual void InstallCollisionBuffer(char* pBuffer, int nSize); 
        virtual void InstallInsideBuffer(char* pBuffer, int nSize); 
        virtual void FreeSceneMemory(); 
        virtual unsigned int GetRoomsLst(ZROOM** ppRooms1, ZROOM** ppRooms2, ZOctreeCompiled* pOctree, const float* pfPos, const float* pfDir, const float* pfExtents); 
        virtual unsigned int GetRoomsLst(ZROOM** ppRooms1, ZROOM** ppRooms2, ZOctreeCompiled* pOctree, const float* pfPos); 
        virtual unsigned int GetInsideRoomsLst(ZROOM** ppRooms1, ZROOM** ppRooms2, const float* pfMin, const float* pfMax, const float* pfUnk); 
        virtual unsigned int GetColiRoomsLst(ZROOM** ppRooms1, ZROOM** ppRooms2, const float* pfPos, const float* pfRadius, const float* pfUnk); 
        virtual unsigned int GetInnerRoomsLst(ZROOM** ppRooms1, ZROOM** ppRooms2, const float* pfMin, const float* pfMax, const float* pfUnk, bool bStrict); 
        virtual bool CalcLineColi(SExtendedImpactInfo* pImpactInfo, ZROOM* pRoom, eGlobalTreeType treeType, float* const pfFrom, float* const pfTo, bool bUnk, unsigned int uiFlags); 
        virtual bool CalcLineColi(SExtendedImpactInfo* pImpactInfo, eGlobalTreeType treeType, float* const pfFrom, float* const pfTo, bool bUnk1, unsigned int uiFlags, bool bUnk2, bool bUnk3); 
        virtual bool CheckLineColi(SExtendedImpactInfo* pImpactInfo, eGlobalTreeType treeType, float* const pfFrom, float* const pfTo, bool bUnk1, unsigned int uiFlags, bool bUnk2, bool bUnk3); 
        virtual unsigned int GetGeomsInBox(ZBaseGeom** ppGeomsOut1, ZBaseGeom** ppGeomsOut2, eGlobalTreeType treeType, const float* pfCenter, const float* pfExtents, const float* pfRotation, int nMaxGeoms, bool bUnk1, bool bUnk2, bool bUnk3); 
        virtual unsigned int GetGeomsInBoxLocal(ZBaseGeom** ppGeomsOut1, ZBaseGeom** ppGeomsOut2, ZTreeGroup* pTreeGroup, eGlobalTreeType treeType, const float* pfCenter, const float* pfExtents, const float* pfRotation, int nMaxGeoms, bool bUnk1, bool bUnk2, bool bUnk3); 
        virtual void SaveUniqueSubStripInfo(CHUNK* pChunk);
        virtual void LoadUniqueSubStripInfo(char* pData, unsigned int uiSize); 
        virtual bool IsCollisionBoxLocked(); 
        virtual ZCollisionBox* LockCollisionBox(char* pData, unsigned int uiSize); 
        virtual void UnlockCollisionBox(ZCollisionBox* pBox); 
        virtual char* LoadInternColiTree(char* szPath, bool bUnk); 
        virtual bool CalcColiLort(SExtendedImpactInfo* pImpactInfo, ZBaseGeom* pGeom, eGlobalTreeType treeType, const float* pfFrom, const float* pfTo, unsigned int uiFlags, bool bUnk);
        virtual bool CalcDynamicLineCollision(SExtendedImpactInfo* pImpactInfo, const ZTreeGroup* pTreeGroup, eGlobalTreeType treeType, float* const pfFrom, float* const pfTo, bool bUnk, int nFlags);
        virtual bool CalcLineCollision(SExtendedImpactInfo* pImpactInfo, unsigned int uiUnk, const float* pfFrom, const float* pfTo, bool bUnk, unsigned int uiFlags); 
        virtual bool CalcLineCollision(SExtendedImpactInfo* pImpactInfo, const STempStripsUniqueId* pStripId, const float* pfFrom, const float* pfTo, bool bUnk, unsigned int uiFlags);
        virtual bool CalcLineCollision(SExtendedImpactInfo* pImpactInfo, ZBaseGeom* pGeom, float* pfFrom, float* pfTo, bool bUnk, unsigned int uiFlags);
        virtual bool CheckSphereCollision(unsigned int uiUnk1, unsigned char ucUnk2, const float* pfCenter, const float* pfRadius, const float* pfUnk);
        virtual void CreateSubStripId(STempStripsUniqueId* pOutId, const STempStripsUniqueId* pInId, const unsigned int uiUnk);
        virtual STempStrips* GetStripsFromPrim(const unsigned int uiPrimId);
        virtual unsigned int GetPrimFromStripUniqueId(const STempStripsUniqueId* pStripId);
        virtual unsigned int GetColiBits(const STempStripsUniqueId* pStripId); 
        virtual unsigned int FindMaterialDescriptor(unsigned int uiMaterialId); 
        virtual unsigned int GetDynamicGeomsInBox(ZBaseGeom** ppGeomsOut1, ZBaseGeom** ppGeomsOut2, eGlobalTreeType treeType, const float* pfCenter, const float* pfExtents, const float* pfRotation, unsigned int uiFlags, bool bUnk); 
        virtual unsigned int GetDynamicGeomsInBoxInRooms(ZBaseGeom** ppGeomsOut1, ZBaseGeom** ppGeomsOut2, eGlobalTreeType treeType, ZROOM** ppRooms, unsigned int uiRoomCount, const float* pfCenter, const float* pfExtents, const float* pfRotation, unsigned int uiFlags, bool bUnk); 
        virtual bool GetStripsInsideBox(unsigned int* pOutCount, char* pOutBuffer, unsigned int uiBufferSize, const float* pfCenter, const float* pfExtents, const float* pfRotation, unsigned int uiFlags, bool bUnk1, bool bUnk2, eGlobalTreeType treeType); 
        virtual char* GetStripVerticesFromId(ZRawStrip* pOutRawStrip, const STempStripsUniqueId* pStripId, unsigned int uiUnk, const float* pfCenter, const float* pfExtents, const float* pfRotation);

        // data
        ZCollisionBox m_CollisionBox;
        STempStripsUniqueId* m_pUniqueSubStripInfo; // in original it's SUniqueSubStripInfo
        uint32_t m_lUniqueSubStripInfoCount;
        REFTAB* m_pBuildOctreeList;
        bool m_bUniqueInfoLoaded;
        ZOctreeCompiled* m_pRoomColiTree;
        ZOctreeCompiled* m_pRoomInsideTree;
        ZCollisionPackBase* m_pCollisionPackBase;
        ZBaseGeom* m_GeomList[2048];
    };
    RE_VERIFY_SIZE(ZCollisionBase, 0x20B4);
}