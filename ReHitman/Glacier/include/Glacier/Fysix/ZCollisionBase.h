#pragma once

#include <Glacier/Fysix/ZCollisionBox.h>
#include <Glacier/Fysix/ZOctreeCompiled.h>
#include <Glacier/Fysix/eGlobalTreeType.h>
#include <Glacier/Fysix/ZRawStrip.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/STempStripsUniqueId.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZUniMemory.h>
#include <type_traits>


namespace Glacier
{
    struct ZCollisionBase;

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
        // static
        STATIC_CLASS_VAR(ZCollisionBase, ZCollisionBase*, s_pCollisionBase);

        // vtbl (order approved by PC 0x007663D4 & XBOX MiniNinjas 0x8200C660 vftables)
        virtual ~ZCollisionBase();                                                                                                          // +0x00
        virtual ZCollisionPackBase* GetCollisionPack();                                                                                     // +0x04
        virtual void InstallCollisionBuffer(char* pBuffer, int nSize);                                                                      // +0x08
        virtual void InstallInsideBuffer(char* pBuffer, int nSize);                                                                         // +0x0C
        virtual void FreeSceneMemory();                                                                                                     // +0x10
        virtual unsigned int GetRoomsLst(ZROOM** ppRooms1, ZROOM** ppRooms2, ZOctreeCompiled* pOctree, const float* pfPos, const float* pfDir, const float* pfExtents); // +0x14
        virtual unsigned int GetRoomsLst(ZROOM** ppRooms1, ZROOM** ppRooms2, ZOctreeCompiled* pOctree, const float* pfPos);                    // +0x18
        virtual unsigned int GetInsideRoomsLst(ZROOM** ppRooms1, ZROOM** ppRooms2, const float* pfMin, const float* pfMax, const float* pfUnk); // +0x1C
        virtual unsigned int GetColiRoomsLst(ZROOM** ppRooms1, ZROOM** ppRooms2, const float* pfPos, const float* pfRadius, const float* pfUnk); // +0x20
        virtual unsigned int GetInnerRoomsLst(ZROOM** ppRooms1, ZROOM** ppRooms2, const float* pfMin, const float* pfMax, const float* pfUnk, bool bStrict); // +0x24
        virtual bool CalcLineColi(SExtendedImpactInfo* pImpactInfo, ZROOM* pRoom, eGlobalTreeType treeType, float* const pfFrom, float* const pfTo, bool bUnk, unsigned int uiFlags); // +0x28
        virtual bool CalcLineColi(SExtendedImpactInfo* pImpactInfo, eGlobalTreeType treeType, float* const pfFrom, float* const pfTo, bool bUnk1, unsigned int uiFlags, bool bUnk2, bool bUnk3); // +0x2C
        virtual bool CheckLineColi(SExtendedImpactInfo* pImpactInfo, eGlobalTreeType treeType, float* const pfFrom, float* const pfTo, bool bUnk1, unsigned int uiFlags, bool bUnk2, bool bUnk3); // +0x30
        virtual unsigned int GetGeomsInBox(ZBaseGeom** ppGeomsOut1, ZBaseGeom** ppGeomsOut2, eGlobalTreeType treeType, const float* pfCenter, const float* pfExtents, const float* pfRotation, int nMaxGeoms, bool bUnk1, bool bUnk2, bool bUnk3); // +0x34
        virtual unsigned int GetGeomsInBoxLocal(ZBaseGeom** ppGeomsOut1, ZBaseGeom** ppGeomsOut2, ZTreeGroup* pTreeGroup, eGlobalTreeType treeType, const float* pfCenter, const float* pfExtents, const float* pfRotation, int nMaxGeoms, bool bUnk1, bool bUnk2, bool bUnk3); // +0x38
        virtual void SaveUniqueSubStripInfo(CHUNK* pChunk);                                                                                 // +0x3C
        virtual void LoadUniqueSubStripInfo(char* pData, unsigned int uiSize);                                                              // +0x40
        virtual bool IsCollisionBoxLocked() const;                                                                                          // +0x44
        virtual ZCollisionBox* LockCollisionBox(char* pData, unsigned int uiSize);                                                          // +0x48
        virtual void UnlockCollisionBox(ZCollisionBox* pBox);                                                                               // +0x4C
        virtual char* LoadInternColiTree(char* pBuffer);                                                                                    // +0x50
        virtual bool CalcColiLort(SExtendedImpactInfo* pImpactInfo, ZBaseGeom* pGeom, eGlobalTreeType treeType, const float* pfFrom, const float* pfTo, unsigned int uiFlags, bool bUnk); // +0x54
        virtual bool CalcDynamicLineCollision(SExtendedImpactInfo* pImpactInfo, const ZTreeGroup* pTreeGroup, eGlobalTreeType treeType, float* const pfFrom, float* const pfTo, bool bUnk, int nFlags); // +0x58
        virtual bool CalcLineCollision(SExtendedImpactInfo* pImpactInfo, unsigned int uiUnk, const float* pfFrom, const float* pfTo, bool bUnk, unsigned int uiFlags) = 0; // +0x5C
        virtual bool CalcLineCollision(SExtendedImpactInfo* pImpactInfo, const STempStripsUniqueId* pStripId, const float* pfFrom, const float* pfTo, bool bUnk, unsigned int uiFlags) = 0; // +0x60
        virtual bool CalcLineCollision(SExtendedImpactInfo* pImpactInfo, ZBaseGeom* pGeom, float* pfFrom, float* pfTo, bool bUnk, unsigned int uiFlags); // +0x64
        virtual bool CheckSphereCollision(unsigned int uiUnk1, unsigned char ucUnk2, const float* pfCenter, const float* pfRadius, const float* pfUnk) = 0; // +0x68
        virtual void CreateSubStripId(STempStripsUniqueId* pOutId, const STempStripsUniqueId* pInId, const unsigned int uiUnk) = 0;           // +0x6C
        virtual STempStrips* GetStripsFromPrim(const unsigned int uiPrimId) = 0;                                                            // +0x70
        virtual unsigned int GetPrimFromStripUniqueId(const STempStripsUniqueId* pStripId) = 0;                                             // +0x74
        virtual unsigned int GetColiBits(const STempStripsUniqueId* pStripId) = 0;                                                          // +0x78
        virtual unsigned int FindMaterialDescriptor(unsigned int uiMaterialId);                                                             // +0x7C
        virtual unsigned int GetDynamicGeomsInBox(ZBaseGeom** ppGeomsOut1, ZBaseGeom** ppGeomsOut2, eGlobalTreeType treeType, const float* pfCenter, const float* pfExtents, const float* pfRotation, unsigned int uiFlags, bool bUnk) = 0; // +0x80
        virtual unsigned int GetDynamicGeomsInBoxInRooms(ZBaseGeom** ppGeomsOut1, ZBaseGeom** ppGeomsOut2, eGlobalTreeType treeType, ZROOM** ppRooms, unsigned int uiRoomCount, const float* pfCenter, const float* pfExtents, const float* pfRotation, unsigned int uiFlags, bool bUnk) = 0; // +0x84
        virtual bool GetStripsInsideBox(unsigned int* pOutCount, char* pOutBuffer, unsigned int uiBufferSize, const float* pfRotation, const float* pfCenter, const float* pfExtents, unsigned int uiFlags, bool bUnk1, bool bUnk2, bool bUnk3, eGlobalTreeType treeType); // +0x88
        virtual char* GetStripVerticesFromId(ZRawStrip* pOutRawStrip, const STempStripsUniqueId* pStripId, unsigned int uiUnk, const float* pfCenter, const float* pfExtents, const float* pfRotation) = 0; // +0x8C

        // methods
        static ZCollisionBase* GetCollisionInterface();

        template <typename T>
        static T* GetCollisionInterface() requires(std::is_base_of_v<ZCollisionBase, T>)
        {
            return reinterpret_cast<T*>(GetCollisionInterface());
        }

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
    RE_VERIFY_SIZE(ZCollisionBase, 0x20B4); // Approved by PC allocation size

    RE_VERIFY_OFFSET(ZCollisionBase, m_pRoomColiTree, 0xA8); // Approved by ZCollisionBase::InstallCollisionBuffer (PC 0x4BDCB0)
    RE_VERIFY_OFFSET(ZCollisionBase, m_pRoomInsideTree, 0xAC); // Approved by ZCollisionBase::InstallInsideBuffer (PC 0x4BD7F0)
}
