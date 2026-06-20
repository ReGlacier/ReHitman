#pragma once

#include <Glacier/ZSTL/CTreeObject.h>
#include <Glacier/ReGlacier.h>


namespace Glacier
{
    struct SRecurseInfoCompiled;
    
    struct COctreeObj : CTreeObject
    {
        // There are no info about that type, just ptr or smth like that
        struct ZNodeBuild* m_pNode;
    };
    RE_VERIFY_SIZE(COctreeObj, 0x1C);

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
}