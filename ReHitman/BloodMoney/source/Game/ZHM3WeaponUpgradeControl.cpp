#include <BloodMoney/Game/ZHM3WeaponUpgradeControl.h>

#include <BloodMoney/Game/Items/ZHM3ItemWeaponCustom.h>
#include <BloodMoney/Game/Items/ZHM3ItemWeaponCustomTemplate.h>
#include <BloodMoney/Game/ZHM3WeaponUpgrade.h>
#include <BloodMoney/Game/ZHM3WeaponUpgradeWeapon.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/ZSTL/StringUtils.h>
#include <Glacier/ZUniAssert.h>


namespace Hitman
{
    namespace
    {
        // PC 0x653350. Walks the upgrade pool weapon's child geoms and returns the
        // ZHM3WeaponUpgrade whose m_eType matches lUpgradeType.
        ZHM3WeaponUpgrade* ResolveUpgrade(Glacier::ZGEOM* pUpgradeWeapon, uint32_t lUpgradeType)
        {
            // The pool object found by FindGeom("CustomRifle...") is a ZGROUP; the PC reads its
            // child list directly (ZGROUP::m_pGroupFirst).
            Glacier::ZGROUP* pGroup = static_cast<Glacier::ZGROUP*>(pUpgradeWeapon);

            for (Glacier::ZBaseGeom* pBaseGeom = pGroup->m_pGroupFirst; pBaseGeom != nullptr; pBaseGeom = pBaseGeom->Next())
            {
                Glacier::ZGEOM* pGeom = pBaseGeom->GetGeom();
                if (pGeom == nullptr)
                    continue;

                if ((pGeom->GetObjectId() & ZHM3WeaponUpgrade::m_Mask) != ZHM3WeaponUpgrade::m_Id)
                    continue;

                ZHM3WeaponUpgrade* pUpgrade = static_cast<ZHM3WeaponUpgrade*>(pGeom);
                if (static_cast<uint32_t>(pUpgrade->m_eType) == lUpgradeType)
                    return pUpgrade;
            }

            return nullptr;
        }

        // PC 0x64CAC0 -> 0x64A5D0. True when the weapon's custom template lists eUpgradeType.
        bool HasUpgrade(ZHM3ItemWeaponCustom* pWeapon, EUpgradeType eUpgradeType)
        {
            ZHM3ItemWeaponCustomTemplate* pTemplate = pWeapon->GetHM3WeaponTemplate();
            if (pTemplate == nullptr || pTemplate->m_pUpgrades == nullptr)
                return false;

            for (auto it = pTemplate->m_pUpgrades->begin(); it != pTemplate->m_pUpgrades->end(); ++it)
            {
                if (static_cast<EUpgradeType>(*it) == eUpgradeType)
                    return true;
            }

            return false;
        }
    }

    // PC 0x653820. The five upgrade weapons live contiguously and are indexed by EWeaponType.
    void ZHM3WeaponUpgradeControl::InitWeaponReferences()
    {
        Glacier::ZGEOM** apUpgradeWeapons = &m_pCustomRifleSniperUpgradeWeapon;

        apUpgradeWeapons[0] = FindGeom("CustomRifleSniper", nullptr);
        apUpgradeWeapons[1] = FindGeom("CustomGunHardballer", nullptr);
        apUpgradeWeapons[2] = FindGeom("CustomRifleAssault", nullptr);
        apUpgradeWeapons[3] = FindGeom("CustomSmg", nullptr);
        apUpgradeWeapons[4] = FindGeom("CustomRifleShotgun", nullptr);

        for (int i = 0; i < 5; ++i)
        {
            ZASSERT(apUpgradeWeapons[i] != nullptr);
            ZASSERT((apUpgradeWeapons[i]->GetObjectId() & ZHM3WeaponUpgradeWeapon::m_Mask) == ZHM3WeaponUpgradeWeapon::m_Id);
        }
    }

