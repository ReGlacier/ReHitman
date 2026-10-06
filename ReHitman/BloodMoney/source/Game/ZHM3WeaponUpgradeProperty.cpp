#include <BloodMoney/Game/ZHM3WeaponUpgradeProperty.h>

#include <BloodMoney/Game/Items/ZHM3ItemWeaponCustom.h>
#include <BloodMoney/Game/Items/ZHM3ItemWeaponCustomTemplate.h>
#include <Glacier/Items/ZItemTemplateAmmo.h>
#include <Glacier/Items/ZItemTemplateWeapon.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/ZUniAssert.h>

#include <cmath>


namespace Hitman
{
    // vtbl slot 0 (deleting destructor)
    ZHM3WeaponUpgradeProperty::~ZHM3WeaponUpgradeProperty() = default;

    // vtbl slot 12 (RTTI)  PC 0x653010
    const Glacier::RTP::ZPropertyInfo& ZHM3WeaponUpgradeProperty::GetProperties() const
    {
        return ZHM3WeaponUpgradeProperty::Info;
    }

    // PC 0x804018 (PS2). The event must run late so it sees the fully applied weapon state.
    Glacier::ZEventBase::EEventPriority ZHM3WeaponUpgradeProperty::GetEventPriority()
    {
        return Glacier::ZEventBase::PRIORITY_SuperHigh;
    }

    // PC 0x652BB0. The PC body copies the three own fields.
    void ZHM3WeaponUpgradeProperty::CopyData(const Glacier::ZEventBase* Source)
    {
        const ZHM3WeaponUpgradeProperty* pSource = static_cast<const ZHM3WeaponUpgradeProperty*>(Source);
        m_eProperty = pSource->m_eProperty;
        m_ePropertyFunc = pSource->m_ePropertyFunc;
        m_fValue = pSource->m_fValue;
    }

    // PC 0x6533B0. Read the current value, combine it with this property's delta and write it back.
    int ZHM3WeaponUpgradeProperty::Command(Glacier::ZMSGID command, Glacier::ZDATA data)
    {
        ZASSERT(static_cast<bool>(m_msgApplyUpgrade));

        if (command == static_cast<Glacier::ZMSGID>(m_msgApplyUpgrade))
        {
            ZHM3ItemWeaponCustom* pWeapon = static_cast<ZHM3ItemWeaponCustom*>(data);
            SetValue(pWeapon, ProcessValue(pWeapon, GetValue(pWeapon)));
        }

        return 0;
    }

    // PC 0x652BD0. Reads the stat selected by m_eProperty from the custom weapon / its template.
    float ZHM3WeaponUpgradeProperty::GetValue(ZHM3ItemWeaponCustom* pWeapon)
    {
        ZHM3ItemWeaponCustomTemplate* pTemplate = pWeapon->GetHM3WeaponTemplate();
        ZASSERT(pTemplate != nullptr);

        switch (m_eProperty)
        {
        case ePTNearDamage:
            return pWeapon->GetNearDamage();
        case ePTFarDamage:
            return pWeapon->GetFarDamage();
        case ePTDamageMultiplier:
            return pTemplate->m_fDamageMultiplier;
        case ePTNearRange:
            return pTemplate->GetNearRange();
        case ePTFarRange:
            return pTemplate->GetFarRange();
        case ePTImpact:
            return pTemplate->GetImpact();
        case ePTRecoilX:
            return pTemplate->GetRecoilStrengthX();
        case ePTRecoilY:
            return pTemplate->GetRecoilStrengthY();
        case ePTRecoilXY:
            return (pTemplate->GetRecoilStrengthX() + pTemplate->GetRecoilStrengthY()) * 0.5f;
        case ePTPrecisionDeg:
            return pWeapon->GetPrecisionDegrees();
        case ePTMuzzleVelocity:
            // The template stores the velocity scaled by 100.
            return pTemplate->GetMuzzleVelocity() * 0.01f;
        case ePTProjectilesPerMagazine:
            return static_cast<float>(pWeapon->GetProjectilesPerMagazine());
        case ePTShotsPerMinute:
            return 60.0f / pTemplate->GetTimeBetweenShots();
        default:
            ZASSERT(false);
            return 0.f;
        }
    }

