#include <BloodMoney/Game/ZItemUpgradeSelector.h>
#include <BloodMoney/Game/ZHM3GameData.h>
#include <cstring>

#include <BloodMoney/Game/Globals.h>
#include <Glacier/System/ZSysInterfaceWintel.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/ResourceCollection.h>

namespace Hitman
{
    // PC 0x67F940.
    Glacier::zstring IUpgradeInterface::GetButtonText(int index)
    {
        static const char* const buttonNames[] = {
            "UpgradeBuy", "ImprovedEquipped", "UpgradeUnequip", "UpgradeEquip", "NotAvailable"
        };
        auto* resources = ::g_pSysInterface->m_pEngineData->m_pLocaleResources;
        return Glacier::zstring(resources->GetResourceText("AllLevels/Interface/Buttons/", buttonNames[GetState(index)]));
    }

    void IUpgradeInterface::SetDetailedUpgrade(int) {}

    namespace
    {
        constexpr int kStateDisabled = 0;
        constexpr int kStateEnabled = 2;
        constexpr int kStateUnattainable = 4;

        const char* CategoryName(int category)
        {
            switch (category)
            {
            case 0: return "Key";
            case 1: return "Medicine";
            default: return "Equipment";
            }
        }

        const char* StateSuffix(int state)
        {
            switch (state)
            {
            case kStateEnabled: return "Enabled*";
            case kStateUnattainable: return "Unattainable*";
            default: return "Disabled*";
            }
        }
    }

    ZItemUpgrades::ZItemUpgrades()
    {
        for (auto& upgrade : m_aItemUpgrades)
        {
            upgrade = {};
            upgrade.m_eItemType = -1;
        }

        m_aItemUpgrades[0] = {265, 0, 0, 0, "Painkillers", nullptr, 1};
        m_aItemUpgrades[1] = {266, 1, 0, 0, "ImprovedBinocularOptics", nullptr, 2};
        m_aItemUpgrades[2] = {3, 2, 256, 0, "ImprovedLockPick", nullptr, 0};
        m_aItemUpgrades[3] = {174, 3, 0, 3, "KevlarVest", nullptr, 2};
        m_aItemUpgrades[4] = {9, 4, 0, 2, "EnhancedBombRemote", nullptr, 2};
        m_aItemUpgrades[5] = {267, 5, 0, 2, "Adrenaline", nullptr, 1};
        m_aItemUpgrades[6] = {174, 7, 1024, 2, "FlakVest", nullptr, 2};
        m_aItemUpgrades[7] = {8, 6, 0, 2, "ExtraBomb", nullptr, 2};
        m_aItemUpgrades[8] = {8, 8, 0, 3, "CrattSchultzLockPick", nullptr, 0};
        m_aItemUpgrades[9] = {61, 9, 0, 3, "FoilPaddedSafeSuitcase", nullptr, 2};
        m_aItemUpgrades[10] = {174, 10, 0, 4, "FlexibleFlakVest", nullptr, 2};
    }

    int ZItemUpgrades::NormalizeIndex(int index) const
    {
        if (index >= 11)
            index -= 11;
        if (index < 0)
            index += 11;
        return index;
    }

    const SItemUpgrade* ZItemUpgrades::GetUpgrade(int index) const
    {
        return &m_aItemUpgrades[NormalizeIndex(index)];
    }

    int ZItemUpgrades::GetSize()
    {
        return 11;
    }

    int ZItemUpgrades::GetTier(int index)
    {
        return static_cast<int>(GetUpgrade(index)->m_iTier);
    }

    Glacier::zstring ZItemUpgrades::GetIconName(int index)
    {
        const auto* upgrade = GetUpgrade(index);
        Glacier::zstring result(CategoryName(upgrade->m_eCategory));
        const char* suffix = StateSuffix(GetState(index));
        result.append(suffix, static_cast<uint32_t>(std::strlen(suffix)));
        return result;
    }