    // PC 0x653A00.
    void ZHM3WeaponUpgradeControl::ApplyDefaultUpgrades(EWeaponType weaponType, ZHM3ItemWeaponCustom *pCustomGun)
    {
        Glacier::ZGEOM* pWeapon = (&m_pCustomRifleSniperUpgradeWeapon)[static_cast<int>(weaponType)];

        ZASSERT(pWeapon != nullptr);
        ZASSERT((pWeapon->GetObjectId() & ZHM3WeaponUpgradeWeapon::m_Mask) == ZHM3WeaponUpgradeWeapon::m_Id);

        static_cast<ZHM3WeaponUpgradeWeapon*>(pWeapon)->ApplyDefaultBones(pCustomGun);
    }

    // PC 0x653AF0. Walks the weapon's upgrade REFTAB (EUpgradeType entries): every entry that
    // resolves to a ZHM3WeaponUpgrade in the weapon's upgrade pool is applied (including the
    // visible-bone fix-ups); entries that never resolve are erased from the list.
    void ZHM3WeaponUpgradeControl::ApplyUpgrades(EWeaponType weaponType, ZHM3ItemWeaponCustom *pCustomGun, Glacier::REFTAB *pUpgrades, bool bApply)
    {
        if (pUpgrades == nullptr)
            return;

        Glacier::ZGEOM* pUpgradeWeapon = (&m_pCustomRifleSniperUpgradeWeapon)[static_cast<int>(weaponType)];

        for (auto it = pUpgrades->begin(); it != pUpgrades->end(); )
        {
            const uint32_t lUpgradeType = *it;
            ZHM3WeaponUpgrade* pUpgrade = ResolveUpgrade(pUpgradeWeapon, lUpgradeType);

            if (pUpgrade != nullptr)
            {
                pUpgrade->ApplyUpgrade(pCustomGun, bApply);
                AddVisibleBonesFromUpgrade(pCustomGun, pUpgrade);
                HideVisibleBonesFromUpgrade(pCustomGun, pUpgrade);

                if (weaponType == EWeaponType::EW_SUB_MACHINE_GUN)
                    FixCustomSMGBones(weaponType, pCustomGun, static_cast<EUpgradeType>(lUpgradeType));

                ++it;
            }
            else
            {
                // Erase() leaves the iterator on the element that back-filled the hole.
                it.Erase();
            }
        }
    }

    // PC 0x653920. Every ZGEOM referenced by "m_BoneIncludes" contributes its base-geom name to
    // the weapon's visible-bone list.
    void ZHM3WeaponUpgradeControl::AddVisibleBonesFromUpgrade(ZHM3ItemWeaponCustom* pCustomGun, ZHM3WeaponUpgrade* pUpgrade)
    {
        if (pUpgrade->m_BoneIncludes == nullptr)
            return;

        Glacier::RefRun cRun;
        pUpgrade->m_BoneIncludes->RunInitNxtRef(&cRun);
        for (uint32_t* pEntry = pUpgrade->m_BoneIncludes->RunNxtRefPtr(&cRun); pEntry != nullptr;
             pEntry = pUpgrade->m_BoneIncludes->RunNxtRefPtr(&cRun))
        {
            Glacier::ZGEOM* pGeom = Glacier::ZGEOM::RefToPtr(*pEntry);
            if (pGeom == nullptr || pGeom->BaseGeom() == nullptr)
                continue;

            const char* psBoneName = pGeom->BaseGeom()->m_Name;
            if (psBoneName == nullptr)
                psBoneName = "<NONAME>";

            pCustomGun->AddVisibleBone(psBoneName);
        }
    }

    // PC 0x653990. Mirror of AddVisibleBonesFromUpgrade using "m_BoneExcludes".
    void ZHM3WeaponUpgradeControl::HideVisibleBonesFromUpgrade(ZHM3ItemWeaponCustom* pCustomGun, ZHM3WeaponUpgrade* pUpgrade)
    {
        if (pUpgrade->m_BoneExcludes == nullptr)
            return;

        Glacier::RefRun cRun;
        pUpgrade->m_BoneExcludes->RunInitNxtRef(&cRun);
        for (uint32_t* pEntry = pUpgrade->m_BoneExcludes->RunNxtRefPtr(&cRun); pEntry != nullptr;
             pEntry = pUpgrade->m_BoneExcludes->RunNxtRefPtr(&cRun))
        {
            Glacier::ZGEOM* pGeom = Glacier::ZGEOM::RefToPtr(*pEntry);
            if (pGeom == nullptr || pGeom->BaseGeom() == nullptr)
                continue;

            const char* psBoneName = pGeom->BaseGeom()->m_Name;
            if (psBoneName == nullptr)
                psBoneName = "<NONAME>";

            pCustomGun->RemoveVisibleBone(psBoneName);
        }
    }

