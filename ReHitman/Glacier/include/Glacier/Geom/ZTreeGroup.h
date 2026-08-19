#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZSTL/CMemPool.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/ZSTL/CQuadtree.h>
#include <Glacier/ZSTL/ZOctree.h>
#include <Glacier/Physics/eGlobalTreeType.h>

namespace Glacier
{
    struct SExtendedImpactInfo;
    struct ZGEOMCLASSINFO;
    class COLI;
    class CHUNK;
    class ZOctree;

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
        virtual void AttachDynamicGeoms(ZBaseGeom *);
        virtual void DetachDynamicGeoms(ZBaseGeom *);
        virtual void AttachTreeGroupChilds(ZBaseGeom *);
        virtual void DetachTreeGroupChilds(ZBaseGeom *);
        virtual void MakeStaticContainer(bool);
        virtual void MakeDynamicContainer(bool);
        virtual bool IsStaticContainer() const;
        virtual bool IsDynamicContainer() const;
        virtual bool IsPrivate() const;
        virtual CQuadtree* GetDynamicTreePtr() const;
        virtual CQuadtreeObj* AddDynamicGeom(ZBaseGeom *);
        virtual void MoveDynamicGeom(ZBaseGeom *);
        virtual bool RemoveDynamicGeom(ZBaseGeom *);
        virtual void CreateDynamicTrees();
        virtual void SaveBoundTrees(CHUNK*, unsigned int);
        virtual void LoadBoundTrees(char* rmc);
        virtual void FreeGlobalBounds();
        virtual void FreeDynamicTrees();
        virtual void RemoveGlobalBound(eGlobalTreeType,ZBaseGeom *,COctreeObj *);
        virtual void MoveGlobalBound(eGlobalTreeType,ZBaseGeom *,COctreeObj *,float *,float *);
        virtual ZOctree* GetGlobalTreePtr(eGlobalTreeType);
        virtual uint32_t GetStaticGeomsInBound(ZBaseGeom** pGeomList, ZBaseGeom** pGeomListEnd, eGlobalTreeType eGTT, const float* mMat, const float* vCen, const float* vSize, uint32_t lGeomConMask, bool bExact);
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
        SChildTreeGroups* m_pChildTreeGroupsLists;
    };
    RE_VERIFY_SIZE(ZTreeGroup, 0x70);
}