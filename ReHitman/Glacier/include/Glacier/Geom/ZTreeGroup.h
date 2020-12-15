#pragma once

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
        int m_field4C;
        int m_field50;
        int m_field54;
        int m_field58;
        int m_field5C;
        int m_field60;
        int m_field64;
        int m_field68;
        int m_field6C;
    };
}