    // PC 0x653660. The SMG reuses the same silencer/laser geoms for the short-barrel variants, so
    // when both a short barrel and a silencer/laser are present the long bones must be removed
    // (and vice versa).
    void ZHM3WeaponUpgradeControl::FixCustomSMGBones(EWeaponType /*weaponType*/, ZHM3ItemWeaponCustom* pCustomGun, EUpgradeType eUpgradeType)
    {
        switch (eUpgradeType)
        {
        case UT_ShortBarrel:
            if (HasUpgrade(pCustomGun, UT_Silencer1))
                pCustomGun->RemoveVisibleBone("W_Bone_Silencer_Type1");
            else if (HasUpgrade(pCustomGun, UT_Silencer2))
                pCustomGun->RemoveVisibleBone("W_Bone_Silencer_Type2");

            if (HasUpgrade(pCustomGun, UT_LaserSight))
                pCustomGun->RemoveVisibleBone("W_Bone_LaserSight");
            break;

        case UT_Silencer1:
            pCustomGun->RemoveVisibleBone(HasUpgrade(pCustomGun, UT_ShortBarrel)
                ? "W_Bone_Silencer_Type1"
                : "W_Bone_Silencer_Type01_Short");
            break;

        case UT_Silencer2:
            pCustomGun->RemoveVisibleBone(HasUpgrade(pCustomGun, UT_ShortBarrel)
                ? "W_Bone_Silencer_Type2"
                : "W_Bone_Silencer_Type02_Short");
            break;

        case UT_LaserSight:
            pCustomGun->RemoveVisibleBone(HasUpgrade(pCustomGun, UT_ShortBarrel)
                ? "W_Bone_LaserSight"
                : "W_Bone_LaserSight01_ShortBarrel");
            break;

        default:
            break;
        }
    }

    // PC 0x653A50. Appends the ZGEOM of every ZHM3WeaponUpgrade child of the upgrade pool weapon.
    void ZHM3WeaponUpgradeControl::GetUpgradeList(EWeaponType weaponType, Glacier::REFTAB* pOut)
    {
        Glacier::ZGEOM* pUpgradeWeapon = (&m_pCustomRifleSniperUpgradeWeapon)[static_cast<int>(weaponType)];
        Glacier::ZGROUP* pGroup = static_cast<Glacier::ZGROUP*>(pUpgradeWeapon);

        for (Glacier::ZBaseGeom* pBaseGeom = pGroup->m_pGroupFirst; pBaseGeom != nullptr; pBaseGeom = pBaseGeom->Next())
        {
            Glacier::ZGEOM* pGeom = pBaseGeom->GetGeom();
            if (pGeom == nullptr)
                continue;

            if ((pGeom->GetObjectId() & ZHM3WeaponUpgrade::m_Mask) != ZHM3WeaponUpgrade::m_Id)
                continue;

            pOut->Add(reinterpret_cast<uint32_t>(pGeom));
        }
    }

    EWeaponType ZHM3WeaponUpgradeControl::GetWeaponType(const char *psWeaponName)
    {
        // PC 0x6535E0.
        if (!Glacier::strcasecmp("Custom_AssaultRifle", psWeaponName))
            return EWeaponType::EW_ASSAULT_RIFLE;
        if (!Glacier::strcasecmp("Custom_Pistol", psWeaponName) || !Glacier::strcasecmp("Custom_Pistol_PICKUP", psWeaponName))
            return EWeaponType::EW_PISTOL;
        if (!Glacier::strcasecmp("Custom_ShotGun", psWeaponName))
            return EWeaponType::EW_SHOTGUN;
        return Glacier::strcasecmp("Custom_SubMachineGun", psWeaponName) != 0 ? EWeaponType::EW_UNKNOWN : EWeaponType::EW_SUB_MACHINE_GUN;
    }
}
