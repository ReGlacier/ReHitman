#include <BloodMoney/Game/Items/ZHM3ItemBomb.h>

#include <BloodMoney/Game/Items/ZHM3ItemTemplateBomb.h>
#include <BloodMoney/Game/LevelControls/ZHM3LevelControl.h>
#include <BloodMoney/Game/ZHM3GameData.h>

#include <Glacier/Com/CCom.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/GameBase/ZCheckVisible.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/IK/ZIKLNKOBJ.h>
#include <Glacier/Physics/ZCollisionBase.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZMessageResolver.h>
#include <Glacier/ZProjectileBase.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZUniAssert.h>

#include <cstdlib>

namespace
{
    // The PC sorter (sub_64AB20, PC 0x64AB20) is a plain qsort callback that reads the blast
    // center from file-scope storage (iObjectIdx / flt_9B1564 / flt_9B1568 on the PC build);
    // mirror that here.
    Glacier::ZVector3 g_vExplosionCenter;

    int CompareExplosionTargets(const void* pLhs, const void* pRhs)
    {
        const Glacier::ZBaseGeom* pLeft = *static_cast<Glacier::ZBaseGeom* const*>(pLhs);
        const Glacier::ZBaseGeom* pRight = *static_cast<Glacier::ZBaseGeom* const*>(pRhs);

        Glacier::ZVector3 vLeft(pLeft->m_vCen);
        Glacier::ZVector3 vRight(pRight->m_vCen);
        pLeft->GetRootPoint(vLeft);
        pRight->GetRootPoint(vRight);

        vLeft -= g_vExplosionCenter;
        vRight -= g_vExplosionCenter;

        const float fLeft = vLeft.Length();
        const float fRight = vRight.Length();

        if (fLeft >= fRight)
            return fLeft > fRight;

        return -1;
    }
}

namespace Hitman
{
    // PC 0x6506E0. Forward to the weapon ctor, then clear the explosion state and construct
    // the (empty) room-shatter list.
    ZHM3ItemBomb::ZHM3ItemBomb(const char* psName, Glacier::ZBaseGeom* pBaseGeom)
        : ZHM3ItemWeapon(psName, pBaseGeom)
        , m_bExploded(false)
        , m_bExploding(false)
        , m_bTimerActivated(false)
        , m_ttTimeToExplode(0)
        , m_Lines()
        , m_nTopEvCamId(0)
        , m_ttTopViewCamStartTime(0)
    {
    }

    // vtbl slot 12 (RTTI)  PC 0x64AA80
    const Glacier::RTP::ZPropertyInfo& ZHM3ItemBomb::GetProperties() const
    {
        return ZHM3ItemBomb::Info;
    }

    // vtbl slot 13 (RTTI)  PC 0x64B3D0
    uint32_t ZHM3ItemBomb::GetObjectId() const
    {
        return ZHM3ItemBomb::m_Id;
    }

