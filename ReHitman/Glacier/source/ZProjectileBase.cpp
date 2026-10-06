#include <Glacier/ZProjectileBase.h>

#include <Glacier/Com/CCom.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Materials/BS_Runtime.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/Runtime/Macro.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniAssert.h>

#include <cstring>


namespace Glacier
{
    // PC 0x540760 (inlined into ZProjectile::Ctor) / PS2 0x2E222C: chain to the base event, clear
    // the ammo enum and let Initialize() register the messages, allocate the COLI and cache the
    // scene refs.
    ZProjectileBase::ZProjectileBase()
        : CBaseEvent<ZGEOM>()
        , m_bOwnerIsPlayer(false)
        , m_rWeaponOwner(0)
        , m_rWeapon(0)
        , m_eAmmoMaterialEnumId(0)
        , m_pWeaponTemplate(nullptr)
        , m_lStatusFlag(0)
        , m_msgActivate(0)
        , m_msgCanPenetrate(0)
        , m_msgProjectileHit(0)
        , m_msgHitObject(0)
        , m_msgWarningMessage(0)
        , m_pColi(nullptr)
    {
        Initialize();
    }

    // PC 0x53ECD0 / PS2 0x2E275C: release the collision record, then chain to ZBaseConRout.
    ZProjectileBase::~ZProjectileBase()
    {
        ZUniMemory::Delete(m_pColi);
        m_pColi = nullptr;
    }

    const RTP::ZPropertyInfo& ZProjectileBase::GetProperties() const
    {
        return ZProjectileBase::Info;
    }

