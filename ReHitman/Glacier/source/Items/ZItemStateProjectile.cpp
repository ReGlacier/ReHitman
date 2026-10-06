#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/Items/ZItemStateProjectile.h>

#include <Glacier/Items/ZItem.h>
#include <Glacier/Items/ZItemTemplateAmmo.h>
#include <Glacier/Items/ZItemTemplateWeapon.h>
#include <Glacier/Items/ZItemWeapon.h>


namespace Glacier
{
    ZGEOM* ZItemStateProjectile::GetUseGeom(ZItem* item)
    {
        if (item == nullptr || !item->IsDerivedFrom<ZItemWeapon>())
            return nullptr;

        auto* weapon = static_cast<ZItemWeapon*>(item);
        ZItemTemplateAmmo* ammoTemplate = weapon->GetAmmoTemplate();
        if (ammoTemplate == nullptr)
            return nullptr;

        return ammoTemplate->GetProjectile();
    }
}