    // vtbl slot 14 (RTTI)  PC 0x64B3E0
    void ZHM3ItemBomb::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZHM3ItemBomb::m_Id;
        mask = ZHM3ItemBomb::m_Mask;
    }

    // vtbl slot 15 (RTTI)  PC 0x64AA90 -> &ZHM3ItemBomb::m_OldClassInfo
    Glacier::ZGEOMCLASSINFO* ZHM3ItemBomb::GetOldClassInfo() const
    {
        return ZHM3ItemBomb::m_OldClassInfo;
    }

    // PC 0x009B1A08 / 0x009B1A14, constructed by sub_746B60 / sub_746B80.
    STATIC_CLASS_VAR_IMPL(ZHM3ItemBomb, Glacier::ZMessageResolver, m_msgGetRoomShatterList, 0x009B1A08, { "GetRoomShatterList" });
    STATIC_CLASS_VAR_IMPL(ZHM3ItemBomb, Glacier::ZMessageResolver, m_msgGetRoomShotActivateList, 0x009B1A14, { "GetRoomShotActivateList" });

    // PC 0x64F810
    void ZHM3ItemBomb::Explode()
    {
        EnablePickup(false);
        Hide(true);

        if (m_bVisibleToNPCs && Glacier::ZCheckVisible::m_pCheckVisible != nullptr)
            Glacier::ZCheckVisible::m_pCheckVisible->RemoveSeeableItem(this);

        m_bExploded = true;
        m_bExploding = true;
        m_bTimerActivated = false;

        // ZHM3ItemBomb::GetHM3ItemTemplateBomb() (inlined on the PC build). The template must be a
        // ZHM3ItemTemplateBomb (registered id 0x100451, Info 0x0080FB0C, m_Id 0x009B1550,
        // m_Mask 0x009B1554).
        Glacier::ZItemTemplate* pTemplate = GetItemTemplate();
        ZASSERT(pTemplate != nullptr);
        ZASSERT((pTemplate->GetObjectId() & ZHM3ItemTemplateBomb::m_Mask) == ZHM3ItemTemplateBomb::m_Id);

        const float fRange = static_cast<ZHM3ItemTemplateBomb*>(pTemplate)->m_fMaxRange;

        m_iTargetCheck = 0;

        Glacier::ZMat3x3 mat;
        Glacier::ZVector3 pos;
        GetMainItemRootTM(mat.data, pos.Get());
        Glacier::mreset(mat.data);

        // The query box is lifted by 20% of the blast radius and spans {range, 0.7*range, range}.
        pos.y += fRange * 0.2f;
        const Glacier::ZVector3 size(fRange, fRange * 0.5f + fRange * 0.2f, fRange);

        Glacier::ZBaseGeom* aGeoms[30];
        const uint32_t lGeomCount = Glacier::ZCollisionBase::s_pCollisionBase->GetGeomsInBox(
            aGeoms, aGeoms + 30, Glacier::GT_StdObjs, mat.data, pos.Get(), size.Get(),
            2, false, true, true);

        m_iNumberOfTargets = 0;
        for (uint32_t i = 0; i < lGeomCount; ++i)
        {
            Glacier::ZBaseGeom* pBase = aGeoms[i];
            if (pBase == nullptr)
                continue;

            Glacier::ZGEOM* pGeom = pBase->GetGeom();
            const bool bIsLnkObj = pGeom != nullptr
                ? pGeom->IsDerivedFrom<Glacier::ZIKLNKOBJ>()
                : pBase->IsDerivedFromStdObj(Glacier::ZIKLNKOBJ::m_Id);

            if (bIsLnkObj || (pGeom != nullptr && pGeom->FindEvent("ExplosionController") != nullptr))
                m_pTargets[m_iNumberOfTargets++] = pBase;
        }

        // The crowd-controller ref is cached by ZProjectileBase::Initialize (PC 0x53F130) in
        // ZProjectileBase::m_rCrowd; the PC Explode re-reads the same scene COM value.
        Glacier::ZGEOM* pCrowd = Glacier::ZGEOM::RefToPtr(
            static_cast<Glacier::ZREF>(Glacier::ZProjectileBase::m_rCrowd));
        if (pCrowd != nullptr)
        {
            // PC 0x64F9F0: the "rCrowd" crowd-controller geom's own virtual at ZGEOM vtable offset
            // +0x1EC (slot 123) is called with the bomb's root position and a 500.0f radius (the PS2
            // spin 0x5E8E20 forwards only the position).
            using CrowdExplodeRadius = void(__thiscall*)(Glacier::ZGEOM*, const Glacier::ZVector3*, float);
            auto function = reinterpret_cast<CrowdExplodeRadius>((*reinterpret_cast<void***>(pCrowd))[123]);
            function(pCrowd, &pos, 500.0f);
        }

        // The blast center is the parent group's base-geom center, transformed to world space.
        Glacier::ZBaseGeom* pCenterBase = m_baseGeom->ParentGroup()->m_baseGeom;
        g_vExplosionCenter = pCenterBase->m_vCen;
        GetRootPoint(g_vExplosionCenter);

        qsort(m_pTargets, m_iNumberOfTargets, sizeof(Glacier::ZBaseGeom*), CompareExplosionTargets);

        Glacier::g_pEngineData->RegisterZMsg("MSG_NOTIFYDUPLICATEGEOM", 0, __FILE__, __LINE__);

        Glacier::ZROOM* aRooms[32];
        const uint32_t lRoomCount = Glacier::ZCollisionBase::s_pCollisionBase->GetInnerRoomsLst(
            aRooms, aRooms + 32, mat.data, pos.Get(), size.Get(), false);

        if (Glacier::g_pSysInterface->m_pSoundDll != nullptr && lRoomCount != 0)
        {
            Glacier::ZMat3x3 soundMat = mat;
            Glacier::ZVector3 soundPos = pos;
            aRooms[0]->GetLocalMatPos(soundMat, soundPos);
            aRooms[0]->AddSound3d(soundMat.data, soundPos.Get(), 0, 14, 0, 0);
        }

        for (uint32_t i = 0; i < lRoomCount; ++i)
        {
            // Room shatter list: gather every shattered geom inside the blast radius.
            Glacier::REFTAB* pShatterList = nullptr;
            ZASSERT(m_msgGetRoomShatterList.m_MessageID != 0);
            SendCommand(aRooms[i], static_cast<Glacier::ZMSGID>(m_msgGetRoomShatterList), &pShatterList);

            if (pShatterList != nullptr)
            {
                Glacier::RefRun run;
                pShatterList->RunInitNxtRef(&run);
                for (uint32_t r = pShatterList->RunNxtRef(&run); run; r = pShatterList->RunNxtRef(&run))
                {
                    Glacier::ZGEOM* pGeom = Glacier::ZGEOM::RefToPtr(r);
                    if (pGeom == nullptr)
                        continue;

                    if (m_iNumberOfTargets >= 30)
                        break;

                    Glacier::ZBaseGeom* pBase = pGeom->m_baseGeom;
                    Glacier::ZVector3 vGeom(pBase->m_vCen);
                    pGeom->GetRootPoint(vGeom);

                    if (Glacier::vdist(pos.Get(), vGeom.Get()) <= fRange)
                        m_pTargets[m_iNumberOfTargets++] = pBase;
                }
            }

            // Room shot-activate list: gather the still-active geoms (skip the masked-out ones).
            pShatterList = nullptr;
            ZASSERT(m_msgGetRoomShotActivateList.m_MessageID != 0);
            SendCommand(aRooms[i], static_cast<Glacier::ZMSGID>(m_msgGetRoomShotActivateList), &pShatterList);

            if (pShatterList != nullptr)
            {
                Glacier::RefRun run;
                pShatterList->RunInitNxtRef(&run);
                for (uint32_t r = pShatterList->RunNxtRef(&run); run; r = pShatterList->RunNxtRef(&run))
                {
                    Glacier::ZGEOM* pGeom = Glacier::ZGEOM::RefToPtr(r);
                    if (pGeom == nullptr)
                        continue;

                    Glacier::ZBaseGeom* pBase = pGeom->m_baseGeom;
                    if ((pBase->m_lControl & 0x400) != 0)
                        continue;

                    if (m_iNumberOfTargets >= 30)
                        break;

                    Glacier::ZVector3 vGeom(pBase->m_vCen);
                    pGeom->GetRootPoint(vGeom);

                    if (Glacier::vdist(pos.Get(), vGeom.Get()) <= fRange)
                        m_pTargets[m_iNumberOfTargets++] = pBase;
                }
            }
        }

        Glacier::ZItemTemplate* pTemplate2 = GetItemTemplate();
        ZASSERT(pTemplate2 != nullptr);
        ZASSERT((pTemplate2->GetObjectId() & ZHM3ItemTemplateBomb::m_Mask) == ZHM3ItemTemplateBomb::m_Id);

        if (static_cast<ZHM3ItemTemplateBomb*>(pTemplate2)->m_pEffectGroup != nullptr)
            ActivateEffect();

        ZHM3GameData* pGameData = static_cast<ZHM3GameData*>(Glacier::g_pGameData);
        ZHM3LevelControl* pLevelControl = pGameData != nullptr ? pGameData->GetLevelControl() : nullptr;
        if (pLevelControl != nullptr)
            pLevelControl->BombExploded(this);
    }

    // PC 0x64CE00
    void ZHM3ItemBomb::ActivateEffect()
    {
        Glacier::ZROOM* pCurrentRoom = m_baseGeom->GetOwnerRoom();
        ZASSERT(pCurrentRoom != nullptr);
        ZASSERT((pCurrentRoom->GetObjectId() & Glacier::ZROOM::m_Mask) == Glacier::ZROOM::m_Id);

        Glacier::ZMat3x3 mat;
        Glacier::ZVector3 pos;
        GetMainItemRootTM(mat.data, pos.Get());
        Glacier::mreset(mat.data);

        Glacier::ZItemTemplate* pTemplate = GetItemTemplate();
        ZASSERT(pTemplate != nullptr);
        ZASSERT((pTemplate->GetObjectId() & ZHM3ItemTemplateBomb::m_Mask) == ZHM3ItemTemplateBomb::m_Id);

        Glacier::ZGROUP* pEffectGroup = static_cast<ZHM3ItemTemplateBomb*>(pTemplate)->m_pEffectGroup;
        Glacier::ZGEOM* pEffect = pEffectGroup->DuplicateInit(Glacier::g_pEngineData->m_pRoot, &mat, &pos, nullptr, false);
        pEffect->MakeDynamic(true);
        pEffect->SetAutoRoomAssign(true);
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        // The PC chain is laid out tail-first; the head (m_bExploded) is the FirstProperty passed
        // to DECLARE_GEOM_CLASS_IMPL. Game offset -> class offset = +4
        // (ZHM3ItemBomb::Info.First is 0x80FBF4).
        static Glacier::RTP::ZDataProperty<Glacier::TIMETYPE> TopViewCamStartTime{
            .m_Node = {.m_Next = nullptr, .m_Name = "m_ttTopViewCamStartTime", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_TIMETYPE,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemBomb, m_ttTopViewCamStartTime)};

        static Glacier::RTP::ZDataProperty<uint32_t> TopEvCamId{
            .m_Node = {.m_Next = TopViewCamStartTime, .m_Name = "m_nTopEvCamId", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_uint,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemBomb, m_nTopEvCamId)};

        // PC 0x80FB40: m_pPlaceBombActionGeom (game offset 0x290, vtbl 0x0080655C) serializes as a
        // REF name, using the same data-property table as ZProjectileBase::m_pWeaponTemplate. It sits
        // between m_Lines and m_nTopEvCamId.
        static Glacier::RTP::ZDataProperty<Glacier::ZGEOMREF> PlaceBombActionGeom{
            .m_Node = {.m_Next = TopEvCamId, .m_Name = "m_pPlaceBombActionGeom", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_ZGEOMREF,
            .m_Offset = reinterpret_cast<Glacier::ZGEOMREF*>(CLASS_PROPERTY(ZHM3ItemBomb, m_pPlaceBombActionGeom))};

        static Glacier::RTP::ZDataProperty<Glacier::REFTAB32> Lines{
            .m_Node = {.m_Next = PlaceBombActionGeom, .m_Name = "m_Lines", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_REFTAB32,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemBomb, m_Lines)};

        static Glacier::RTP::ZDataProperty<uint32_t> NumberOfLines{
            .m_Node = {.m_Next = Lines, .m_Name = "m_iNumberOfLines", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_uint,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemBomb, m_iNumberOfLines)};

        static Glacier::RTP::ZDataProperty<Glacier::TIMETYPE> TimeToExplode{
            .m_Node = {.m_Next = NumberOfLines, .m_Name = "m_ttTimeToExplode", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_TIMETYPE,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemBomb, m_ttTimeToExplode)};

        static Glacier::RTP::ZDataProperty<uint32_t> TargetCheck{
            .m_Node = {.m_Next = TimeToExplode, .m_Name = "m_iTargetCheck", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_uint,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemBomb, m_iTargetCheck)};

        // PC 0x80FBA4: m_pTargets (game offset 0x160, vtbl 0x0080647C) serializes as a REF name. It
        // sits between m_iNumberOfTargets and m_iTargetCheck.
        static Glacier::RTP::ZDataProperty<Glacier::ZGEOMREF> Targets{
            .m_Node = {.m_Next = TargetCheck, .m_Name = "m_pTargets", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_ZGEOMREF,
            .m_Offset = reinterpret_cast<Glacier::ZGEOMREF*>(CLASS_PROPERTY(ZHM3ItemBomb, m_pTargets))};

        static Glacier::RTP::ZDataProperty<uint32_t> NumberOfTargets{
            .m_Node = {.m_Next = Targets, .m_Name = "m_iNumberOfTargets", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_uint,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemBomb, m_iNumberOfTargets)};

        static Glacier::RTP::ZDataProperty<bool> TimerActivated{
            .m_Node = {.m_Next = NumberOfTargets, .m_Name = "m_bTimerActivated", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_bool,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemBomb, m_bTimerActivated)};

        static Glacier::RTP::ZDataProperty<bool> Exploding{
            .m_Node = {.m_Next = TimerActivated, .m_Name = "m_bExploding", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_bool,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemBomb, m_bExploding)};

        static Glacier::RTP::ZDataProperty<bool> Exploded{
            .m_Node = {.m_Next = Exploding, .m_Name = "m_bExploded", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_bool,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemBomb, m_bExploded)};
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZHM3ItemBomb,        // ClassName
        ZHM3ItemWeapon,      // BaseClass
        0x009B1928,          // OldClassInfoAddr
        "ZHM3ItemBomb",      // FactoryName
        0x0,                 // FactoryNameAddr (documentation only; not bound by the macro)
        cProperties::Exploded, // FirstProperty
        0x0080FC08,          // PropertiesAddr (ZHM3ItemBomb::Info)
        0x009B1548,          // IdAddr (ZHM3ItemBomb::m_Id)
        0x009B154C           // MaskAddr (ZHM3ItemBomb::m_Mask)
    );
#   pragma endregion
}