    // PC 0x53F130 / PS2 0x2E22A0 / iOS 0x10031BC0C. The PC registers the messages through
    // ZEngineDataBase::RegisterZMsg, allocates 104 bytes (0x68) for the COLI and reads the three
    // scene refs from the scene COM.
    void ZProjectileBase::Initialize()
    {
        ZEngineDataBase* pEngineData = g_pEngineData;
        ZASSERT(pEngineData);

        m_msgActivate = pEngineData->RegisterZMsg("Activate", 0, __FILE__, __LINE__);
        m_msgCanPenetrate = pEngineData->RegisterZMsg("CanPenetrate", 0, __FILE__, __LINE__);
        m_msgProjectileHit = pEngineData->RegisterZMsg("ProjectileHit", 0, __FILE__, __LINE__);
        m_msgHitObject = pEngineData->RegisterZMsg("HitObject", 0, __FILE__, __LINE__);

        m_pColi = ZUniMemory::New<COLI>();
        std::memset(m_pColi, 0, sizeof(COLI));
        m_pColi->m_bBothSides = true;
        m_lStatusFlag = 0;

        m_msgWarningMessage = pEngineData->RegisterZMsg("Explode_Warning", 0, __FILE__, __LINE__);

        BS_Runtime::ZMaterialDescriptionDB& materialDb = BS_Runtime::ZMaterialDescriptionDB::Instance();
        ZASSERT(materialDb.m_ByteStream);

        m_MaterialProperty_SoundEnter = materialDb.GetPropertyId("SoundEnter");
        m_MaterialProperty_DebrisEnter = materialDb.GetPropertyId("DebrisEnter");
        m_MaterialProperty_DebrisExit = materialDb.GetPropertyId("DebrisExit");
        m_MaterialProperty_BulletHole = materialDb.GetPropertyId("BulletHole");
        m_MaterialProperty_BulletHoleSize = materialDb.GetPropertyId("HoleSize");
        m_MaterialProperty_BulletHoleSizeVariation = materialDb.GetPropertyId("HoleSizeVariation");
        m_MaterialProperty_BloodSplatter = materialDb.GetPropertyId("BloodSplatter");

        CCom* pSceneCom = pEngineData->GetSceneCom();
        ZASSERT(pSceneCom);

        int lRef = 0;
        pSceneCom->GetVal("rBulletMarks", &lRef);
        m_rBulletMarks = static_cast<uint32_t>(lRef);

        lRef = 0;
        pSceneCom->GetVal("rCrowd", &lRef);
        m_rCrowd = static_cast<uint32_t>(lRef);

        lRef = 0;
        pSceneCom->GetVal("rBloodSplatters", &lRef);
        m_rBloodSplatters = static_cast<uint32_t>(lRef);

        m_bOwnerIsPlayer = false;
        m_rWeaponOwner = 0;
        m_rWeapon = 0;
        m_eAmmoMaterialEnumId = 0;
        m_pWeaponTemplate = nullptr;
        m_lStatusFlag = 0;
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        // PS2 ZProjectileBase::Info chain at 0x9417F8 (names/filter/order), matched against the PC
        // records at 0x80764C. The PC release serializes the game offset (class offset - 4);
        // CLASS_PROPERTY yields the real class offset so both platforms agree.
        static RTP::ZDataProperty<uint32_t> StatusFlag{
            .m_Node = {.m_Next = nullptr, .m_Name = "m_lStatusFlag", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Data_uint,
            .m_Offset = CLASS_PROPERTY(ZProjectileBase, m_lStatusFlag)};

        static RTP::ZDataProperty<ZGEOMREF> WeaponTemplate{
            .m_Node = {.m_Next = StatusFlag, .m_Name = "m_pWeaponTemplate", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Data_ZGEOMREF,
            .m_Offset = reinterpret_cast<ZGEOMREF*>(CLASS_PROPERTY(ZProjectileBase, m_pWeaponTemplate))};

        static RTP::ZDataProperty<int32_t> AmmoMaterialEnumId{
            .m_Node = {.m_Next = WeaponTemplate, .m_Name = "m_eAmmoMaterialEnumId", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Data_int,
            .m_Offset = reinterpret_cast<int32_t*>(CLASS_PROPERTY(ZProjectileBase, m_eAmmoMaterialEnumId))};

        static RTP::ZDataProperty<ZGEOMREF> Weapon{
            .m_Node = {.m_Next = AmmoMaterialEnumId, .m_Name = "m_rWeapon", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Data_ZGEOMREF,
            .m_Offset = reinterpret_cast<ZGEOMREF*>(CLASS_PROPERTY(ZProjectileBase, m_rWeapon))};

        static RTP::ZDataProperty<ZGEOMREF> WeaponOwner{
            .m_Node = {.m_Next = Weapon, .m_Name = "m_rWeaponOwner", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Data_ZGEOMREF,
            .m_Offset = reinterpret_cast<ZGEOMREF*>(CLASS_PROPERTY(ZProjectileBase, m_rWeaponOwner))};

        static RTP::ZDataProperty<bool> OwnerIsPlayer{
            .m_Node = {.m_Next = WeaponOwner, .m_Name = "m_bOwnerIsPlayer", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Data_bool,
            .m_Offset = CLASS_PROPERTY(ZProjectileBase, m_bOwnerIsPlayer)};
    }

    // Rout-event class registration (ZProjectileBase derives CBaseEvent<ZGEOM> -> ZBaseConRout).
    // RoutCases = 0x30 (48), Prio = 0 (PC ZGEOM_ProjectileBallistic registration sub_73F620).
    DEFINE_ROUT_CLASS(ZProjectileBase, ZGEOM, ProjectileBase, 48, 0, 0x00807660,
                      &cProperties::OwnerIsPlayer.m_Node, ZEventBase);
#   pragma endregion

    // PC 0x9A1294 (`rRef`, bullet marks), 0x9A1298 (blood splatters) and 0x9A129C (crowd). All three
    // are populated by Initialize()'s scene-COM reads; 0xFFFFFFFF is the untouched image value.
    STATIC_CLASS_VAR_IMPL(ZProjectileBase, uint32_t, m_rBulletMarks, 0x009A1294, 0u);
    STATIC_CLASS_VAR_IMPL(ZProjectileBase, uint32_t, m_rCrowd, 0x009A129C, 0u);
    STATIC_CLASS_VAR_IMPL(ZProjectileBase, uint32_t, m_rBloodSplatters, 0x009A1298, 0u);

    // PC 0x9A1410..0x9A1428: resolved by Initialize() through ZMaterialDescriptionDB::GetPropertyId.
    STATIC_CLASS_VAR_IMPL(ZProjectileBase, int32_t, m_MaterialProperty_SoundEnter, 0x009A1410, 0);
    STATIC_CLASS_VAR_IMPL(ZProjectileBase, int32_t, m_MaterialProperty_DebrisEnter, 0x009A1414, 0);
    STATIC_CLASS_VAR_IMPL(ZProjectileBase, int32_t, m_MaterialProperty_DebrisExit, 0x009A1418, 0);
    STATIC_CLASS_VAR_IMPL(ZProjectileBase, int32_t, m_MaterialProperty_BulletHole, 0x009A141C, 0);
    STATIC_CLASS_VAR_IMPL(ZProjectileBase, int32_t, m_MaterialProperty_BulletHoleSize, 0x009A1420, 0);
    STATIC_CLASS_VAR_IMPL(ZProjectileBase, int32_t, m_MaterialProperty_BulletHoleSizeVariation, 0x009A1424, 0);
    STATIC_CLASS_VAR_IMPL(ZProjectileBase, int32_t, m_MaterialProperty_BloodSplatter, 0x009A1428, 0);
}
