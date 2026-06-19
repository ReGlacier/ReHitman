#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/Geom/ZEntityLocator.h>

namespace Glacier
{
    enum eGlobalTreeType {}; ///TODO: Recognize all values
    struct SExtendedImpactInfo;
    class COctreeObj;
    class ZGEOMCLASSINFO;
    class COLI;
    class CHUNK;
    struct SRecurseInfoCompiled;

    struct ZOctree
    {
        using RemapObjectIdFn = unsigned int(*)(unsigned int);

        // Vtbl
        virtual ~ZOctree();
        virtual void lort();
        virtual COctreeObj* AddMinMax(float*, float*, unsigned int, COctreeObj*);
        virtual void Delete(COctreeObj*, bool);
        virtual void* Compile(int*); // Not implemented in release hbm
        virtual void RemapObjects(RemapObjectIdFn);
        virtual bool CheckLinesegment(SRecurseInfoCompiled*, float*, float*);
        virtual void CheckCube(SRecurseInfoCompiled*, float*, float*);
        virtual void GetEverything(SRecurseInfoCompiled*);
        virtual void CheckPoint(SRecurseInfoCompiled*, const float*);

        // Data
        ZVector3 m_vOrigin;
        float m_fScale;
    };
    RE_VERIFY_SIZE(ZOctree, 0x14);

    struct CMemPool
    {
        void* m_pxPool;
        uint16_t* m_pAllocTable;
        int m_iEntries;
        int m_iBlockSize;
        int m_iNumAlloc;
        bool m_bOwnPool;
        bool m_padding[3];
    };
    RE_VERIFY_SIZE(CMemPool, 0x18);

    struct CObjectInfo
    {
        uint32_t iID;
        uint16_t iMinX;
        uint16_t iMinY;
        uint16_t iMinZ;
        uint16_t iMaxX;
        uint16_t iMaxY;
        uint16_t iMaxZ;
    };
    RE_VERIFY_SIZE(CObjectInfo, 0x10);

    struct CTreeObject
    {
        struct CTreeObject* m_pNext;
        struct CTreeObject* m_pPrev;
        CObjectInfo m_tInfo;
    };
    RE_VERIFY_SIZE(CTreeObject, 0x18);

    struct CTreeObjectList
    {
        CTreeObject* m_pHead;
    };
    RE_VERIFY_SIZE(CTreeObjectList, 0x4);

    struct CNodeQuad
    {
        uint16_t m_aiChildren[4];
        uint16_t m_iParent;
        uint16_t m_iDepth;
        CTreeObjectList m_tObjectList;
    };
    RE_VERIFY_SIZE(CNodeQuad, 0x10);

    struct CQuadtree
    {
        CNodeQuad* m_pRoot;
        CMemPool m_tPool;
        int m_iObjects;
        float m_fScale;
        ZVector3 m_vOrigin;
    };

    RE_VERIFY_SIZE(CQuadtree, 0x30);

    struct SChildTreeGroups
    {
        uint16_t m_iNrStaticChildTreeGroups;
        uint16_t m_iNrDynamicChildTreeGroups;
        class ZTreeGroup* m_pStaticChildTreeGroups[384];
        class ZTreeGroup* m_pDynamicChildTreeGroups[32];
    };
    RE_VERIFY_SIZE(SChildTreeGroups, 0x684);

    class ZTreeGroup : public ZGROUP
    {
    public:
        //vftable
        virtual void MakePrivate(bool);
        virtual void AddChildTreeGroup(ZTreeGroup*);
        virtual void RemoveChildTreeGroup(ZTreeGroup*);
        virtual void GetStaticChildTreeGroupList(ZTreeGroup***);
        virtual void GetDynamicChildTreeGroupList(ZTreeGroup***);
        virtual void AddZGEOMBoxLst(eGlobalTreeType,REFTAB *,float const*,float const*,float const*,int,int,bool);
        virtual void ChkLineColi(COLI *,eGlobalTreeType,int,int,bool,bool);
        virtual void CheckDynamicLineColi(eGlobalTreeType,float *,float *,bool,int,int);
        virtual void CalcDynamicLineColiRecur(SExtendedImpactInfo *,eGlobalTreeType,float *,float *,bool,int,ZGEOMCLASSINFO *);
        virtual void CalcDynamicLineColi(SExtendedImpactInfo *,eGlobalTreeType,float *,float *,bool,int,ZGEOMCLASSINFO *);
        virtual void AttachDynamicGeoms(ZEntityLocator *);
        virtual void DetachDynamicGeoms(ZEntityLocator *);
        virtual void AttachTreeGroupChilds(ZEntityLocator *);
        virtual void DetachTreeGroupChilds(ZEntityLocator *);
        virtual void MakeStaticContainer(bool);
        virtual void MakeDynamicContainer(bool);
        virtual bool IsStaticContainer();
        virtual bool IsDynamicContainer();
        virtual bool IsPrivate();
        virtual void* GetDynamicTreePtr();
        virtual void AddDynamicGeom(ZEntityLocator *);
        virtual void MoveDynamicGeom(ZEntityLocator *);
        virtual bool RemoveDynamicGeom(ZEntityLocator *);
        virtual void CreateDynamicTrees();
        virtual void SaveBoundTrees(CHUNK*, unsigned int);
        virtual void LoadBoundTrees(char* rmc);
        virtual void FreeGlobalBounds();
        virtual void FreeDynamicTrees();
        virtual void RemoveGlobalBound(eGlobalTreeType,ZEntityLocator *,COctreeObj *);
        virtual void MoveGlobalBound(eGlobalTreeType,ZEntityLocator *,COctreeObj *,float *,float *);
        virtual void* GetGlobalTreePtr(eGlobalTreeType);
        virtual void* GetStaticGeomsInBound(ZEntityLocator **a2, ZEntityLocator **a3, eGlobalTreeType treeType, Glacier::ZMat3x3* mat, Glacier::ZVector3* a6, Glacier::ZVector3* a7, int rmc, char a9);
        virtual void SetDynamicContainer(bool const&);
        virtual void SetStaticContainer(bool const&);

        //data (total size is 0x70, base size is 0x4C)
        bool m_bPrivate;
        bool m_pad4D[3];
        ZOctree* m_pGlobalTrees[5];
        CQuadtree* m_pDynamicTree;
        bool m_bStaticContainer;
        bool m_bDynamicContainer;
        bool m_pad6A[2];
        struct SChildTreeGroups* m_pChildTreeGroupsLists;
    };
    RE_VERIFY_SIZE(ZTreeGroup, 0x70);
}