    Glacier::zstring ZItemUpgrades::GetSmallIcon(int index)
    {
        return Glacier::zstring(GetUpgrade(index)->m_eCategory == 0
            ? "KeySmall"
            : GetUpgrade(index)->m_eCategory == 1 ? "MedicineSmall" : "EquipmentSmall");
    }

    Glacier::zstring ZItemUpgrades::GetName(int index)
    {
        return Glacier::zstring(GetUpgrade(index)->m_pszName);
    }

    Glacier::zstring ZItemUpgrades::Get3dGroup()
    {
        return Glacier::zstring("Item_Upgrades");
    }

    Glacier::zstring ZItemUpgrades::GetLocalePath()
    {
        return Glacier::zstring("AllLevels/Upgrades/ItemUpgrades/");
    }

    Glacier::zstring ZItemUpgrades::GetButtonText(int index)
    {
        Glacier::zstring result = IUpgradeInterface::GetButtonText(index);
        if (GetState(index) == 2)
            result = Glacier::zstring("");
        return result;
    }

    void ZItemUpgrades::GetPrice(Glacier::zstring& result, int index)
    {
        auto* gameData = static_cast<ZHM3GameData*>(Glacier::g_pGameData);
        result.format("%i", static_cast<int>(gameData->m_pMoneySystem->UpgradePrice(GetTier(index))));
    }

    int ZItemUpgrades::GetState(int index)
    {
        auto* gameData = static_cast<ZHM3GameData*>(Glacier::g_pGameData);
        if (GetTier(index) > gameData->m_pMoneySystem->GetCurrentTier()
            && !Glacier::ZSysInterface::GetOption("UnlockAllUpgrades", nullptr))
            return 4;
        const auto* upgrade = GetUpgrade(index);
        sSuitcaseItem item;
        const int available = gameData->m_LevelLinking.GetAvailableItem(upgrade->m_eItemType, item);
        ZASSERT(available != -1);
        if (available == -1)
            return 4;
        if ((item.m_lUpgradesAvailable & upgrade->m_iExcludedBy) != 0)
            return 1;
        return (item.m_lUpgradesAvailable & (uint64_t(1) << upgrade->m_iBitShift)) != 0 ? 2 : 0;
    }

    bool ZItemUpgrades::IsAffordable(int index)
    {
        auto* gameData = static_cast<ZHM3GameData*>(Glacier::g_pGameData);
        return gameData->m_LevelLinking.GetHitmanMoney() >= gameData->m_pMoneySystem->UpgradePrice(GetTier(index));
    }

    void ZItemUpgrades::Setup()
    {
        // PC Setup is a no-op for this concrete item-upgrade provider.
    }

    void ZItemUpgrades::Select(int index)
    {
        const int state = GetState(index);
        if (state == 4)
            return;
        if (state == 1)
        {
            // TODO: Finish me after ZGui::m_pXMLGUISystem reversed.
            // gameData->m_Gui->m_pXMLGUISystem->OpenWindow("ImprovedEquippedDialog", true, 0);
            return;
        }
        auto* gameData = static_cast<ZHM3GameData*>(Glacier::g_pGameData);
        const auto* upgrade = GetUpgrade(index);
        sSuitcaseItem item;
        const int available = gameData->m_LevelLinking.GetAvailableItem(upgrade->m_eItemType, item);
        ZASSERT(available != -1);
        if (available == -1)
            return;
        if (state == 0 && IsAffordable(index))
        {
            gameData->m_LevelLinking.IncMoney(-static_cast<int>(gameData->m_pMoneySystem->UpgradePrice(upgrade->m_iTier)));
            const uint64_t mask = uint64_t(1) << upgrade->m_iBitShift;
            item.m_lUpgradesAvailable |= mask;
            item.m_lUpgradeMask |= mask;
            gameData->m_LevelLinking.AddAvailableItem(item, true);
        }
    }

    bool ZItemUpgrades::HasDetails() const
    {
        return false;
    }

    void ZItemUpgrades::SetDetailedUpgrade(int)
    {
        // Base interface implementation is empty in the PC vtable.
    }
}
