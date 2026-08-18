#include <Glacier/Fysix/ZCollisionBase.h>


namespace Glacier
{
    ZCollisionBase* ZCollisionBase::GetCollisionInterface()
    {
        return ZCollisionBase::s_pCollisionBase;
    }

    STATIC_CLASS_VAR_IMPL(ZCollisionBase, ZCollisionBase*, s_pCollisionBase, 0x0090DE98, nullptr);
}