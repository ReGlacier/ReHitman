#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZSTL/CMemPool.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/ZSTL/CQuadtree.h>
#include <Glacier/ZSTL/ZOctree.h>
#include <Glacier/Physics/eGlobalTreeType.h>
#include <Glacier/Runtime/Macro.h>


namespace Glacier
{
    // fwds
    struct SExtendedImpactInfo;
    struct ZGEOMCLASSINFO;
    struct COLI;
    struct CHUNK;
    class ZOctree;
    class ZTreeGroup;

    struct SChildTreeGroups
    {
        uint16_t m_iNrStaticChildTreeGroups;
        uint16_t m_iNrDynamicChildTreeGroups;
        ZTreeGroup* m_pStaticChildTreeGroups[384];
        ZTreeGroup* m_pDynamicChildTreeGroups[32];
    };
    // RE_VERIFY_SIZE(SChildTreeGroups, 0x684);

    class ZTreeGroup : public ZGROUP
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZTreeGroup, 0x100020u);

        // constants
        static constexpr int32_t GLOBAL_TREES_NR = eGlobalTreeType::GT_SIZE;

        // vtbl
        ~ZTreeGroup() override;

        // ZSerializable
        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void ClassInit() override;
        void CopyData(const ZGEOM* Source) override;

        // ZGROUP

        // ZTreeGroup
        virtual void MakePrivate(bool bPrivate);
        virtual void AddChildTreeGroup(ZTreeGroup* pTreeGroup);
        virtual void RemoveChildTreeGroup(ZTreeGroup* pTreeGroup);
        virtual uint16_t GetStaticChildTreeGroupList(ZTreeGroup*** pTreeGroupPtr);
        virtual uint16_t GetDynamicChildTreeGroupList(ZTreeGroup*** pTreeGroupPtr);
        virtual void AddZGEOMBoxLst(eGlobalTreeType eGTT, REFTAB* pGeomRefTab, const ZMat3x3& mMat, const ZVector3& vCen, const ZVector3& vSize, int32_t GeomConMask, int32_t lGeomType, bool bCheckDynamic);
        virtual bool ChkLineColi(COLI* pColi, eGlobalTreeType eGTT, int32_t GeomConMask, int32_t lGeomType, bool bCheckStatic, bool bCheckDynamic);
        virtual bool CheckDynamicLineColi(eGlobalTreeType eGTT, float*, float*, bool ,int , int);
        virtual bool CalcDynamicLineColiRecur(SExtendedImpactInfo* Impact, eGlobalTreeType eGTT, const ZVector3& vP, const ZVector3& vD, bool bBothSides, uint32_t GeomConMask, ZGEOMCLASSINFO* pGeomClassInfo);
        virtual bool CalcDynamicLineColi(SExtendedImpactInfo* Impact, eGlobalTreeType eGTT, const ZVector3& vP, const ZVector3& vD, bool bBothSides, uint32_t GeomConMask, ZGEOMCLASSINFO* pGeomClassInfo);
        virtual void AttachDynamicGeoms(ZBaseGeom* pBaseGeom);
        virtual void DetachDynamicGeoms(ZBaseGeom* pBaseGeom);
        virtual void AttachTreeGroupChilds(ZBaseGeom* pBaseGeom);
        virtual void DetachTreeGroupChilds(ZBaseGeom* pBaseGeom);
        virtual void MakeStaticContainer(bool bStaticContainer);
        virtual void MakeDynamicContainer(bool bDynamicContainer);
        virtual bool IsStaticContainer() const;
        virtual bool IsDynamicContainer() const;
        virtual bool IsPrivate() const;
        virtual CQuadtree* GetDynamicTreePtr() const;
        virtual CQuadtreeObj* AddDynamicGeom(ZBaseGeom* pDynBaseGeom);
        virtual void MoveDynamicGeom(ZBaseGeom* pDynBaseGeom);
        virtual bool RemoveDynamicGeom(ZBaseGeom* pDynBaseGeom);
        virtual void CreateDynamicTrees();
        virtual uint32_t SaveBoundTrees(CHUNK* pChunk, uint32_t lTreeOffset);
        virtual char* LoadBoundTrees(char* pOctreeBuffer);
        virtual void FreeGlobalBounds();
        virtual void FreeDynamicTrees();
        virtual void RemoveGlobalBound(eGlobalTreeType eGTT, ZBaseGeom* pBaseGeom, COctreeObj* pMMLink);
        virtual void MoveGlobalBound(eGlobalTreeType eGTT, ZBaseGeom* pBaseGeom, COctreeObj* pMMLink, ZVector3& vMin, ZVector3& vMax);
        virtual ZOctree* GetGlobalTreePtr(eGlobalTreeType eGTT);
        virtual uint32_t GetStaticGeomsInBound(ZBaseGeom** pGeomList, ZBaseGeom** pGeomListEnd, eGlobalTreeType eGTT, const ZMat3x3& mMat, const ZVector3& vCen, const ZVector3& vSize, uint32_t lGeomConMask, bool bExact);
        virtual void SetDynamicContainer(const bool& dynamic_container);
        virtual void SetStaticContainer(const bool& static_container);

        // methods
        ZTreeGroup(const char* psName, ZBaseGeom* pBaseGeom);

        float GetOriginAndSize(ZVector3& vCen);
        void AddStaticChildTreeGroup(ZTreeGroup* pTreeGroup);
        void RemoveStaticChildTreeGroup(ZTreeGroup* pTreeGroup);
        void AddDynamicChildTreeGroup(ZTreeGroup* pTreeGroup);
        void RemoveDynamicChildTreeGroup(ZTreeGroup* pTreeGroup);
        void InitGlobalBounds();

#       pragma region " --- RTTI Methods --- "
        void GetDynamicContainer(bool& bIsDynamic);
        void Proxy_SetDynamicContainer(const bool& bIsDynamic);
        void GetIsPrivateContainer(bool& bIsPrivate);
        void SetIsPrivateContainer(const bool& bIsPrivate);
        void IsStaticContainer(bool& bIsStatic);
        void SetIsStaticContainer(const bool& bIsStatic);
#       pragma endregion

        //data (total size is 0x70, base size is 0x4C)
        bool m_bPrivate;
        RE_ADD_PADDING(3);
        ZOctree* m_pGlobalTrees[GLOBAL_TREES_NR];
        CQuadtree* m_pDynamicTree;
        bool m_bStaticContainer;
        bool m_bDynamicContainer;
        RE_ADD_PADDING(2);
        SChildTreeGroups* m_pChildTreeGroupsLists;
    };
    RE_VERIFY_SIZE(ZTreeGroup, 0x70);
}
