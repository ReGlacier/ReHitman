#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/EventBase/ZBaseConRout.h>
#include <Glacier/RTP/Base.h>
#include <Glacier/Runtime/Macro.h>
#include <Glacier/ZMessageResolver.h>

namespace Hitman
{
    class ZHM3ItemWeaponCustom;

    /**
     * @brief Weapon stat a ZHM3WeaponUpgradeProperty operates on (PC enum `EPropertyType`).
     */
    enum EPropertyType : int {
        ePTNearDamage = 0,
        ePTFarDamage = 1,
        ePTDamageMultiplier = 2,
        ePTNearRange = 3,
        ePTFarRange = 4,
        ePTImpact = 5,
        ePTRecoilX = 6,
        ePTRecoilY = 7,
        ePTRecoilXY = 8,
        ePTPrecisionDeg = 9,
        ePTMuzzleVelocity = 10,
        ePTProjectilesPerMagazine = 11,
        ePTShotsPerMinute = 12,
    };

    /**
     * @brief How the stat delta is combined with the current value (PC enum `EPropertyFunc`).
     */
    enum EPropertyFunc : int {
        ePFSetValue = 0,
        ePFAddValue = 1,
        ePFSubValue = 2,
        ePFMultValue = 3,
        ePFDivValue = 4,
        ePFMaxValue = 5,
        ePFMinValue = 6,
        ePFIncAbsValue = 7,
        ePFDecAbsValue = 8,
    };

    /**
     * @brief PC RTTI 0x79D4FC (36 vtable slots), a ZBaseConRout event, size 0x3C.
     *
     * One of these events is attached to an upgrade geom. When the geom broadcasts the
     * "ApplyWeaponUpgrade" command the event reads the current weapon stat (GetValue), combines it
     * with its own delta via the selected EPropertyFunc (ProcessValue) and writes the result back
     * (SetValue).
     */
    class ZHM3WeaponUpgradeProperty : public Glacier::ZBaseConRout
    {
    public:
        // RTTI
        DECLARE_ROUT_CLASS(ZHM3WeaponUpgradeProperty, ZBaseConRout, ZHM3WeaponUpgradeProperty, 0, 0);

        // static
        STATIC_CLASS_VAR(ZHM3WeaponUpgradeProperty, Glacier::ZMessageResolver, m_msgApplyUpgrade);

        // vtbl
        ~ZHM3WeaponUpgradeProperty() override;
        const Glacier::RTP::ZPropertyInfo& GetProperties() const override;
        Glacier::ZEventBase::EEventPriority GetEventPriority() override;
        void CopyData(const Glacier::ZEventBase* Source) override;
        int Command(Glacier::ZMSGID command, Glacier::ZDATA data) override;

        // methods
        // PC 0x652BD0.
        float GetValue(ZHM3ItemWeaponCustom* pWeapon);
        // PC 0x652D00.
        float ProcessValue(ZHM3ItemWeaponCustom* pWeapon, float fValue);
        // PC 0x652E40.
        void SetValue(ZHM3ItemWeaponCustom* pWeapon, float fValue);

        // data (total size is 0x3C, base ZBaseConRout is 0x30)
        EPropertyType m_eProperty;     // +0x30  "m_eProperty"
        EPropertyFunc m_ePropertyFunc; // +0x34  "m_ePropertyFunc"
        float m_fValue;                // +0x38  "m_fValue"
    };
    RE_VERIFY_SIZE(ZHM3WeaponUpgradeProperty, 0x3C);
    RE_VERIFY_OFFSET(ZHM3WeaponUpgradeProperty, m_eProperty, 0x30);
    RE_VERIFY_OFFSET(ZHM3WeaponUpgradeProperty, m_ePropertyFunc, 0x34);
    RE_VERIFY_OFFSET(ZHM3WeaponUpgradeProperty, m_fValue, 0x38);
}
