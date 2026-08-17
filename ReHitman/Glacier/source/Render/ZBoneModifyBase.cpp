#include <Glacier/Render/ZBoneModifyBase.h>
#include <Glacier/Render/Draw/IDraw.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/IK/ZLNKOBJ.h>


namespace Glacier
{
    const ZBone* ZBoneModifyBase::GetBones(const ZLNKOBJ* pLnkObj) const
    {
        const ZBone* pBones = IDraw::Instance()->GetBaseGeomBones(pLnkObj->BaseGeom());
        if (!pBones)
        {
            // it's ok due ZBone is POD type contains only 'float' entries.
            return reinterpret_cast<const ZBone*>(ZPrimControlBase::Instance()->GetGlobalPrimBones(pLnkObj->Prim()));            
        }

        // TODO: Finish missing assert
        // Smth about Animation::Model entry
        return nullptr;
    }
}
