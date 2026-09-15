#pragma once

#include <Glacier/Physics/ZCollisionBox.h>
#include <Glacier/Physics/ZOctreeCompiled.h>
#include <Glacier/Physics/eGlobalTreeType.h>
#include <Glacier/Physics/ZRawStrip.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/STempStripsUniqueId.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZUniMemory.h>
#include <type_traits>


namespace Glacier
{
    class ZCollisionBase;

    class ZCollisionPackBase
    {
    public:
        // vtbl
        virtual ~ZCollisionPackBase();
        virtual void* CreateRoomTree(int*, bool, bool);

        // data
        ZCollisionBase* m_pCollisionBase;
    };

    struct SStripLineChk : SRecurseInfoCompiled
    {
        SExtendedImpactInfo *pImpact;
        char *pUniqueSubStripInfo;
        float vStart[3];
        float vVect[3];
        bool bBothSides;
        unsigned int ColiMask;
    };
    RE_VERIFY_SIZE(SStripLineChk, 0x84);

    struct SGeomLineChk : SRecurseInfoCompiled
    {
        ZVector3 vStart;
        ZVector3 vVect;
        unsigned int GeomConMask;
        SStripLineChk *pStripLineChk;
    };
    RE_VERIFY_SIZE(SGeomLineChk, 0x7C);

    struct SStripInsideBoxCubeChk : SRecurseInfoCompiled
    {
        char *pUniqueSubStripInfo;
        char *pMemBuffer;
        char *pMemBufferEnd;
        unsigned int lColiMask;
        unsigned int lNrStrips;
        float mBaseGeomToBox[9];
        float vBaseGeomToBox[3];
        float vBoxDimensions[3];
        ZBaseGeom *pBaseGeom;
        ZCollisionBase *pCollisionBase;
    };
    RE_VERIFY_SIZE(SStripInsideBoxCubeChk, 0xB4); // Verified PC CallBackStripInsideBox

    struct STreeGetRightType : SRecurseInfoCompiled
    {
        // methods
        STreeGetRightType();
        
        // members
        ZBaseGeom **pGeomList;
        ZBaseGeom **pGeomListEnd;
        unsigned int lNrGeomsInList;
        unsigned int lGeomConMask;
    };
    RE_VERIFY_SIZE(STreeGetRightType, 0x6C);

    struct SRoomLineChk : SRecurseInfoCompiled
    {
        // constants
        static constexpr int MAX_WORK_ROOM_NR = 16;

        // members
        ZVector3 vStart;
        ZVector3 vVect;
        eGlobalTreeType eGTT;
        SGeomLineChk *pGeomLineChk;
        char* m_pRoomTreePtr;
        ZROOM* m_pWorkRoomList[MAX_WORK_ROOM_NR];
        uint32_t m_lCurWorkRoom;
        bool m_bWorkRoomsWrapAround;
    };
    RE_VERIFY_SIZE(SRoomLineChk, 0xC8);

    struct STempStrips
    {
        // methods
        STempStrips();

        // members
        struct STempStrips *pNext;
        uint32_t lNrEntries;
        uint32_t lNrStrips;
        uint32_t *pRawEntryIndices;
        float *pVertices;
        float *pNormals;
        float *pUVCds;
        uint32_t *pColors;
        uint32_t *pLengths;
        float *pTangents;
        float *pBinormals;
        uint32_t lTextureId;
        uint32_t lSpecularMapId;
        uint32_t lSpecularColor;
        uint32_t lNormalMapId;
        uint32_t lDrawMode;
        STempStripsUniqueId *pUniqueId;
        uint32_t lColiBits;
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

    struct SUniqueSubStripInfo : STempStripsUniqueId
    {};

    class ZTreeGroup;

    class ZCollisionBase
    {
    public:
        // constants
        static constexpr int UNIQUESUBSTRIPINFOCOUNT = 125000;
        static constexpr int MAX_GEOMS_NR = 2048;

        // static
        STATIC_CLASS_VAR(ZCollisionBase, ZCollisionBase*, s_pCollisionBase);

