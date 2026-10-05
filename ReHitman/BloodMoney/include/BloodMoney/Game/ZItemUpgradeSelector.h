#pragma once

#include <Glacier/GUI/XMLInterface/Elements/IGUIElement.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <Glacier/GUI/XMLInterface/ZResourceManager.h>
#include <BloodMoney/Game/ZItemUpgrades.h>
#include <cstdint>

namespace Glacier
{
    class ZResourceManager;
    class ZWINGROUP;
    class ZBUTTON;
    class ZColorSet;
}

namespace Hitman
{
    struct sSuitcaseItem
    {
        int32_t m_eItemType;
        int32_t m_lNumAmmo;
        uint64_t m_lUpgradeMask;
        uint64_t m_lUpgradesAvailable;
        bool m_bInSuitcase;
        uint8_t m_Padding[7];
    };
    static_assert(sizeof(sSuitcaseItem) == 0x20, "sSuitcaseItem layout");

    struct SWeaponUpgrade
    {
        Glacier::ZWINGROUP* m_apGraphic[5];
        Glacier::ZWINGROUP* m_pParent;
        Glacier::ZWINGROUP* m_pText;

        SWeaponUpgrade();
        void ReleaseResources(Glacier::ZResourceManager*);
        void Setup(float*, const Glacier::zstring&, const Glacier::zstring&, Glacier::ZResourceManager*, Glacier::ZWINGROUP*, const Glacier::zstring&, bool);
        void SetPos(float*);
        void SetNumber(int);
        void HideText(bool);
        Glacier::ZWINGROUP* GetTierGraphic();
    };
    static_assert(sizeof(SWeaponUpgrade) == 0x1C, "SWeaponUpgrade layout");

    enum EMovement : int32_t
    {
        ENONE = 0,
        ELEFT = 1,
        ERIGHT = 2,
    };



    class ZUpgradeSelector : public Glacier::IGUIElement
    {
    public:
        static constexpr int AVAILABLE_BACKUP_ITEMS = 18;
        static constexpr int NUM_OF_UPGRADES_ON_SCREEN = 6;

        IUpgradeInterface* m_pWeaponUpgrade; // +0x68
        sSuitcaseItem m_aBackupSuitCases[AVAILABLE_BACKUP_ITEMS]; // +0x70
        bool m_abGotBackup[AVAILABLE_BACKUP_ITEMS]; // +0x2B0
        uint8_t m_Padding2[2]; // +0x2C2, aligns the following uint32
        uint32_t m_iHitmanMoneyBackup; // +0x2C4
        int32_t m_iFocus; // +0x2C8
        Glacier::ZResourceManager* m_pResourceManager; // +0x2CC
        Glacier::ZWINGROUP* m_pParent; // +0x2D0
        Glacier::ZWINGROUP* m_pBackground; // +0x2D4
        Glacier::ZWINGROUP* m_pCurrentFocus; // +0x2D8
        Glacier::ZBUTTON* m_pButton; // +0x2DC
        Glacier::ZBUTTON* m_pLeftButton; // +0x2E0
        Glacier::ZBUTTON* m_pRightButton; // +0x2E4
        Glacier::ZWINGROUP* m_pWhiteBorder; // +0x2E8
        Glacier::ZColorSet* m_pColorSetNonBuyable; // +0x2EC
        Glacier::ZColorSet* m_pColorSetTier; // +0x2F0
        EMovement m_eLastMove; // +0x2F4
        SWeaponUpgrade m_aWeaponUpgrades[NUM_OF_UPGRADES_ON_SCREEN]; // +0x2F8
        float m_v2CurrentPos[2]; // +0x3A0
        Glacier::ZEaseIn m_EaseIn; // +0x3A8
        bool m_bAnimating; // +0x3C8
        uint8_t m_Padding3[7];

        ZUpgradeSelector();
        virtual Glacier::ZGUIElementLink Setup(float*, Glacier::ZResourceManager*, Glacier::ZWINGROUP*);
        virtual void ReleaseResources(Glacier::ZResourceManager*);
        virtual void Click(Glacier::eZWUserEvents, int, Glacier::ZXMLGUISystem*);
        virtual bool SetFocus(bool);
        virtual void Update(bool);
        virtual void readParams(const char**, Glacier::ZMenuElements*);
        virtual void Invalidate();
        virtual void Cancel();

        int GetFocus();
        IUpgradeInterface* GetUpgradeInterface();

    private:
        void SetupGraphics(EMovement);
        void Backup();
        void MoveLeft();
        void MoveRight();
        void Select();
        int GetUpgradeNumber(int) const;
    };
    static_assert(sizeof(ZUpgradeSelector) == 0x3D0, "ZUpgradeSelector layout");

    class ZItemUpgradeSelector : public ZUpgradeSelector
    {
    public:
        ZItemUpgradeSelector();
        ZItemUpgrades m_ItemUpgrade;
    };
    static_assert(sizeof(ZItemUpgradeSelector) == 0x538, "ZItemUpgradeSelector layout");
}
