#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/Render/ZBoneModifyBase.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>


namespace Glacier
{
    const ZBone* ZLNKOBJ::GetBones() const
    {
        return m_pBoneModify->GetBones(this);
    }

    const ZBone* ZLNKOBJ::GetGlobalPrimBones() const
    {
        // it's ok due ZBone is POD type contains only 'float' entries.
        return reinterpret_cast<const ZBone*>(ZPrimControlBase::Instance()->GetGlobalPrimBones(Prim()));
    }
}
