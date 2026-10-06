#include <BloodMoney/Game/ZHM3WeaponUpgradeWeapon.h>

#include <BloodMoney/Game/Items/ZHM3ItemWeaponCustom.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/ZUniAssert.h>


namespace Hitman
{
    // vtbl slot 0 (deleting destructor)  PC 0x653200
    ZHM3WeaponUpgradeWeapon::~ZHM3WeaponUpgradeWeapon() = default;

    // PC 0x653020
    ZHM3WeaponUpgradeWeapon::ZHM3WeaponUpgradeWeapon(const char* psName, Glacier::ZBaseGeom* pBaseGeom)
        : Glacier::ZGROUP(psName, pBaseGeom)
        , m_pDefaultBones(nullptr)
    {
    }

    // vtbl slot 12 (RTTI)  PC 0x652830
    const Glacier::RTP::ZPropertyInfo& ZHM3WeaponUpgradeWeapon::GetProperties() const
    {
        return ZHM3WeaponUpgradeWeapon::Info;
    }

    // vtbl slot 13 (RTTI)  PC 0x653170
    uint32_t ZHM3WeaponUpgradeWeapon::GetObjectId() const
    {
        return ZHM3WeaponUpgradeWeapon::m_Id;
    }

    // vtbl slot 14 (RTTI)  PC 0x653180
    void ZHM3WeaponUpgradeWeapon::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZHM3WeaponUpgradeWeapon::m_Id;
        mask = ZHM3WeaponUpgradeWeapon::m_Mask;
    }

    // vtbl slot 15 (RTTI)  PC 0x652840 -> &ZHM3WeaponUpgradeWeapon::m_OldClassInfo
    Glacier::ZGEOMCLASSINFO* ZHM3WeaponUpgradeWeapon::GetOldClassInfo() const
    {
        return ZHM3WeaponUpgradeWeapon::m_OldClassInfo;
    }

    // vtbl slot 104  PC 0x653220
    void ZHM3WeaponUpgradeWeapon::CopyData(const Glacier::ZGEOM* Source)
    {
        ZASSERT(Source != nullptr);
        ZASSERT((Source->GetObjectId() & ZHM3WeaponUpgradeWeapon::m_Mask) == ZHM3WeaponUpgradeWeapon::m_Id);
    }

    // PC 0x653240
    void ZHM3WeaponUpgradeWeapon::ApplyDefaultBones(ZHM3ItemWeaponCustom* pCustomGun)
    {
        if (m_pDefaultBones == nullptr)
            return;

        Glacier::RefRun cRun;
        m_pDefaultBones->RunInitNxtRef(&cRun);
        for (uint32_t* pEntry = m_pDefaultBones->RunNxtRefPtr(&cRun); pEntry != nullptr;
             pEntry = m_pDefaultBones->RunNxtRefPtr(&cRun))
        {
            Glacier::ZGEOM* pGeom = Glacier::ZGEOM::RefToPtr(*pEntry);
            if (pGeom == nullptr)
                continue;

            const char* psBoneName = pGeom->BaseGeom() != nullptr ? pGeom->BaseGeom()->m_Name : nullptr;
            if (psBoneName == nullptr)
                psBoneName = "<NONAME>";

            pCustomGun->AddVisibleBone(psBoneName);
        }
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        // PC 0x808C6C. The PS2 property record names this entry "m_pDefaultBones"; it is the
        // only property of the chain (Info.First == 0x808C6C on PC).
        static Glacier::RTP::ZDataProperty<Glacier::REFTAB*> DefaultBones{
            .m_Node = {.m_Next = nullptr, .m_Name = "m_pDefaultBones", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_REFTAB_ptr,
            .m_Offset = CLASS_PROPERTY(ZHM3WeaponUpgradeWeapon, m_pDefaultBones)};
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZHM3WeaponUpgradeWeapon,   // ClassName
        Glacier::ZGROUP,           // BaseClass
        0x009B1CC0,                // OldClassInfoAddr
        "ZHM3WeaponUpgradeWeapon", // FactoryName
        0x0,                       // FactoryNameAddr (documentation only; not bound by the macro)
        cProperties::DefaultBones, // FirstProperty
        0x00808C80,                // PropertiesAddr (ZHM3WeaponUpgradeWeapon::Info)
        0x009B1C60,                // IdAddr (ZHM3WeaponUpgradeWeapon::m_Id)
        0x009B1C64                 // MaskAddr (ZHM3WeaponUpgradeWeapon::m_Mask)
    );
#   pragma endregion
}
