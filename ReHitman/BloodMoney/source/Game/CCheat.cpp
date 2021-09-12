#include <BloodMoney/Game/CCheat.h>
#include <BloodMoney/Game/ZHM3WeaponUpgradeControl.h>
#include <BloodMoney/Game/Items/EHM3ItemType.h>
#include <BloodMoney/Game/Items/ZHM3ItemTool.h>
#include <BloodMoney/Game/Items/ZHM3ItemWeaponCustom.h>
#include <BloodMoney/Game/ZHM3GameData.h>
#include <BloodMoney/Game/Globals.h>

#include <Glacier/ZSysInterfaceWintel.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/EventBase/ZEventBase.h>
#include <Glacier/CInventory.h>
#include <Glacier/Geom/ZEntityLocator.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZGEOM.h>

#include <spdlog/spdlog.h>

#include <array>

namespace Hitman::BloodMoney {
    static constexpr std::array<std::string_view, 20> kGiveSomeArray = {
            "SMG_MP7_01",
            "Gun_HKusp_01",
            "Equip_Bomb_01",
            "Equip_Bomb_01",
            "Equip_Bomb_01",
            "Equip_BombRemote_01",
            "Ammo_SMG_01",
            "Ammo_SMG_01",
            "Ammo_Custom_Sniper_01",
            "Ammo_Custom_Sniper_01",
            "Ammo_Custom_Shotgun_01",
            "Ammo_Custom_Shotgun_01",
            "Ammo_Custom_Shotgun_01",
            "Ammo_Custom_Shotgun_01",
            "Ammo_Pistol_01",
            "Ammo_Pistol_01",
            "Ammo_45ACP_01",
            "Custom_Pistol",
            "Custom_ShotGun",
            "Custom_SniperRifle"
    };

    static constexpr std::array<std::pair<EHM3ItemType, int>, 16> kGiveSomeList = {
            std::make_pair(EHM3ItemType::Item_LockPick, 1),
            std::make_pair(EHM3ItemType::Custom_Pistol, 1),
            std::make_pair(EHM3ItemType::Equip_Bomb_01, 4),
            std::make_pair(EHM3ItemType::Equip_BombRemote_01, 1),
            std::make_pair(EHM3ItemType::Gun_HKusp_Silenced_01, 1),
            std::make_pair(EHM3ItemType::CC_FiberWire_01, 1),
            std::make_pair(EHM3ItemType::CC_Knife_Bowie_01, 1),
            std::make_pair(EHM3ItemType::CC_Syringe_Poison_01, 4),
            std::make_pair(EHM3ItemType::Ammo_SMG_01, 10),
            std::make_pair(EHM3ItemType::Ammo_Pistol_01, 10),
            std::make_pair(EHM3ItemType::Ammo_Custom_ShotGun_01, 10),
            std::make_pair(EHM3ItemType::CC_Scythe_01, 1),
            std::make_pair(EHM3ItemType::CC_Scythe_01, 1),
            std::make_pair(EHM3ItemType::Equip_KevlarVest_01, 1),
            std::make_pair(EHM3ItemType::Item_Coin_01, 4)
    };

    void CCheat::GiveItem(Glacier::ZREF rItem) {
        auto pGameData = Glacier::getInterface<Hitman::BloodMoney::ZHM3GameData>(Globals::kGameDataAddr);
        if (!pGameData) {
            return;
        }

        auto pInventory = reinterpret_cast<Glacier::CInventory*>(reinterpret_cast<Glacier::ZGEOM*>(pGameData->m_Hitman3)->FindEvent(Glacier::CInventory::Name));
        if (!pInventory) {
            spdlog::error("Failed to locate Inventory ZEventBase instance in player (something goes wrong?), player ptr: {:08X}", reinterpret_cast<std::intptr_t>(pGameData->m_Hitman3));
        } else {
            auto pAddedItem = pInventory->AddItem(rItem);
            if (!pAddedItem) {
                return;
            }

            auto eWeaponType = ZHM3WeaponUpgradeControl::GetWeaponType(pAddedItem->m_baseGeom->entityName);
            if (eWeaponType != EWeaponType::EW_UNKNOWN) { //TODO: Add fix for WA-2000 (It's not classified as 'custom' weapon but it is)
                auto pCustomItem = reinterpret_cast<ZHM3ItemWeaponCustom*>(pAddedItem);

                pCustomItem->ApplyUpgrades(true);
            }
        }
    }

    void CCheat::GiveItem(Glacier::ZGROUP *pWeaponsGroup, std::string_view sItemName) {
        Glacier::ZEntityLocator* pCurrentEnt = pWeaponsGroup->m_baseGeom;
        Glacier::ZGEOM* pCurrentGeom = nullptr;

        do {
            pCurrentGeom = reinterpret_cast<Glacier::ZGEOM*>(pCurrentEnt->m_assignedTo);

            auto kZItemTemplate_Id = reinterpret_cast<std::intptr_t*>(0x0099BF30);
            auto kZItemTemplate_Mask = reinterpret_cast<std::intptr_t*>(0x0099BF34);
            const bool isItemTemplate = ((*kZItemTemplate_Mask) & pCurrentGeom->GetObjectId() )== *(kZItemTemplate_Id);
            const std::string_view sCurrentItemName { pCurrentEnt->entityName };

            if (sCurrentItemName == sItemName) {
                if (!isItemTemplate) {
                    spdlog::warn("Unable to give item '{}', it's not a ZItemTemplate thing", sItemName.data());
                }
                else {
                    CCheat::GiveItem(pCurrentGeom->GetRef());
                }

                return;
            }

            pWeaponsGroup->RecurGetNext(&pCurrentEnt);
        }
        while (pCurrentEnt);
    }

    void CCheat::GiveSome() {
        auto sysInterface = Glacier::getInterface<Glacier::ZSysInterfaceWintel>(Globals::kSysInterfaceAddr);
        if (!sysInterface) {
            return;
        }

        auto engineDB = sysInterface->m_engineDataBase;
        if (!engineDB) {
            return;
        }

        std::intptr_t rpWeaponsGroup = engineDB->GetSceneVar("WeaponsGroup");
        if (!rpWeaponsGroup) {
            spdlog::warn("No WeaponsGroup presented");
            return;
        }

        Glacier::ZREF rWeaponsGroup = *reinterpret_cast<Glacier::ZREF*>(rpWeaponsGroup);
        if (!rWeaponsGroup) {
            spdlog::warn("No WeaponsGroup value presented");
            return;
        }

        auto pWeaponsGroup = reinterpret_cast<Glacier::ZGROUP*>(Glacier::ZGEOM::RefToPtr(rWeaponsGroup));
        if (!pWeaponsGroup) {
            spdlog::warn("No WeaponsGroup ({}) instance presented", rWeaponsGroup);
            return;
        }

        for (const auto& [eItemType, amount]: kGiveSomeList) {
            for (int i = 0; i < amount; i++) {
                CCheat::GiveItem(pWeaponsGroup, ZHM3ItemTool::GetHM3ItemName(eItemType));
            }
        }

        for (const auto& sItemName: kGiveSomeArray) {
            CCheat::GiveItem(pWeaponsGroup, sItemName);
        }
    }
}