        // vtbl (order approved by PC 0x007663D4 & XBOX MiniNinjas 0x8200C660 vftables)
        virtual ~ZCollisionBase(); // +0x00
        virtual ZCollisionPackBase* GetCollisionPack(); // +0x04
        virtual void InstallCollisionBuffer(char* pData, int lDataSize); // +0x08
        virtual void InstallInsideBuffer(char* pData, int lDataSize); // +0x0C
        virtual void FreeSceneMemory(); // +0x10
        virtual uint32_t GetRoomsLst(ZROOM** pRoomList, ZROOM** pRoomListEnd, ZOctreeCompiled* pRoomTree, const float* Mat1, const float* Cen1, const float* Size1); // +0x14
        virtual uint32_t GetRoomsLst(ZROOM** pRoomList, ZROOM** pRoomListEnd, ZOctreeCompiled* pRoomTree, const float* vPos); // +0x18
        virtual uint32_t GetInsideRoomsLst(ZROOM** pRoomList, ZROOM** pRoomListEnd, const float* Mat1, const float* Cen1, const float* Size1); // +0x1C
        virtual uint32_t GetColiRoomsLst(ZROOM** pRoomList, ZROOM** pRoomListEnd, const float* Mat1, const float* Cen1, const float* Size1); // +0x20
        virtual uint32_t GetInnerRoomsLst(ZROOM** pRoomList, ZROOM** pRoomListEnd, const float* Mat1, const float* Cen1, const float* Size1, bool bIncludeBackdrop); // +0x24
        virtual bool CalcLineColi(SExtendedImpactInfo* pImpact, ZROOM* pRoom, eGlobalTreeType eGTT, const float* vPos, float* const vVect, bool bBothSides, uint32_t GeomConMask); // +0x28
        virtual bool CalcLineColi(SExtendedImpactInfo* pImpact, eGlobalTreeType eGTT, const float* vPos, const float* vVect, bool bBothSides, uint32_t GeomConMask, bool bCheckStatic, bool bCheckDynamic); // +0x2C
        virtual bool CheckLineColi(SExtendedImpactInfo* pImpact, eGlobalTreeType eGTT, const float* vPos, const float* vVect, bool bBothSides, uint32_t GeomConMask, bool bCheckStatic, bool bCheckDynamic); // +0x30
        virtual uint32_t GetGeomsInBox(ZBaseGeom** pGeomList, ZBaseGeom** pGeomListEnd, eGlobalTreeType eGTT, const float* mMat, const float* vCen, const float* vSize, int GeomConMask, bool bCheckStatic, bool bCheckDynamic, bool bExact); // +0x34
        virtual uint32_t GetGeomsInBoxLocal(ZBaseGeom** pGeomList, ZBaseGeom** pGeomListEnd, ZTreeGroup* pTreeGroup, eGlobalTreeType eGTT, const float* mMat, const float* vCen, const float* vSize, int GeomConMask, bool bCheckStatic, bool bCheckDynamic, bool bExact); // +0x38
        virtual void SaveUniqueSubStripInfo(CHUNK* pChunk); // +0x3C
        virtual void LoadUniqueSubStripInfo(SUniqueSubStripInfo* pData, uint32_t lSize); // +0x40
        virtual bool IsCollisionBoxLocked() const; // +0x44
        virtual ZCollisionBox* LockCollisionBox(char* pMemBuffer, uint32_t lMemBufferSize); // +0x48
        virtual void UnlockCollisionBox(ZCollisionBox* pCollisionBox); // +0x4C
        virtual char* LoadInternColiTree(char* pOctreeBuffer); // +0x50
        virtual bool CalcColiLort(SExtendedImpactInfo* Impact, ZBaseGeom* pDynBaseGeom, eGlobalTreeType eGTT, const float* vLineStart, const float* vLineDirection, uint32_t GeomConMask, bool bBothSides); // +0x54
        virtual bool CalcDynamicLineCollision(SExtendedImpactInfo* Impact, const ZTreeGroup* pTreeGroup, eGlobalTreeType eGTT, const float* vP, const float* vD, bool bBothSides, int GeomConMask); // +0x58
        virtual bool CalcLineCollision(SExtendedImpactInfo* pImpact, uint32_t lPrim, const float* vLineStart, const float* vLineDirection, bool bTwoSided, uint32_t lColiMask) = 0; // +0x5C
        virtual bool CalcLineCollision(SExtendedImpactInfo* pImpact, const STempStripsUniqueId* pStripId, const float* vLineStart, const float* vLineDirection, bool bTwoSided, uint32_t lColiMask) = 0; // +0x60
        virtual bool CalcLineCollision(SExtendedImpactInfo* pImpact, ZBaseGeom* pBaseGeom, float* vLineStart, float* vLineDirection, bool bTwoSided, uint32_t lColiMask); // +0x64
        virtual bool CheckSphereCollision(uint32_t lPrim, uint8_t lColiMask, const float* vSpherePosition, const float* vSphereMatrix, const float* vSphereDimensions) = 0; // +0x68
        virtual void CreateSubStripId(STempStripsUniqueId* pDst, const STempStripsUniqueId* pSrc, const uint32_t lCount) = 0; // +0x6C
        virtual STempStrips* GetStripsFromPrim(const uint32_t lPrimId) = 0; // +0x70
        virtual uint32_t GetPrimFromStripUniqueId(const STempStripsUniqueId* pId) = 0; // +0x74
        virtual uint32_t GetColiBits(const STempStripsUniqueId* pId) = 0; // +0x78
        virtual uint32_t FindMaterialDescriptor(uint32_t lPrim); // +0x7C
        virtual uint32_t GetDynamicGeomsInBox(ZBaseGeom** pGeomList, ZBaseGeom** pGeomListEnd, eGlobalTreeType eGTT, const float* mMat, const float* vCen, const float* vSize, uint32_t lGeomConMask, bool bExact); // +0x80
        virtual uint32_t GetDynamicGeomsInBoxInRooms(ZBaseGeom** pGeomList, ZBaseGeom** pGeomListEnd, eGlobalTreeType eGTT, ZROOM** pRoomList, uint32_t lNrRooms, const float* mMat, const float* vCen, const float* vSize, uint32_t lGeomConMask, bool bExact); // +0x84
        virtual bool GetStripsInsideBox(uint32_t* pNrStrips, char* pMemBuffer, uint32_t lMemBufferSize, const float* mBoxMatrix, const float* vBoxPosition, const float* vBoxDimensions, uint32_t lColiMask, bool bCheckStatic, bool bCheckDynamic, bool bIgnoreActors, eGlobalTreeType eGTT); // +0x88
        virtual char* GetStripVerticesFromId(ZRawStrip* pRawStrips, const STempStripsUniqueId* pId, uint32_t lColiMask, const float* m0, const float* p0, const float* s0) = 0; // +0x8C

        // methods
        ZCollisionBase(bool bPackEnabled);

        bool CalcLineColi_(SExtendedImpactInfo *pImpact, eGlobalTreeType eGTT, const float* vPos, const float* vVect, bool bBothSides, uint32_t GeomConMask, bool bCheckStatic, bool bCheckDynamic);

        static ZCollisionBase* GetCollisionInterface();
        static ZCollisionBase* InitCollision(bool bPackEnabled);

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
        ZBaseGeom* m_GeomList[MAX_GEOMS_NR];
    };
    RE_VERIFY_SIZE(ZCollisionBase, 0x20B4); // Approved by PC allocation size

    RE_VERIFY_OFFSET(ZCollisionBase, m_pCollisionPackBase, 0xB0); // Verified by ZCollisionBase::GetCollisionPack
    RE_VERIFY_OFFSET(ZCollisionBase, m_pRoomColiTree, 0xA8); // Approved by ZCollisionBase::InstallCollisionBuffer (PC 0x4BDCB0)
    RE_VERIFY_OFFSET(ZCollisionBase, m_pRoomInsideTree, 0xAC); // Approved by ZCollisionBase::InstallInsideBuffer (PC 0x4BD7F0)
}
