#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/Render/ZBoneModifyBase.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/ZPrimControlBase.h>


namespace Glacier
{
    const ZBone* ZLNKOBJ::GetBones() const
    {
        return m_pBoneModify->GetBones(this);
    }

    const ZBone* ZLNKOBJ::GetGlobalPrimBones() const
    {
        // TODO: Finish this place after ZPrimControlBase will be reversed
        // return g_pRenderDll->m_pPrimControl->GetGlobalPrimBones(Prim());
        return nullptr;
    }
}