    // PC 0x652D00. Combines the current value with this property's delta according to m_ePropertyFunc.
    float ZHM3WeaponUpgradeProperty::ProcessValue(ZHM3ItemWeaponCustom* /*pWeapon*/, float fValue)
    {
        switch (m_ePropertyFunc)
        {
        case ePFSetValue:
            return m_fValue;
        case ePFAddValue:
            return fValue + m_fValue;
        case ePFSubValue:
            return fValue - m_fValue;
        case ePFMultValue:
            return fValue * m_fValue;
        case ePFDivValue:
            return fValue == 0.f ? 0.f : (m_fValue / fValue);
        case ePFMaxValue:
            // The PC branch returns the smaller of the two values.
            return fValue < m_fValue ? fValue : m_fValue;
        case ePFMinValue:
            // The PC branch returns the larger of the two values.
            return fValue <= m_fValue ? m_fValue : fValue;
        case ePFIncAbsValue:
            if (fValue < 0.f)
                return -(std::fabs(m_fValue) + std::fabs(fValue));
            return std::fabs(m_fValue) + fValue;
        case ePFDecAbsValue:
            if (fValue < 0.f)
            {
                const float fResult = -(std::fabs(fValue) - std::fabs(m_fValue));
                return fResult > 0.f ? 0.f : fResult;
            }
            else
            {
                const float fResult = fValue - std::fabs(m_fValue);
                return fResult <= 0.f ? 0.f : fResult;
            }
        default:
            ZASSERT(false);
            return fValue;
        }
    }

