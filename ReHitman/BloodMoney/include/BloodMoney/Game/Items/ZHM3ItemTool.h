#pragma once

#include <Glacier/GlacierFWD.h>

#include <BloodMoney/Game/Items/EHM3ItemCategory.h>
#include <BloodMoney/Game/Items/EHM3ItemType.h>

namespace Glacier
{
    class ZItem;
}

namespace Hitman
{
    class ZHM3ItemTemplate;
    class ZHM3ItemTemplateWeapon;

    // Static helper around the HM3 item family. The class holds no instance data; every entry
    // point maps to a free-standing PC routine (see the per-function addresses below and the
    // implementations).
    class ZHM3ItemTool
    {
    public:
        // PC 0x64B470. Returns the item's template when pItem is a ZHM3Item whose template is a
        // ZHM3ItemTemplate, otherwise nullptr.
        static ZHM3ItemTemplate* GetHM3ItemTemplate(Glacier::ZItem* pItem);

        // PC 0x64B4D0 (tail-calls the shared body at PC 0x518D00). Returns pItem's
        // ZHM3ItemTemplateWeapon when pItem is a ZHM3ItemWeapon, otherwise nullptr.
        static ZHM3ItemTemplateWeapon* GetHM3ItemTemplateWeapon(Glacier::ZItem* pItem);

        // PC 0x64D190. Resolves the HM3 item type of an item instance (template, weapon or
        // container).
        static EHM3ItemType GetHM3Type(Glacier::ZItem* pItem);

        // PC 0x649400. Looks a script/editor item name up in the HM3 name table.
        static EHM3ItemType GetHM3Type(const char* pszName);

        // PC 0x6493D0. Returns the script/editor name of an HM3 item type.
        static const char* GetHM3ItemName(EHM3ItemType eItemType);

        // PC 0x649440.
        static int GetHM3OSDSpriteID(EHM3ItemType eItemType);

        // PC 0x6495C0.
        static bool SpriteSizeLarge(EHM3ItemType eItemType);

        // PC 0x649630.
        static EHM3ItemCategory GetItemCategory(EHM3ItemType eItemType);

        // PC 0x649A30.
        static int MaxNumOfItem(EHM3ItemCategory eCategory);
    };
}
