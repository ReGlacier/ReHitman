#include <Glacier/Render/ZBoneModifyBase.h>
#include <Glacier/Render/Draw/IDraw.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/ZPrimControlBase.h>
#include <Glacier/IK/ZLNKOBJ.h>


namespace Glacier
{
    const ZBone* ZBoneModifyBase::GetBones(const ZLNKOBJ* pLnkObj) const
    {
        const ZBone* pBones = IDraw::Instance()->GetBaseGeomBones(pLnkObj->BaseGeom());
        if (pBones)
        {
            return pBones;
        }

        // TODO: Finish this place after ZPrimControlBase will be reversed
        // return g_pRenderDll->m_pPrimControl->GetGlobalPrimBones(pLnkObj->Prim());
        return nullptr;
    }
}