    // PC 0x652E40. Writes the combined value back through the matching setter.
    void ZHM3WeaponUpgradeProperty::SetValue(ZHM3ItemWeaponCustom* pWeapon, float fValue)
    {
        ZHM3ItemWeaponCustomTemplate* pTemplate = pWeapon->GetHM3WeaponTemplate();
        ZASSERT(pTemplate != nullptr);

        switch (m_eProperty)
        {
        case ePTNearDamage:
            {
                Glacier::ZItemTemplateAmmo* pAmmo = pWeapon->GetAmmoTemplate();
                ZASSERT(pAmmo != nullptr);
                pAmmo->m_fNearDamage = fValue;
            }
            break;
        case ePTFarDamage:
            {
                Glacier::ZItemTemplateAmmo* pAmmo = pWeapon->GetAmmoTemplate();
                ZASSERT(pAmmo != nullptr);
                pAmmo->m_fFarDamage = fValue;
            }
            break;
        case ePTDamageMultiplier:
            pTemplate->m_fDamageMultiplier = fValue;
            break;
        case ePTNearRange:
            pTemplate->m_fNearRange = fValue * 100.f;
            break;
        case ePTFarRange:
            pTemplate->m_fFarRange = fValue * 100.f;
            break;
        case ePTImpact:
            pTemplate->m_fImpact = fValue;
            break;
        case ePTRecoilX:
            pTemplate->m_fRecoilStrengthX = fValue;
            break;
        case ePTRecoilY:
            pTemplate->m_fRecoilStrengthY = fValue;
            break;
        case ePTRecoilXY:
            pTemplate->m_fRecoilStrengthX = fValue;
            pTemplate->m_fRecoilStrengthY = fValue;
            break;
        case ePTPrecisionDeg:
            pTemplate->m_fPrecisionDeg = fValue;
            break;
        case ePTMuzzleVelocity:
            // The template stores the velocity scaled by 100.
            pTemplate->SetMuzzleVelocity(fValue * 100.f);
            break;
        case ePTProjectilesPerMagazine:
            {
                Glacier::ZItemTemplateAmmo* pAmmo = pWeapon->GetAmmoTemplate();
                ZASSERT(pAmmo != nullptr);
                pAmmo->m_lProjectilesPerMagazine = static_cast<int>(fValue);
                // Re-read the (now updated) magazine capacity into the live weapon.
                pWeapon->SetProjectilesInMagazine(pWeapon->GetProjectilesPerMagazine());
            }
            break;
        case ePTShotsPerMinute:
            pTemplate->m_fTimeBetweenShots = 60.f / fValue;
            break;
        default:
            ZASSERT(false);
            break;
        }
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        // "EPropertyFunc" (PC 0x809040).
        static Glacier::ZEnumEntry PropertyFuncEntries[] = {
            {nullptr, ePFSetValue, "ePFSetValue"},
            {&PropertyFuncEntries[0], ePFAddValue, "ePFAddValue"},
            {&PropertyFuncEntries[1], ePFSubValue, "ePFSubValue"},
            {&PropertyFuncEntries[2], ePFMultValue, "ePFMultValue"},
            {&PropertyFuncEntries[3], ePFDivValue, "ePFDivValue"},
            {&PropertyFuncEntries[4], ePFMaxValue, "ePFMaxValue"},
            {&PropertyFuncEntries[5], ePFMinValue, "ePFMinValue"},
            {&PropertyFuncEntries[6], ePFIncAbsValue, "ePFIncAbsValue"},
            {&PropertyFuncEntries[7], ePFDecAbsValue, "ePFDecAbsValue"}};
        static Glacier::ZEnumInfo PropertyFuncInfo{&PropertyFuncEntries[8], "EPropertyFunc", sizeof(EPropertyFunc)};

        // "EPropertyType" (PC 0x809100).
        static Glacier::ZEnumEntry PropertyTypeEntries[] = {
            {nullptr, ePTNearDamage, "ePTNearDamage"},
            {&PropertyTypeEntries[0], ePTFarDamage, "ePTFarDamage"},
            {&PropertyTypeEntries[1], ePTDamageMultiplier, "ePTDamageMultiplier"},
            {&PropertyTypeEntries[2], ePTNearRange, "ePTNearRange"},
            {&PropertyTypeEntries[3], ePTFarRange, "ePTFarRange"},
            {&PropertyTypeEntries[4], ePTImpact, "ePTImpact"},
            {&PropertyTypeEntries[5], ePTRecoilX, "ePTRecoilX"},
            {&PropertyTypeEntries[6], ePTRecoilY, "ePTRecoilY"},
            {&PropertyTypeEntries[7], ePTRecoilXY, "ePTRecoilXY"},
            {&PropertyTypeEntries[8], ePTPrecisionDeg, "ePTPrecisionDeg"},
            {&PropertyTypeEntries[9], ePTMuzzleVelocity, "ePTMuzzleVelocity"},
            {&PropertyTypeEntries[10], ePTProjectilesPerMagazine, "ePTProjectilesPerMagazine"},
            {&PropertyTypeEntries[11], ePTShotsPerMinute, "ePTShotsPerMinute"}};
        static Glacier::ZEnumInfo PropertyTypeInfo{&PropertyTypeEntries[12], "EPropertyType", sizeof(EPropertyType)};

        // The PC chain is laid out tail-first in memory; the head ("m_eProperty", PC 0x80910C) is the
        // FirstProperty passed to DEFINE_ROUT_CLASS. Game offset -> class offset = +4.
        static Glacier::RTP::ZDataProperty<float> Value{
            .m_Node = {.m_Next = nullptr, .m_Name = "m_fValue", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_float,
            .m_Offset = CLASS_PROPERTY(ZHM3WeaponUpgradeProperty, m_fValue)};

        static Glacier::RTP::ZEnumProperty PropertyFunc{
            .m_Node = {.m_Next = Value, .m_Name = "m_ePropertyFunc", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Enum,
            .m_Offset = CLASS_PROPERTY(ZHM3WeaponUpgradeProperty, m_ePropertyFunc),
            .m_Info = &PropertyFuncInfo};

        static Glacier::RTP::ZEnumProperty Property{
            .m_Node = {.m_Next = PropertyFunc, .m_Name = "m_eProperty", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Enum,
            .m_Offset = CLASS_PROPERTY(ZHM3WeaponUpgradeProperty, m_eProperty),
            .m_Info = &PropertyTypeInfo};
    }

    DEFINE_ROUT_CLASS(
        ZHM3WeaponUpgradeProperty, // Class
        ZBaseConRout,              // BaseClass
        ZHM3WeaponUpgradeProperty, // FactoryName
        0,                         // RoutCases
        0,                         // Prio
        0x00809124,                // PropertiesAddr (ZHM3WeaponUpgradeProperty::Info)
        cProperties::Property,     // FirstProperty
        Glacier::ZBaseConRout      // SuperClass
    );

    STATIC_CLASS_VAR_IMPL(ZHM3WeaponUpgradeProperty, Glacier::ZMessageResolver, m_msgApplyUpgrade, 0x009B1E24, {"ApplyWeaponUpgrade"});
#   pragma endregion
}
