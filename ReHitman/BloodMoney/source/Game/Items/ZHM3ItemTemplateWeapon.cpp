#include <BloodMoney/Game/Items/ZHM3ItemTemplateWeapon.h>

#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/Items/ZItemTemplateAmmo.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/ZUniAssert.h>

#include <cstdio>

namespace Hitman
{
    ZHM3ItemTemplateWeapon::ZHM3ItemTemplateWeapon(const char* psName, Glacier::ZBaseGeom* pBaseGeom)
        : Glacier::ZItemTemplateWeapon(psName, pBaseGeom)
        , m_pClipParticleControl(nullptr)
        , m_pFireShellParticleControl(nullptr)
    {
    }

    // vtbl slot 12 (RTTI)  PC 0x64D8A0
    const Glacier::RTP::ZPropertyInfo& ZHM3ItemTemplateWeapon::GetProperties() const
    {
        return ZHM3ItemTemplateWeapon::Info;
    }

    // vtbl slot 13 (RTTI)  PC 0x64D930
    uint32_t ZHM3ItemTemplateWeapon::GetObjectId() const
    {
        return ZHM3ItemTemplateWeapon::m_Id;
    }

    // vtbl slot 14 (RTTI)  PC 0x64D940
    void ZHM3ItemTemplateWeapon::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZHM3ItemTemplateWeapon::m_Id;
        mask = ZHM3ItemTemplateWeapon::m_Mask;
    }

    // vtbl slot 15 (RTTI)  PC 0x64D8B0 -> &ZHM3ItemTemplateWeapon::m_OldClassInfo
    Glacier::ZGEOMCLASSINFO* ZHM3ItemTemplateWeapon::GetOldClassInfo() const
    {
        return ZHM3ItemTemplateWeapon::m_OldClassInfo;
    }

    // vtbl slot 192  PC 0x64D8D0
    EHM3ItemType ZHM3ItemTemplateWeapon::GetHM3ItemType()
    {
        return m_eHM3ItemType;
    }

    // vtbl slot 193  PC 0x64D8E0. The PC returns the EHM3RecoilRandom value stored at +0x16C.
    float ZHM3ItemTemplateWeapon::GetRecoilRandom()
    {
        return m_eHM3RecoilRandom;
    }

    // vtbl slot 194  PC 0x64D8F0
    float ZHM3ItemTemplateWeapon::GetRecoilStrengthX()
    {
        return m_fRecoilStrengthX;
    }

    // vtbl slot 195  PC 0x64D900
    float ZHM3ItemTemplateWeapon::GetRecoilStrengthY()
    {
        return m_fRecoilStrengthY;
    }

    // vtbl slot 196  PC 0x69D5C0 (shared "return 0" stub)
    void* ZHM3ItemTemplateWeapon::GetLaserIndicator()
    {
        return nullptr;
    }

    // vtbl slot 197  PC 0x69D5C0 (shared "return 0" stub)
    void* ZHM3ItemTemplateWeapon::GetRedDot()
    {
        return nullptr;
    }

    // vtbl slot 198  PC 0x649FE0. True for the custom sniper and for the whole sniper-rifle range.
    bool ZHM3ItemTemplateWeapon::CanPackIntoSuitcase()
    {
        return m_eHM3ItemType == EHM3ItemType::eHM3CustomSniper ||
            (m_eHM3ItemType >= EHM3ItemType::eHM3Sniper && m_eHM3ItemType <= EHM3ItemType::eHM3SniperRifle_SakoTRG_01);
    }

    // vtbl slot 199  PC 0x64D910
    EHM3WeaponScope ZHM3ItemTemplateWeapon::GetScope()
    {
        return m_eScope;
    }

    // vtbl slot 200  PC 0x64D920
    ESilencerType ZHM3ItemTemplateWeapon::GetSilencerType()
    {
        return m_eSilencerType;
    }

    // vtbl slot 201  PC 0x64A010. Damage-per-shot figure derived from the first ammo template,
    // clamped to 10.0; the constant 1.0 is returned when the weapon has no ammo template.
    float ZHM3ItemTemplateWeapon::GetWeaponStrength()
    {
        Glacier::ZItemTemplateAmmo* pAmmo = GetAmmoTemplate(0);
        if (pAmmo == nullptr)
            return 1.0f;

        const float fStrength = pAmmo->GetNearDamage() *
            static_cast<float>(pAmmo->GetProjectilesPerShot()) /
            static_cast<float>(GetTimeBetweenShots());

        return fStrength < 10.0f ? fStrength : 10.0f;
    }

    // vtbl slot 202  PC 0x64A760. The PC returns the ZAnimVariationHandle stored at +0x15C.
    int ZHM3ItemTemplateWeapon::Get1stPersonAimId()
    {
        return m_1stPersonAimId.iIndex;
    }

    // vtbl slot 203  PC 0x649390. The PC returns the ZAnimVariationHandle stored at +0x15E.
    int ZHM3ItemTemplateWeapon::Get1stPersonRecoilId()
    {
        return m_1stPersonRecoilId.iIndex;
    }

    // vtbl slot 204  PC 0x64B2F0
    const char* ZHM3ItemTemplateWeapon::GetAnimNameActorReload(int)
    {
        return m_pAnimNameActorReload.c_str();
    }

    // vtbl slot 205  PC 0x64D960
    const char* ZHM3ItemTemplateWeapon::GetAnimNameActorChamber()
    {
        return m_pAnimNameActorChamber.c_str();
    }

    // vtbl slot 206  PC 0x6493B0. True selects the clip particle, false the fired-shell particle.
    void* ZHM3ItemTemplateWeapon::GetClipParticleControl(bool bClip)
    {
        return bClip ? m_pClipParticleControl : m_pFireShellParticleControl;
    }

    // vtbl slot 4  PC 0x649A70. The PC calls the shared "return 1" PostLoad stub.
    bool ZHM3ItemTemplateWeapon::PostLoad(Glacier::ISerializerStream& stream)
    {
        return Glacier::ZSerializable::PostLoad(stream);
    }

    // vtbl slot 82  PC 0x649F20
    void ZHM3ItemTemplateWeapon::ClassInit()
    {
        Glacier::ZItemTemplateWeapon::ClassInit();

        m_fRecoilStrengthY = -m_fRecoilStrengthY;
    }

    // vtbl slot 83  PC 0x649F40
    void ZHM3ItemTemplateWeapon::ClassInit2()
    {
        Glacier::ZGEOM::ClassInit2();

        Glacier::ZAnimTemplatesNames names;
        if (!names.Init())
        {
            printf("Couldn't find anim templates names...\n");
            return;
        }

        if (*m_pAnimName1stPersonAim)
            names.FindAnimVariationHandle(m_1stPersonAimId, m_pAnimName1stPersonAim.c_str());

        if (*m_pAnimName1stPersonRecoil)
            names.FindAnimVariationHandle(m_1stPersonRecoilId, m_pAnimName1stPersonRecoil.c_str());
    }

    // vtbl slot 172  PC 0x64A000
    bool ZHM3ItemTemplateWeapon::HasSniperMode()
    {
        return m_eScope != EHM3WeaponScope::eNoScope;
    }

    // vtbl slot 149  PC 0x64D8C0
    uint32_t ZHM3ItemTemplateWeapon::GetItemClassId() const
    {
        return 0x10042Bu;
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        // "EHM3RecoilRandom" (PC property at game offset 0x168).
        static Glacier::ZEnumEntry RecoilRandomEntries[] = {
            {nullptr, static_cast<int>(EHM3RecoilRandom::eNoRandom), "eNoRandom"},
            {&RecoilRandomEntries[0], static_cast<int>(EHM3RecoilRandom::eXRandom), "eXRandom"},
            {&RecoilRandomEntries[1], static_cast<int>(EHM3RecoilRandom::eYRandom), "eYRandom"},
            {&RecoilRandomEntries[2], static_cast<int>(EHM3RecoilRandom::eXYRandom), "eXYRandom"}};
        static Glacier::ZEnumInfo RecoilRandomInfo{&RecoilRandomEntries[3], "EHM3RecoilRandom", sizeof(EHM3RecoilRandom)};

        // "EHM3WeaponScope" (PC property at game offset 0x174).
        static Glacier::ZEnumEntry WeaponScopeEntries[] = {
            {nullptr, static_cast<int>(EHM3WeaponScope::eNoScope), "eNoScope"},
            {&WeaponScopeEntries[0], static_cast<int>(EHM3WeaponScope::eNormal), "eNormal"},
            {&WeaponScopeEntries[1], static_cast<int>(EHM3WeaponScope::eHiTech), "eHiTech"}};
        static Glacier::ZEnumInfo WeaponScopeInfo{&WeaponScopeEntries[2], "EHM3WeaponScope", sizeof(EHM3WeaponScope)};

        // "ESilencerType" (PC property at game offset 0x198).
        static Glacier::ZEnumEntry SilencerTypeEntries[] = {
            {nullptr, static_cast<int>(ESilencerType::eNotSilent), "eNotSilent"},
            {&SilencerTypeEntries[0], static_cast<int>(ESilencerType::eSilentLowVelocityAmmo), "eSilentLowVelocityAmmo"},
            {&SilencerTypeEntries[1], static_cast<int>(ESilencerType::eSilentUpTo20), "eSilentUpTo20"},
            {&SilencerTypeEntries[2], static_cast<int>(ESilencerType::eSilentOver20), "eSilentOver20"},
            {&SilencerTypeEntries[3], static_cast<int>(ESilencerType::eSilentWithNoEvent), "eSilentWithNoEvent"}};
        static Glacier::ZEnumInfo SilencerTypeInfo{&SilencerTypeEntries[4], "ESilencerType", sizeof(ESilencerType)};

        // The PC chain is laid out tail-first; the head (m_eHM3RecoilRandom) is the FirstProperty
        // passed to DECLARE_GEOM_CLASS_IMPL. Game offset -> class offset = +4
        // (ZHM3ItemTemplateWeapon::Info.First is 0x80F938).
        static Glacier::RTP::ZEnumProperty SilencerType{
            .m_Node = {.m_Next = nullptr, .m_Name = "m_eSilencerType", .m_Filter = 3},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Enum,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemTemplateWeapon, m_eSilencerType),
            .m_Info = &SilencerTypeInfo};

        static Glacier::RTP::ZDataProperty<float> PelvisLeftRight{
            .m_Node = {.m_Next = SilencerType, .m_Name = "m_PelvisLeftRight", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_float,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemTemplateWeapon, m_PelvisLeftRight)};

        static Glacier::RTP::ZDataProperty<float> PelvisFrontBack{
            .m_Node = {.m_Next = PelvisLeftRight, .m_Name = "m_PelvisFrontBack", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_float,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemTemplateWeapon, m_PelvisFrontBack)};

        static Glacier::RTP::ZDataProperty<float> PelvisUpDown{
            .m_Node = {.m_Next = PelvisFrontBack, .m_Name = "m_PelvisUpDown", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_float,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemTemplateWeapon, m_PelvisUpDown)};

        static Glacier::RTP::ZDataProperty<float> RecognitionDistance{
            .m_Node = {.m_Next = PelvisUpDown, .m_Name = "RecognitionDistance", .m_Filter = 3},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_float,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemTemplateWeapon, RecognitionDistance)};

        static Glacier::RTP::ZDataProperty<Glacier::ZRTString> AnimName1stPersonRecoil{
            .m_Node = {.m_Next = RecognitionDistance, .m_Name = "m_pAnimName1stPersonRecoil", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_ZRTString,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemTemplateWeapon, m_pAnimName1stPersonRecoil)};

        static Glacier::RTP::ZDataProperty<Glacier::ZRTString> AnimName1stPersonAim{
            .m_Node = {.m_Next = AnimName1stPersonRecoil, .m_Name = "m_pAnimName1stPersonAim", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_ZRTString,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemTemplateWeapon, m_pAnimName1stPersonAim)};

        static Glacier::RTP::ZDataProperty<Glacier::ZRTString> AnimNameActorChamber{
            .m_Node = {.m_Next = AnimName1stPersonAim, .m_Name = "m_pAnimNameActorChamber", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_ZRTString,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemTemplateWeapon, m_pAnimNameActorChamber)};

        static Glacier::RTP::ZDataProperty<Glacier::ZRTString> AnimNameActorReload{
            .m_Node = {.m_Next = AnimNameActorChamber, .m_Name = "m_pAnimNameActorReload", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_ZRTString,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemTemplateWeapon, m_pAnimNameActorReload)};

        static Glacier::RTP::ZEnumProperty Scope{
            .m_Node = {.m_Next = AnimNameActorReload, .m_Name = "m_eScope", .m_Filter = 3},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Enum,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemTemplateWeapon, m_eScope),
            .m_Info = &WeaponScopeInfo};

        static Glacier::RTP::ZDataProperty<float> RecoilStrengthY{
            .m_Node = {.m_Next = Scope, .m_Name = "m_fRecoilStrengthY", .m_Filter = 3},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_float,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemTemplateWeapon, m_fRecoilStrengthY)};

        static Glacier::RTP::ZDataProperty<float> RecoilStrengthX{
            .m_Node = {.m_Next = RecoilStrengthY, .m_Name = "m_fRecoilStrengthX", .m_Filter = 3},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_float,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemTemplateWeapon, m_fRecoilStrengthX)};

        static Glacier::RTP::ZEnumProperty RecoilRandom{
            .m_Node = {.m_Next = RecoilStrengthX, .m_Name = "m_eHM3RecoilRandom", .m_Filter = 3},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Enum,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemTemplateWeapon, m_eHM3RecoilRandom),
            .m_Info = &RecoilRandomInfo};
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZHM3ItemTemplateWeapon,       // ClassName
        Glacier::ZItemTemplateWeapon, // BaseClass
        0x009B1608,                   // OldClassInfoAddr
        "ZHM3ItemTemplateWeapon",     // FactoryName
        0x0,                          // FactoryNameAddr (documentation only; not bound by the macro)
        cProperties::RecoilRandom,    // FirstProperty
        0x0080F950,                   // PropertiesAddr (ZHM3ItemTemplateWeapon::Info)
        0x009B14F8,                   // IdAddr (ZHM3ItemTemplateWeapon::m_Id)
        0x009B14FC                    // MaskAddr (ZHM3ItemTemplateWeapon::m_Mask)
    );
#   pragma endregion
}
