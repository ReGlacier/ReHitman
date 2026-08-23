#pragma once

#include <Glacier/ZSTL/CTreeObject.h>
#include <Glacier/ZSTL/ZMath.h>
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

        // vtbl
        virtual void lort();
        virtual COctreeObj* AddMinMax(float*, float*, unsigned int, COctreeObj*);
        virtual void Move(COctreeObj* pObj, float*, float*);
        virtual void Delete(COctreeObj*, bool);
        virtual void* Compile(int*); // Not implemented in release hbm
        virtual void RemapObjects(RemapObjectIdFn pFnRemap) = 0;
        virtual bool CheckLinesegment(SRecurseInfoCompiled* pInfo, float* pvA, float* pvB) = 0;
        virtual void CheckCube(SRecurseInfoCompiled* pInfo, float* pvMin, float* pvMax) = 0;
        virtual void GetEverything(SRecurseInfoCompiled* pInfo) = 0;
        virtual void CheckPoint(SRecurseInfoCompiled* pInfo, const float* pvPos) = 0;

        // methods
        ZOctree();
        void SetScale(float fScale);
        void SetOrigin(const ZVector3& vOrigin);
        float GetScale() const;
        void GetOrigin(ZVector3& vOrigin) const;
        void ConvToOCS_NoAssert(int* pOCS, const ZVector3& vWorld) const;
        void ConvToOCS_NoAssert(int* pOCS, const float* pvWorld) const;

        // members
        ZVector3 m_vOrigin{0.f};
        float m_fScale{1.f};
    };
    RE_VERIFY_SIZE(ZOctree, 0x14);
}
