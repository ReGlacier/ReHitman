#include <BloodMoney/Game/Physics/CRigidBody.h>
#include <BloodMoney/Game/ZHM3Actor.h>
#include <BloodMoney/Game/ZHM3GameData.h>
#include <Glacier/Physics/ZCollisionBox.h>
#include <Glacier/Physics/ZCollisionBase.h>
#include <BloodMoney/Game/Physics/ZRigidBodyPoolManager.h>
#include <Glacier/Physics/COLI.h>
#include <Glacier/Physics/SExtendedImpactInfo.h>
#include <Glacier/Physics/ZCommonAlgorithms.h>

#include <Glacier/Audio/ZSoundDllBase.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/EventBase/ZEventBuffer.h>
#include <Glacier/Geom/ZGeomBuffer.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/Geom/ZSTDOBJ.h>
#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/Items/ZItem.h>
#include <Glacier/Items/ZItemContainer.h>
#include <Glacier/Render/Prim/ZPrimAccess.h>
#include <Glacier/Render/Prim/ZPrimAccessMesh.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Materials/BS_Runtime.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZUniMemory.h>

#include <cmath>
#include <cstring>

// <windows.h> (pulled in transitively by the audio/engine headers) defines `PlaySound` as
// `PlaySoundA`; keep the reversed member name intact.
#ifdef PlaySound
#undef PlaySound
#endif

namespace Hitman
{
    // ---------------------------------------------------------------------------------------------
    // Statics
    // ---------------------------------------------------------------------------------------------

    // RTTI. The PC property chain lives at 0x815424..0x815514 and is stored tail-first; its node
    // names are stripped (cNode.m_Name == nullptr) in the retail image. Recovered list order with
    // (offset, filter): 0x7C(2), 0xA0(2), 0xBC(2), 0xC8(2), 0xD0(2), 0xD4(2), 0xD8(2), 0xDC(2),
    // RemoveAfterUse, 0xE0(3), Frozen, 0xE4(2), 0xE8(2), 0xEC(2) -> tail.
    namespace cProperties
    {
        // Property names come from the PS2 build (0x954640..); the retail PC image has them stripped.
        // The PS2 property offsets match this project's layout (the PC ones are 4 bytes smaller).
        // PC chain head is 0x815500 (m_QMat), the tail is 0x815424 (m_rContainingElevator).

        static Glacier::RTP::ZDataProperty<uint32_t> Property_rContainingElevator{
            .m_Node = {.m_Next = nullptr, .m_Name = "m_rContainingElevator", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_uint,
            .m_Offset = CLASS_PROPERTY(CRigidBody, m_rContainingElevator)};

        static Glacier::RTP::ZDataProperty<uint8_t> Property_Status{
            .m_Node = {.m_Next = Property_rContainingElevator, .m_Name = "m_Status", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_uchar,
            .m_Offset = CLASS_PROPERTY(CRigidBody, m_Status)};

        static Glacier::RTP::ZVirtualProperty<bool> Property_Frozen{
            .m_Node = {.m_Next = Property_Status, .m_Name = "Frozen", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Virtual_bool,
            .m_Get = &CRigidBody::GetFrozen,
            .m_Set = &CRigidBody::SetFrozen};

        static Glacier::RTP::ZDataProperty<uint32_t> Property_rMaterial{
            .m_Node = {.m_Next = Property_Frozen, .m_Name = "m_rMaterial", .m_Filter = 3},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_uint,
            .m_Offset = CLASS_PROPERTY(CRigidBody, m_rMaterial)};

        static Glacier::RTP::ZVirtualProperty<bool> Property_RemoveAfterUse{
            .m_Node = {.m_Next = Property_rMaterial, .m_Name = "RemoveAfterUse", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Virtual_bool,
            .m_Get = &CRigidBody::GetRemoveAfterUse,
            .m_Set = &CRigidBody::SetRemoveAfterUse};

        static Glacier::RTP::ZDataProperty<uint16_t> Property_iImpactNum{
            .m_Node = {.m_Next = Property_RemoveAfterUse, .m_Name = "m_iImpactNum", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_ushort,
            .m_Offset = CLASS_PROPERTY(CRigidBody, m_iImpactNum)};

        static Glacier::RTP::ZDataProperty<int32_t> Property_fLastHitTime{
            .m_Node = {.m_Next = Property_iImpactNum, .m_Name = "m_fLastHitTime", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_int,
            .m_Offset = reinterpret_cast<int32_t*>(offsetof(CRigidBody, m_fLastHitTime))};

        static Glacier::RTP::ZDataProperty<float> Property_fLastNrg{
            .m_Node = {.m_Next = Property_fLastHitTime, .m_Name = "m_fLastNrg", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_float,
            .m_Offset = CLASS_PROPERTY(CRigidBody, m_fLastNrg)};

        static Glacier::RTP::ZDataProperty<float> Property_fThreshold{
            .m_Node = {.m_Next = Property_fLastNrg, .m_Name = "m_fThreshold", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_float,
            .m_Offset = CLASS_PROPERTY(CRigidBody, m_fThreshold)};

        static Glacier::RTP::ZDataProperty<uint16_t> Property_nTimeOut{
            .m_Node = {.m_Next = Property_fThreshold, .m_Name = "m_nTimeOut", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_ushort,
            .m_Offset = CLASS_PROPERTY(CRigidBody, m_nTimeOut)};

        static Glacier::RTP::ZDataProperty<float[3]> Property_OldPos{
            .m_Node = {.m_Next = Property_nTimeOut, .m_Name = "m_OldPos", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_float_3,
            .m_Offset = reinterpret_cast<float(*)[3]>(offsetof(CRigidBody, m_OldPos))};

        static Glacier::RTP::ZDataProperty<float[3]> Property_QPos{
            .m_Node = {.m_Next = Property_OldPos, .m_Name = "m_QPos", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_float_3,
            .m_Offset = reinterpret_cast<float(*)[3]>(offsetof(CRigidBody, m_QPos))};

        static Glacier::RTP::ZDataProperty<float[9]> Property_QMat{
            .m_Node = {.m_Next = Property_QPos, .m_Name = "m_QMat", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_float_9,
            .m_Offset = reinterpret_cast<float(*)[9]>(offsetof(CRigidBody, m_QMat))};
    }

    DEFINE_ROUT_CLASS(
        CRigidBody,    // Class
        Glacier::ZBaseConRout,  // BaseClass
        CRigidBody,    // FactoryName
        0,             // RoutCases
        0,             // Prio
        0x00815514,    // PropertiesAddr (CRigidBody::Info)
        cProperties::Property_QMat, // FirstProperty
        Glacier::ZBaseConRout   // SuperClass (Info chain)
    );

    STATIC_CLASS_VAR_IMPL(CRigidBody, Glacier::ZMessageResolver, m_msgProjectileHit, 0x009A349C, Glacier::ZMessageResolver{ "ProjectileHit" });
    STATIC_CLASS_VAR_IMPL(CRigidBody, Glacier::ZMessageResolver, m_msgExplodeBomb, 0x009A34A8, Glacier::ZMessageResolver{ "ExplodeBomb" });
    STATIC_CLASS_VAR_IMPL(CRigidBody, Glacier::ZMessageResolver, m_msgIKBomb_Explode, 0x009A34B4, Glacier::ZMessageResolver{ "IKBomb_Explode" });
    STATIC_CLASS_VAR_IMPL(CRigidBody, Glacier::ZMessageResolver, m_msgActivate, 0x009A34C0, Glacier::ZMessageResolver{ "Activate" });
    STATIC_CLASS_VAR_IMPL(CRigidBody, Glacier::ZMessageResolver, m_msgSetVelocity, 0x009A34D8, Glacier::ZMessageResolver{ "SetVelocity" });
    STATIC_CLASS_VAR_IMPL(CRigidBody, Glacier::ZMessageResolver, m_msgSetPos, 0x009A34CC, Glacier::ZMessageResolver{ "SetPosition" });
    STATIC_CLASS_VAR_IMPL(CRigidBody, Glacier::ZMessageResolver, m_msgSetMaterial, 0x009A34E4, Glacier::ZMessageResolver{ "SetMaterial" });
    STATIC_CLASS_VAR_IMPL(CRigidBody, Glacier::ZMessageResolver, m_msgSendImpactEvent, 0x009A34F0, Glacier::ZMessageResolver{ "SendImpactEvent" });
    STATIC_CLASS_VAR_IMPL(CRigidBody, Glacier::ZMessageResolver, m_msgFreeze, 0x009A34FC, Glacier::ZMessageResolver{ "Freeze" });
    STATIC_CLASS_VAR_IMPL(CRigidBody, Glacier::ZMessageResolver, m_msgThaw, 0x009A3508, Glacier::ZMessageResolver{ "Thaw" });
    STATIC_CLASS_VAR_IMPL(CRigidBody, Glacier::ZMessageResolver, m_msgCollision, 0x009A3514, Glacier::ZMessageResolver{ "Collision" });
    STATIC_CLASS_VAR_IMPL(CRigidBody, Glacier::ZMessageResolver, m_msgRemove, 0x009A3520, Glacier::ZMessageResolver{ "Remove" });
    STATIC_CLASS_VAR_IMPL(CRigidBody, Glacier::ZMessageResolver, m_msgRequestDeltaY, 0x009A352C, Glacier::ZMessageResolver{ "RequestDeltaY" });
    STATIC_CLASS_VAR_IMPL(CRigidBody, Glacier::ZMessageResolver, m_msgGetElevBound, 0x009A3538, Glacier::ZMessageResolver{ "GETELEVBOUND" });
    STATIC_CLASS_VAR_IMPL(CRigidBody, Glacier::ZMessageResolver, m_msgGetHatchGeom, 0x009A3544, Glacier::ZMessageResolver{ "GetHatchGeom" });
    STATIC_CLASS_VAR_IMPL(CRigidBody, Glacier::TAudioPropertyID, m_MaterialProperty_SoundMaterial, 0x009A3498, {});

    namespace
    {
        // PC 0x009A3550: cached Glacier::TMaterialDescID of the "Water" material, lazily set by Setup2().
        int32_t g_WaterMaterialDescriptionId = -1;

        inline bool HasStatus(const uint8_t status, const uint8_t flag) { return (status & flag) != 0; }
    }

    // ---------------------------------------------------------------------------------------------
    // Construction / lifetime
    // ---------------------------------------------------------------------------------------------

    CRigidBody::CRigidBody()
    {
        m_id = 0xFFFF;
        m_iImpactNum = 0;
        m_rMaterial = 0;
        m_pHitAnything = nullptr;
        m_nTimeOut = 1000;
        m_fWeightedSpeed = 1600.0f;
        m_fThreshold = 400.0f;
        m_nNumVertices = 0;
        m_pVertexReps = nullptr;
        m_Status = 24;
        m_fLastHitTime = Glacier::TIMETYPE{ 0 };
        m_rContainingElevator = 0;
        m_bDoNotAddSoundEvent = false;

        m_MaterialProperty_SoundMaterial = Glacier::BS_Runtime::ZMaterialDescriptionDB::Instance().GetAudioPropertyId("SoundMaterial");
    }

    CRigidBody::~CRigidBody()
    {
        ZUniMemory::Free(m_pVertexReps);
        m_pVertexReps = nullptr;
    }

    const Glacier::RTP::ZPropertyInfo& CRigidBody::GetProperties() const
    {
        return Info;
    }

    const Glacier::RTP::ZPropertyInfo& CRigidBody::Properties()
    {
        return Info;
    }

    void CRigidBody::Init()
    {
        ZASSERT(m_pBaseGeom);
        m_pBaseGeom->m_baseGeom->SetControl(0, 1);

        ZASSERT(m_pBaseGeom);
        m_pBaseGeom->SetMoving(true);

        ZASSERT(m_pBaseGeom);
        m_pBaseGeom->CreateExData();

        DeactivateFrameUpdate();
    }

    void CRigidBody::End()
    {
        ZUniMemory::Free(m_pVertexReps);
        m_pVertexReps = nullptr;
    }

    void CRigidBody::CopyData(const Glacier::ZEventBase* source)
    {
        const CRigidBody* pSource = static_cast<const CRigidBody*>(source);
        m_rMaterial = pSource->m_rMaterial;
        m_Status = pSource->m_Status;
        m_rContainingElevator = pSource->m_rContainingElevator;
    }

    bool CRigidBody::PostLoad(Glacier::ISerializerStream& stream)
    {
        const bool bBase = ZSerializable::PostLoad(stream);

        // PC 0x576170: Setup2 runs when the stream status bit 1 is set, i.e. `(1 << m_Status) & 2`.
        if ((1 << static_cast<int32_t>(stream.m_Status)) & 2)
            Setup2();

        return bBase;
    }

    // ---------------------------------------------------------------------------------------------
    // Properties / commands
    // ---------------------------------------------------------------------------------------------

    int32_t CRigidBody::Command(Glacier::ZMSGID command, Glacier::ZDATA data)
    {
        if (m_msgProjectileHit == command)
        {
            if (data)
                HandleHit(static_cast<Glacier::SHitInfo*>(data));
            return 0;
        }

        if (m_msgExplodeBomb == command || m_msgIKBomb_Explode == command)
        {
            if (data)
                HandleExplodeBomb(static_cast<Glacier::SExplosionInfo*>(data));
            return 0;
        }

        if (m_msgActivate == command)
        {
            SetupTransform();
            Enable();
            return 0;
        }

        if (m_msgSetPos == command)
        {
            SetPos(static_cast<const Glacier::ZVector3*>(data));
            return 0;
        }

        if (m_msgSetVelocity == command)
        {
            SetVelocity(static_cast<const Glacier::SRigidBodyVelocity*>(data));
            return 0;
        }

        if (m_msgSetMaterial == command)
        {
            m_rMaterial = *static_cast<const uint32_t*>(data);
            return 0;
        }

        if (m_msgSendImpactEvent == command)
        {
            if (*static_cast<const uint8_t*>(data))
                m_Status |= 0x10;
            else
                m_Status &= ~0x10u;
            return 0;
        }

        if (m_msgFreeze == command)
        {
            m_Status |= 4;
            return 0;
        }

        if (m_msgThaw == command)
        {
            m_Status &= ~4u;
            SetupTransform();
            return 0;
        }

        if (m_msgRemove == command)
        {
            DisableRemove(true);
            return 0;
        }

        return 0;
    }

    bool CRigidBody::Activated()
    {
        return HasStatus(m_Status, 1);
    }

    void CRigidBody::GetRemoveAfterUse(bool& bRemoveAfterUse)
    {
        bRemoveAfterUse = HasStatus(m_Status, 8);
    }

    void CRigidBody::SetRemoveAfterUse(const bool& bRemoveAfterUse)
    {
        if (bRemoveAfterUse)
            m_Status |= 8;
        else
            m_Status &= ~8u;
    }

    void CRigidBody::GetFrozen(bool& bFrozen)
    {
        bFrozen = HasStatus(m_Status, 4);
    }

    void CRigidBody::SetFrozen(const bool& bFrozen)
    {
        if (bFrozen)
            m_Status |= 4;
        else
            m_Status &= ~4u;
    }

    // ---------------------------------------------------------------------------------------------
    // Setup
    // ---------------------------------------------------------------------------------------------

    void CRigidBody::Setup()
    {
        Setup2();
    }

    void CRigidBody::Setup2()
    {
        m_Status &= ~1u;
        UpdateElevatorState();

        bool bDoNotAddSound = m_rContainingElevator != 0;

        // PC: any child geom that is (or derives from) a ZHM3Actor also forbids the impact sound event.
        ZASSERT(m_pBaseGeom);
        if ((Glacier::ZGROUP::m_Mask & m_pBaseGeom->GetObjectId()) == Glacier::ZGROUP::m_Id)
        {
            const Glacier::ZGROUP* pGroup = static_cast<const Glacier::ZGROUP*>(m_pBaseGeom);
            for (const Glacier::ZBaseGeom* i = pGroup->m_pGroupFirst; i; i = i->Next())
            {
                const Glacier::ZGEOM* pChildGeom = i->GetGeom();
                if (pChildGeom)
                {
                    if ((Hitman::ZHM3Actor::m_Mask & pChildGeom->GetObjectId()) == Hitman::ZHM3Actor::m_Id)
                        bDoNotAddSound = true;
                }
                else if (i->IsDerivedFromStdObj(Hitman::ZHM3Actor::m_Id))
                {
                    bDoNotAddSound = true;
                }
            }
        }

        ZASSERT(m_pBaseGeom);
        if ((Glacier::ZItem::m_Mask & m_pBaseGeom->GetObjectId()) == Glacier::ZItem::m_Id)
        {
            ZASSERT(m_pBaseGeom);
            const char* pName = m_pBaseGeom->Name();
            if (!pName)
                pName = "<NONAME>";
            if (strcmp(pName, "Item_Coin") != 0)
                m_Status |= 0x20;
        }

        m_bDoNotAddSoundEvent = bDoNotAddSound;

        if (!g_WaterMaterialDescriptionId)
            g_WaterMaterialDescriptionId = Glacier::BS_Runtime::ZMaterialDescriptionDB::Instance().GetMaterialDescriptionId("Water").m_Value;

        Glacier::ZVector3 aVertices[6000 / 12];
        const uint32_t lCount = GetVertices(aVertices, false);
        m_nNumVertices = lCount;

        if (lCount)
        {
            InitCentroid(aVertices);
            InitParticles();
            InitVertexReps(aVertices);

            ZASSERT(m_pBaseGeom);
            m_pBaseGeom->m_baseGeom->SetControl(0, 32);
            m_Status |= 1;
        }
    }

    bool CRigidBody::AddVertex(Glacier::ZVector3* pVertices, const Glacier::ZVector3* pPoint, uint32_t& lCount, float fMinDistSq)
    {
        if (lCount >= kMaxVertices)
            return false;

        for (uint32_t i = 0; i < lCount; ++i)
        {
            Glacier::ZVector3 d;
            Glacier::vsub(d.Get(), pVertices[i].Get(), pPoint->Get());
            if (Glacier::vdot(d.Get(), d.Get()) < fMinDistSq)
                return true;
        }

        pVertices[lCount] = *pPoint;
        ++lCount;
        return true;
    }

    void CRigidBody::AddBBox(Glacier::ZVector3* pVertices, uint32_t& lCount, const float* pfCen, const float* pfSize)
    {
        for (uint32_t i = 0; i < 8; ++i)
        {
            Glacier::ZVector3 corner;
            corner.x = pfCen[0] + ((i & 1) ? pfSize[0] : -pfSize[0]);
            corner.y = pfCen[1] + ((i & 2) ? pfSize[1] : -pfSize[1]);
            corner.z = pfCen[2] + ((i & 4) ? pfSize[2] : -pfSize[2]);
            AddVertex(pVertices, &corner, lCount, 0.001f);
        }
    }

    bool CRigidBody::AddVertices(const Glacier::ZBaseGeom* pBaseGeom, Glacier::ZVector3* pVertices, uint32_t& lCount)
    {
        ZASSERT(pBaseGeom);

        Glacier::ZVector3 vSize;
        pBaseGeom->GetSize(vSize);

        float fMaxSize = vSize.x;
        if (vSize.y > fMaxSize)
            fMaxSize = vSize.y;
        if (vSize.z > fMaxSize)
            fMaxSize = vSize.z;

        const float fMinDistSq = fMaxSize * 0.25f;
        bool bResult = true;

        uint32_t lSubPrim = 0;
        for (uint32_t hPrim = Glacier::g_pRenderDll->m_pPrimControl->GetSubPrim(pBaseGeom->m_lPrim, 0);
             hPrim;
             hPrim = Glacier::g_pRenderDll->m_pPrimControl->GetSubPrim(pBaseGeom->m_lPrim, lSubPrim))
        {
            Glacier::ZPrimHandle hPrimHandle;
            hPrimHandle.m_lHandleValue = hPrim;

            Glacier::ZPrimAccessMesh* pMesh = static_cast<Glacier::ZPrimAccessMesh*>(Glacier::ZPrimAccess::Create(hPrimHandle));
            if (pMesh)
            {
                pMesh->Lock(Glacier::ZPrimAccess::LF_READONLY);

                const uint32_t lNumVertices = pMesh->GetNumVertices();
                for (uint32_t v = 0; v < lNumVertices; ++v)
                {
                    Glacier::ZVector3 pos;
                    pMesh->GetPositions(v, 1, pos.Get());

                    // Child-local -> world -> this body's local space.
                    pBaseGeom->GetRootPoint(pos);
                    m_pBaseGeom->GetLocalPoint(pos);

                    if (!AddVertex(pVertices, &pos, lCount, fMinDistSq))
                    {
                        bResult = false;
                        break;
                    }
                }

                pMesh->Unlock();
                pMesh->Destroy();
            }

            ++lSubPrim;
        }

        return bResult;
    }

    uint32_t CRigidBody::GetVertices(Glacier::ZVector3* pVertices, bool bForced)
    {
        ZASSERT(m_pBaseGeom);

        uint32_t lCount = 0;

        if (!bForced)
        {
            if ((Glacier::ZSTDOBJ::m_Mask & m_pBaseGeom->GetObjectId()) == Glacier::ZSTDOBJ::m_Id)
            {
                if (!AddVertices(m_pBaseGeom->m_baseGeom, pVertices, lCount))
                    lCount = 0;
            }
            else if ((Glacier::ZGROUP::m_Mask & m_pBaseGeom->GetObjectId()) == Glacier::ZGROUP::m_Id)
            {
                if ((Glacier::ZItemContainer::m_Mask & m_pBaseGeom->GetObjectId()) == Glacier::ZItemContainer::m_Id)
                {
                    // Glacier::ZItemContainer: walk its linked base-geom list (backwards, like the PC).
                    for (Glacier::ZBaseGeom* i = m_pBaseGeom->m_baseGeom; i && Glacier::ForNotGroupsCheck(i); i = i->GetPrev())
                    {
                        if (i->IsDerivedFromStdObj(Glacier::ZSTDOBJ::m_Id) && !AddVertices(i, pVertices, lCount))
                        {
                            lCount = 0;
                            break;
                        }
                    }
                }
                else
                {
                    // Plain Glacier::ZGROUP: walk every child geom in the subtree.
                    Glacier::ZBaseGeom* i = m_pBaseGeom->m_baseGeom;
                    while (i)
                    {
                        if (i->IsDerivedFromStdObj(Glacier::ZSTDOBJ::m_Id) && !AddVertices(i, pVertices, lCount))
                        {
                            lCount = 0;
                            break;
                        }

                        m_pBaseGeom->RecurGetNext(&i);
                    }
                }
            }
        }

        if (!lCount || lCount >= kMaxVertices)
        {
            // Fallback (and the forced path): bound boxes.
            lCount = 0;

            if ((Glacier::ZGROUP::m_Mask & m_pBaseGeom->GetObjectId()) == Glacier::ZGROUP::m_Id)
            {
                for (Glacier::ZBaseGeom* j = m_pBaseGeom->m_baseGeom; j; j = j->Next())
                {
                    Glacier::ZVector3 vCen, vSize;

                    if (j->IsDerivedFromStdObj(Glacier::ZLNKOBJ::m_Id))
                    {
                        // Glacier::ZLNKOBJ children report the bone-extended (skeleton) bounds instead of
                        // the cached base-geom box.
                        geom_cast<Glacier::ZLNKOBJ>(j->GetGeom())->GetBonesCenSize(vCen.Get(), vSize.Get());
                    }
                    else if (j->IsDerivedFromStdObj(Glacier::ZSTDOBJ::m_Id))
                    {
                        j->GetCen(vCen);
                        j->GetSize(vSize);
                    }
                    else
                    {
                        continue;
                    }

                    Glacier::TransformBox(j->Mat(), vSize.Get());
                    Glacier::vmmul(vCen.Get(), j->Mat());
                    Glacier::vadd(vCen.Get(), j->Pos());
                    AddBBox(pVertices, lCount, vCen.Get(), vSize.Get());
                }
            }
            else if ((Glacier::ZLNKOBJ::m_Mask & m_pBaseGeom->GetObjectId()) == Glacier::ZLNKOBJ::m_Id)
            {
                Glacier::ZVector3 vCen, vSize;
                geom_cast<Glacier::ZLNKOBJ>(m_pBaseGeom)->GetBonesCenSize(vCen.Get(), vSize.Get());
                AddBBox(pVertices, lCount, vCen.Get(), vSize.Get());
            }
            else if ((Glacier::ZSTDOBJ::m_Mask & m_pBaseGeom->GetObjectId()) == Glacier::ZSTDOBJ::m_Id)
            {
                Glacier::ZVector3 vCen, vSize;
                m_pBaseGeom->GetCen(vCen);
                m_pBaseGeom->GetSize(vSize);
                AddBBox(pVertices, lCount, vCen.Get(), vSize.Get());
            }

            return lCount >= kMaxVertices ? 0 : lCount;
        }

        return lCount;
    }

    void CRigidBody::InitCentroid(Glacier::ZVector3* pVertices)
    {
        ZASSERT(m_pBaseGeom);

        m_Centroid.Reset();

        for (uint32_t i = 0; i < m_nNumVertices; ++i)
            Glacier::vadd(m_Centroid.Get(), pVertices[i].Get());

        Glacier::vscalar(m_Centroid.Get(), 1.0f / static_cast<float>(m_nNumVertices));

        m_LocalCentroid = m_Centroid;

        ZASSERT(m_pBaseGeom);
        m_pBaseGeom->GetRootPoint(m_Centroid);
    }

    void CRigidBody::InitVertexReps(Glacier::ZVector3* pVertices)
    {
        Glacier::ZVector3 v0, v1, v2, v3;
        Tetrahedron(v0, v1, v2, v3);

        float base[9];
        Glacier::vsub(&base[0], v1.Get(), v0.Get());
        Glacier::vsub(&base[3], v2.Get(), v0.Get());
        Glacier::vsub(&base[6], v3.Get(), v0.Get());

        ZUniMemory::Free(m_pVertexReps);
        m_pVertexReps = static_cast<SVertexRep*>(ZUniMemory::Allocate(sizeof(SVertexRep) * m_nNumVertices));

        for (uint32_t i = 0; i < m_nNumVertices; ++i)
        {
            Glacier::ZVector3 v = pVertices[i];
            Glacier::vsub(v.Get(), m_LocalCentroid.Get());
            Glacier::vsub(v.Get(), v0.Get());

            Glacier::ZVector3 weights;
            Glacier::ZCommonAlgorithms::Solve3x3System(base, v.Get(), weights.Get());

            SVertexRep& rep = m_pVertexReps[i];
            rep.c[0] = 1.0f - weights.x - weights.y - weights.z;
            rep.c[1] = weights.x;
            rep.c[2] = weights.y;
            rep.c[3] = weights.z;
            rep.fQuadSum = rep.c[0] * rep.c[0] + rep.c[1] * rep.c[1] + rep.c[2] * rep.c[2] + rep.c[3] * rep.c[3];
        }

        Glacier::ZMat3x3 mat;
        Glacier::ZVector3 pos;
        m_pBaseGeom->GetMatPos(mat, pos);
        m_pBaseGeom->GetRootPoint(pos);

        SetMatPosFromTetrahedron();

        Glacier::ZBaseGeom* pBase = m_pBaseGeom->m_baseGeom;
        if (pBase->ParentGroup())
        {
            Glacier::ZGROUP* pParent = pBase->ParentGroup();
            pParent->GetLocalPoint(m_QPos);
            pParent->GetRootVect(m_QMat.XAxis());
            pParent->GetRootVect(m_QMat.YAxis());
            pParent->GetRootVect(m_QMat.ZAxis());
        }

        SetMatPosFromTetrahedron();
    }

    // ---------------------------------------------------------------------------------------------
    // Tetrahedron / matrix
    // ---------------------------------------------------------------------------------------------

    void CRigidBody::Tetrahedron(Glacier::ZVector3& v0, Glacier::ZVector3& v1, Glacier::ZVector3& v2, Glacier::ZVector3& v3)
    {
        ZASSERT(m_pBaseGeom);

        Glacier::ZVector3 size;
        m_pBaseGeom->GetSize(size);
        Glacier::vscalar(size.Get(), 0.01f);

        if (size.x <= 0.25f) size.x = 0.25f;
        if (size.y <= 0.25f) size.y = 0.25f;
        if (size.z <= 0.25f) size.z = 0.25f;

        const float sizeX = size.x;
        const float sizeY = size.y;
        const float sizeZ = size.z;

        float scaleX = sizeX;
        float scaleY = sizeY;
        float scaleZ = sizeZ;

        float ratio = sizeX / ((sizeZ + sizeY) * 0.5f);
        if (ratio < 0.33333001f)
            scaleX = sizeX / ratio * 0.33333001f;

        ratio = sizeY / ((sizeX + sizeZ) * 0.5f);
        if (ratio < 0.33333001f)
            scaleY = sizeY / ratio * 0.33333001f;

        ratio = sizeZ / ((sizeX + sizeY) * 0.5f);
        if (ratio < 0.33333001f)
            scaleZ = sizeZ / ratio * 0.33333001f;

        Glacier::vset(v0.Get(),  50.0f, -20.412415f, -28.867514f);
        Glacier::vset(v1.Get(), -50.0f, -20.412415f, -28.867514f);
        Glacier::vset(v2.Get(),   0.0f, -20.412415f,  57.735027f);
        Glacier::vset(v3.Get(),   0.0f,  61.237247f,   0.0f);

        v0.x *= scaleX; v0.y *= scaleY; v0.z *= scaleZ;
        v1.x *= scaleX; v1.y *= scaleY; v1.z *= scaleZ;
        v2.x *= scaleX; v2.y *= scaleY; v2.z *= scaleZ;
        v3.x *= scaleX; v3.y *= scaleY; v3.z *= scaleZ;
    }

    void CRigidBody::InitParticles()
    {
        Glacier::ZVector3 v0, v1, v2, v3;
        Tetrahedron(v0, v1, v2, v3);

        Glacier::vadd(v0.Get(), m_LocalCentroid.Get());
        Glacier::vadd(v1.Get(), m_LocalCentroid.Get());
        Glacier::vadd(v2.Get(), m_LocalCentroid.Get());
        Glacier::vadd(v3.Get(), m_LocalCentroid.Get());

        ZASSERT(m_pBaseGeom);
        m_pBaseGeom->GetRootPoint(v0);
        ZASSERT(m_pBaseGeom);
        m_pBaseGeom->GetRootPoint(v1);
        ZASSERT(m_pBaseGeom);
        m_pBaseGeom->GetRootPoint(v2);
        ZASSERT(m_pBaseGeom);
        m_pBaseGeom->GetRootPoint(v3);

        Glacier::ZVector3 zero;
        zero.Reset();

        m_Particles.InitParticle(1, v0, zero, 1.0f);
        m_Particles.InitParticle(2, v1, zero, 1.0f);
        m_Particles.InitParticle(3, v2, zero, 1.0f);
        m_Particles.InitParticle(4, v3, zero, 1.0f);

        m_Particles.InitConstraint(0, 4, 1);
        m_Particles.InitConstraint(1, 4, 2);
        m_Particles.InitConstraint(2, 4, 3);
        m_Particles.InitConstraint(3, 1, 2);
        m_Particles.InitConstraint(4, 1, 3);
        m_Particles.InitConstraint(5, 2, 3);

        m_Particles.SetNumConstraints(6);
    }

    void CRigidBody::SetMatPosFromTetrahedron()
    {
        Glacier::ZVector3 p1, p2, p3, p4;
        m_Particles.GetParticlePos(1, p1);
        m_Particles.GetParticlePos(2, p2);
        m_Particles.GetParticlePos(3, p3);
        m_Particles.GetParticlePos(4, p4);

        Glacier::ZVector3 zAxis;
        Glacier::vsub(zAxis.Get(), p1.Get(), p2.Get());
        Glacier::vnorm(zAxis.Get());

        Glacier::ZVector3 tmp;
        Glacier::vadd(tmp.Get(), p1.Get(), p2.Get());
        Glacier::vscalar(tmp.Get(), 0.5f);
        Glacier::vsub(tmp.Get(), p3.Get());

        Glacier::ZVector3 yAxis;
        Glacier::vcross(yAxis.Get(), zAxis.Get(), tmp.Get());
        Glacier::vnorm(yAxis.Get());

        Glacier::ZVector3 xAxis;
        Glacier::vcross(xAxis.Get(), zAxis.Get(), yAxis.Get());

        Glacier::vcpy(m_QMat.XAxis().Get(), xAxis.Get());
        Glacier::vcpy(m_QMat.YAxis().Get(), yAxis.Get());
        Glacier::vcpy(m_QMat.ZAxis().Get(), zAxis.Get());

        Glacier::ZVector3 pos;
        Glacier::vadd(pos.Get(), p1.Get(), p2.Get());
        Glacier::vadd(pos.Get(), p3.Get());
        Glacier::vadd(pos.Get(), p4.Get());
        Glacier::vscalar(pos.Get(), 0.25f);

        Glacier::ZVector3 local;
        Glacier::vmmul(local.Get(), m_LocalCentroid.Get(), m_QMat.data);

        Glacier::vsub(pos.Get(), local.Get());
        m_QPos = pos;
    }

    float CRigidBody::KineticEnergy(Glacier::ZVector3* pOutCentroid)
    {
        if (pOutCentroid)
            pOutCentroid->Reset();

        float fEnergy = 0.0f;
        for (int i = 1; i <= 4; ++i)
        {
            Glacier::ZVector3 pos, vel, oldPos;
            float mass;
            m_Particles.GetParticleValues(i, pos, vel, mass, oldPos);

            if (pOutCentroid)
                Glacier::vadd(pOutCentroid->Get(), pos.Get());

            fEnergy = Glacier::vdot(vel.Get(), vel.Get()) * mass + fEnergy;
        }

        if (pOutCentroid)
            Glacier::vscalar(pOutCentroid->Get(), 0.25f);

        return fEnergy * 0.125f;
    }

    // ---------------------------------------------------------------------------------------------
    // Enable / disable
    // ---------------------------------------------------------------------------------------------

    void CRigidBody::Enable()
    {
        if (!HasStatus(m_Status, 1))
            return;

        m_Status |= 2;
        m_nTimeOut = 1000;
        m_fWeightedSpeed = 1600.0f;
        m_fThreshold = 400.0f;
        m_iImpactNum = 3;

        ZASSERT(m_pBaseGeom);
        if ((Glacier::ZItem::m_Mask & m_pBaseGeom->GetObjectId()) == Glacier::ZItem::m_Id)
        {
            ZASSERT(m_pBaseGeom);
            static_cast<Glacier::ZItem*>(m_pBaseGeom)->OnMoving();
        }

        ZASSERT(m_pBaseGeom);
        m_pBaseGeom->MakeDynamic(true);

        ZASSERT(m_pBaseGeom);
        m_pBaseGeom->SetAutoRoomAssign(true);

        ActivateFrameUpdate(false);

        if (HasStatus(m_Status, 0x20))
        {
            ZASSERT(m_pBaseGeom);
            Glacier::ZBaseGeom* pBaseGeom = m_pBaseGeom->m_baseGeom;
            while (pBaseGeom)
            {
                pBaseGeom->SetControl(0, 2);
                ZASSERT(m_pBaseGeom);
                m_pBaseGeom->RecurGetNext(&pBaseGeom);
            }
        }

        m_iStopCount = 0;
        m_fLastNrg = 0.0f;
    }

    void CRigidBody::Disable()
    {
        if (!HasStatus(m_Status, 1))
            return;

        m_Status &= ~2u;

        ZASSERT(m_pBaseGeom);
        if ((Glacier::ZItem::m_Mask & m_pBaseGeom->GetObjectId()) == Glacier::ZItem::m_Id)
        {
            ZASSERT(m_pBaseGeom);
            static_cast<Glacier::ZItem*>(m_pBaseGeom)->OnMoved();
        }

        if (HasStatus(m_Status, 0x20))
        {
            ZASSERT(m_pBaseGeom);
            Glacier::ZBaseGeom* pBaseGeom = m_pBaseGeom->m_baseGeom;
            while (pBaseGeom)
            {
                pBaseGeom->SetControl(2, 0);
                ZASSERT(m_pBaseGeom);
                m_pBaseGeom->RecurGetNext(&pBaseGeom);
            }
        }

        DeactivateFrameUpdate();
    }

    void CRigidBody::DisableRemove(bool bRemove)
    {
        Disable();

        if (HasStatus(m_Status, 8) || bRemove)
        {
            ZASSERT(m_pBaseGeom);
            m_pBaseGeom->m_baseGeom->SetControl(33, 0);

            Remove();

            if (m_id != 0xFFFF)
            {
                ZASSERT(ZRigidBodyPoolManager::Exists());
                ZRigidBodyPoolManager::Instance().Remove(m_id);
            }
        }

        Glacier::ZBaseGeom* pHitAnything = m_pHitAnything;
        if (pHitAnything && (pHitAnything->m_lControl & 0x40000000) != 0)
        {
            Glacier::ZGROUP* pGroup = pHitAnything->ParentGroup();

            Glacier::ZMat3x3 mat;
            Glacier::ZVector3 pos;
            m_pBaseGeom->GetRootTM(mat, pos);
            m_pBaseGeom->MakeDynamic(false);
            pGroup->AttachGeom(m_pBaseGeom, true);
            m_pBaseGeom->SetRootTM(mat, pos);
        }
    }

    // ---------------------------------------------------------------------------------------------
    // Positioning
    // ---------------------------------------------------------------------------------------------

    void CRigidBody::SetPos(const Glacier::ZVector3* position)
    {
        m_Centroid = *position;
        InitParticles();
    }

    void CRigidBody::SetVelocity(const Glacier::SRigidBodyVelocity& velocity)
    {
        Glacier::ZVector3 delta;
        Glacier::vsub(delta.Get(), velocity.m_vOldPos.Get(), velocity.m_vPos.Get());
        Glacier::vadd(delta.Get(), m_Centroid.Get());

        for (int i = 1; i <= 4; ++i)
        {
            Glacier::ZVector3 particlePos;
            m_Particles.GetParticlePos(i, particlePos);
            Glacier::vsub(particlePos.Get(), m_Centroid.Get());

            Glacier::ZVector3 transformed;
            Glacier::vmtmul(transformed.Get(), particlePos.Get(), velocity.m_mMat.data);
            Glacier::TransformRootVector(transformed.Get(), velocity.m_mOldMat.data);
            Glacier::vadd(transformed.Get(), delta.Get());

            m_Particles.SetParticleOldPos(i, transformed);
        }

        Enable();
    }

    void CRigidBody::SetVelocity(const Glacier::SRigidBodyVelocity* velocity)
    {
        if (velocity)
            SetVelocity(*velocity);
    }

    void CRigidBody::SetupTransform()
    {
        m_Centroid = m_LocalCentroid;

        ZASSERT(m_pBaseGeom);
        m_pBaseGeom->GetRootPoint(m_Centroid);

        InitParticles();
    }

    // ---------------------------------------------------------------------------------------------
    // Hit / explosion
    // ---------------------------------------------------------------------------------------------

    void CRigidBody::HandleHit(Glacier::SHitInfo* hitInfo)
    {
        if (HasStatus(m_Status, 4))
            return;

        Glacier::ZVector3 direction;
        Glacier::vnorm(direction.Get(), hitInfo->pColi->ln.Get());
        Glacier::vscalar(direction.Get(), 50.0f * Glacier::g_pSysInterface->DeltaFrameTime * 128.0f);

        Impact(hitInfo->pColi->cp, direction);
        Enable();
    }

    void CRigidBody::HandleExplodeBomb(Glacier::SExplosionInfo* explosionInfo)
    {
        if (HasStatus(m_Status, 4))
            return;

        ZASSERT(m_pBaseGeom);
        Glacier::ZVector3 rootPos;
        m_pBaseGeom->GetRootPoint(rootPos);

        Glacier::ZVector3 diff;
        Glacier::vsub(diff.Get(), explosionInfo->m_vector.Get(), rootPos.Get());

        if (Glacier::vdot(diff.Get(), diff.Get()) <= 250000.0f)
        {
            m_Particles.BlowBomb(explosionInfo->m_vector, 5.0f * explosionInfo->m_velocity);
            Enable();
        }
    }

    void CRigidBody::PlaySound()
    {
        Glacier::ZSoundDllBase* pSoundDll = Glacier::g_pSysInterface->GetSoundDll();
        if (!pSoundDll || !m_pHitAnything)
            return;

        if (m_iImpactNum && (Glacier::g_pSysInterface->FrameTime.secs - m_fLastHitTime.secs) > 153)
        {
            int soundIndex = 0;
            if (m_rMaterial && m_MaterialProperty_SoundMaterial.m_Value > 0)
            {
                const uint32_t materialId = m_iColiMaterialDescId ? m_iColiMaterialDescId : 1u;
                const int soundMaterial = Glacier::BS_Runtime::ZMaterialDescriptionDB::Instance().GetAudioResourceProperty(
                    Glacier::TMaterialDescID{ static_cast<int>(materialId) }, m_MaterialProperty_SoundMaterial, Glacier::TEnumID{ 0 });

                soundIndex = static_cast<int>(pSoundDll->GetMapping(m_rMaterial, soundMaterial));
            }

            Glacier::ZMat3x3 mat;
            mat.Reset();

            Glacier::ZVector3 soundPos;
            m_pBaseGeom->GetWorldPosition(soundPos);

            Glacier::ZROOM* pRoom = m_pHitAnything->GetOwnerRoom();
            pRoom->GetRootPoint(soundPos);

            int soundEvent = 0;
            if (HasStatus(m_Status, 0x10) && m_iImpactNum == 3)
                soundEvent = m_bDoNotAddSoundEvent ? 0 : 0x11;

            pRoom->AddSound3d(mat.data, soundPos.Get(), soundIndex, soundEvent, m_pBaseGeom->GetRef(), 0);

            --m_iImpactNum;
        }

        m_fLastHitTime = Glacier::g_pSysInterface->FrameTime;
    }

    // ---------------------------------------------------------------------------------------------
    // Impact
    // ---------------------------------------------------------------------------------------------

    void CRigidBody::Impact(const Glacier::ZVector3& point, const Glacier::ZVector3& direction)
    {
        Glacier::ZVector3 particle1, particle2, particle3, particle4;
        m_Particles.GetParticlePos(1, particle1);
        m_Particles.GetParticlePos(2, particle2);
        m_Particles.GetParticlePos(3, particle3);
        m_Particles.GetParticlePos(4, particle4);

        float base[9];
        Glacier::vsub(&base[0], particle2.Get(), particle1.Get());
        Glacier::vsub(&base[3], particle3.Get(), particle1.Get());
        Glacier::vsub(&base[6], particle4.Get(), particle1.Get());

        Glacier::ZVector3 target;
        Glacier::vsub(target.Get(), point.Get(), particle1.Get());

        Glacier::ZVector3 barycentric;
        Glacier::ZCommonAlgorithms::Solve3x3System(base, target.Get(), barycentric.Get());

        const float weights[4] =
        {
            1.0f - barycentric.x - barycentric.y - barycentric.z,
            barycentric.x,
            barycentric.y,
            barycentric.z,
        };

        ImpactImpl(direction, weights, 0);
    }

    void CRigidBody::ImpactImpl(const Glacier::ZVector3& direction, const float* pWeights, int /*a4*/)
    {
        for (int i = 1; i <= 4; ++i)
        {
            Glacier::ZVector3 pos, vel, oldPos;
            float mass;
            m_Particles.GetParticleValues(i, pos, vel, mass, oldPos);

            Glacier::vaddscalar(vel.Get(), vel.Get(), direction.Get(), *pWeights);
            Glacier::vaddscalar(oldPos.Get(), oldPos.Get(), direction.Get(), -*pWeights);

            m_Particles.SetParticleValues(i, pos, vel, mass, oldPos);
            ++pWeights;
        }
    }

    // Adjusts the four tetrahedron particles against a collision plane. The masses are kept for
    // signature fidelity with the original but are unused by the implementation.
    void CRigidBody::AdjustFace2(Glacier::ZVector3& x0, Glacier::ZVector3& v0, float mass0,
                                 Glacier::ZVector3& x1, Glacier::ZVector3& v1, float mass1,
                                 Glacier::ZVector3& x2, Glacier::ZVector3& v2, float mass2,
                                 Glacier::ZVector3& x3, Glacier::ZVector3& v3, float mass3,
                                 const Glacier::ZVector3& shapePos, const Glacier::ZVector3& contactPos,
                                 float w0, float w1, float w2, float w3, float quadSum)
    {
        (void)mass0;
        (void)mass1;
        (void)mass2;
        (void)mass3;

        Glacier::ZVector3 delta;
        Glacier::vsub(delta.Get(), contactPos.Get(), shapePos.Get());

        Glacier::ZVector3 normal;
        Glacier::vnorm(normal.Get(), delta.Get());

        const float invQuadSum = 1.0f / quadSum;

        Glacier::vaddscalar(x0.Get(), x0.Get(), delta.Get(), invQuadSum * w0);
        Glacier::vaddscalar(x1.Get(), x1.Get(), delta.Get(), invQuadSum * w1);
        Glacier::vaddscalar(x2.Get(), x2.Get(), delta.Get(), invQuadSum * w2);
        Glacier::vaddscalar(x3.Get(), x3.Get(), delta.Get(), invQuadSum * w3);

        Glacier::ZVector3 velocitySum;
        Glacier::vmuls(velocitySum.Get(), v0.Get(), w0);
        Glacier::vaddscalar(velocitySum.Get(), velocitySum.Get(), v1.Get(), w1);
        Glacier::vaddscalar(velocitySum.Get(), velocitySum.Get(), v2.Get(), w2);
        Glacier::vaddscalar(velocitySum.Get(), velocitySum.Get(), v3.Get(), w3);

        const float pushOut = -Glacier::vdot(velocitySum.Get(), normal.Get());
        if (pushOut > 0.0f)
        {
            Glacier::vaddscalar(v0.Get(), v0.Get(), normal.Get(), invQuadSum * pushOut * w0);
            Glacier::vaddscalar(v1.Get(), v1.Get(), normal.Get(), invQuadSum * pushOut * w1);
            Glacier::vaddscalar(v2.Get(), v2.Get(), normal.Get(), invQuadSum * pushOut * w2);
            Glacier::vaddscalar(v3.Get(), v3.Get(), normal.Get(), invQuadSum * pushOut * w3);

            Glacier::vmuls(velocitySum.Get(), v0.Get(), w0);
            Glacier::vaddscalar(velocitySum.Get(), velocitySum.Get(), v1.Get(), w1);
            Glacier::vaddscalar(velocitySum.Get(), velocitySum.Get(), v2.Get(), w2);
            Glacier::vaddscalar(velocitySum.Get(), velocitySum.Get(), v3.Get(), w3);

            Glacier::ZVector3 tangent;
            const float friction = Glacier::vnorm(tangent.Get(), velocitySum.Get()) * -0.5f;

            Glacier::vaddscalar(v0.Get(), v0.Get(), tangent.Get(), invQuadSum * friction * w0);
            Glacier::vaddscalar(v1.Get(), v1.Get(), tangent.Get(), invQuadSum * friction * w1);
            Glacier::vaddscalar(v2.Get(), v2.Get(), tangent.Get(), invQuadSum * friction * w2);
            Glacier::vaddscalar(v3.Get(), v3.Get(), tangent.Get(), invQuadSum * friction * w3);
        }
    }

    // ---------------------------------------------------------------------------------------------
    // Collision
    // ---------------------------------------------------------------------------------------------

    void CRigidBody::CheckCollision4a(Glacier::ZCollisionBox* collisionBox)
    {
        Glacier::ZVector3 pos1, vel1, old1; float mass1;
        Glacier::ZVector3 pos2, vel2, old2; float mass2;
        Glacier::ZVector3 pos3, vel3, old3; float mass3;
        Glacier::ZVector3 pos4, vel4, old4; float mass4;

        m_Particles.GetParticleValues(1, pos1, vel1, mass1, old1);
        m_Particles.GetParticleValues(2, pos2, vel2, mass2, old2);
        m_Particles.GetParticleValues(3, pos3, vel3, mass3, old3);
        m_Particles.GetParticleValues(4, pos4, vel4, mass4, old4);

        // The ray is swept from the centroid of the previous particle positions.
        Glacier::ZVector3 centroid;
        Glacier::vadd(centroid.Get(), old1.Get(), old2.Get());
        Glacier::vadd(centroid.Get(), old3.Get());
        Glacier::vadd(centroid.Get(), old4.Get());
        Glacier::vscalar(centroid.Get(), 0.25f);

        Glacier::ZVector3 localCentroid;
        collisionBox->GetLocalPoint(localCentroid.Get(), centroid.Get());

        Glacier::SExtendedImpactInfo impact;
        impact.fPercent = 1.0f;

        for (uint32_t i = 0; i < m_nNumVertices; ++i)
        {
            const SVertexRep& rep = m_pVertexReps[i];

            Glacier::ZVector3 point;
            Glacier::vmuls(point.Get(), pos1.Get(), rep.c[0]);
            Glacier::vaddscalar(point.Get(), point.Get(), pos2.Get(), rep.c[1]);
            Glacier::vaddscalar(point.Get(), point.Get(), pos3.Get(), rep.c[2]);
            Glacier::vaddscalar(point.Get(), point.Get(), pos4.Get(), rep.c[3]);

            Glacier::ZVector3 localPoint;
            collisionBox->GetLocalPoint(localPoint.Get(), point.Get());

            Glacier::ZVector3 localDir;
            Glacier::vsub(localDir.Get(), localPoint.Get(), localCentroid.Get());

            if (!collisionBox->CalcLineCollisionLocal(&impact, localCentroid.Get(), localDir.Get(), false))
                continue;

            if (impact.fPercent < 0.0f || impact.fPercent > 1.0f)
                continue;

            if (!HasStatus(m_Status, 0x20) || impact.m_iColiMaterialDescId != static_cast<uint32_t>(g_WaterMaterialDescriptionId))
            {
                Glacier::ZVector3 edge1, edge2, normal;
                Glacier::vsub(edge1.Get(), impact.vP3.Get(), impact.vP1.Get());
                Glacier::vsub(edge2.Get(), impact.vP2.Get(), impact.vP1.Get());
                Glacier::vcross(normal.Get(), edge1.Get(), edge2.Get());

                if (Glacier::vnorm(normal.Get()) < 0.99998999f)
                    Glacier::vset(normal.Get(), 0.0f, 1.0f, 0.0f);

                collisionBox->GetRootVect(normal.Get());

                Glacier::ZVector3 rootContact;
                collisionBox->GetRootPoint(rootContact.Get(), impact.vP1.Get());

                Glacier::ZVector3 toCentre;
                Glacier::vsub(toCentre.Get(), rootContact.Get(), point.Get());

                const float dist = Glacier::vdot(normal.Get(), toCentre.Get());

                Glacier::ZVector3 projected;
                Glacier::vaddscalar(projected.Get(), point.Get(), normal.Get(), dist);

                AdjustFace2(pos1, vel1, mass1, pos2, vel2, mass2, pos3, vel3, mass3, pos4, vel4, mass4,
                            point, projected, rep.c[0], rep.c[1], rep.c[2], rep.c[3], rep.fQuadSum);
            }

            m_pHitAnything = impact.pBaseGeom;
            m_iColiMaterialDescId = impact.m_iColiMaterialDescId;
            PlaySound();
        }

        m_Particles.SetParticleValues(1, pos1, vel1);
        m_Particles.SetParticleValues(2, pos2, vel2);
        m_Particles.SetParticleValues(3, pos3, vel3);
        m_Particles.SetParticleValues(4, pos4, vel4);
    }

    void CRigidBody::CheckCollision4b(Glacier::ZCollisionBox* collisionBox)
    {
        Glacier::ZVector3 pos1, vel1, old1; float mass1;
        Glacier::ZVector3 pos2, vel2, old2; float mass2;
        Glacier::ZVector3 pos3, vel3, old3; float mass3;
        Glacier::ZVector3 pos4, vel4, old4; float mass4;

        m_Particles.GetParticleValues(1, pos1, vel1, mass1, old1);
        m_Particles.GetParticleValues(2, pos2, vel2, mass2, old2);
        m_Particles.GetParticleValues(3, pos3, vel3, mass3, old3);
        m_Particles.GetParticleValues(4, pos4, vel4, mass4, old4);

        Glacier::ZVector3 centroid;
        Glacier::vadd(centroid.Get(), pos1.Get(), pos2.Get());
        Glacier::vadd(centroid.Get(), pos3.Get());
        Glacier::vadd(centroid.Get(), pos4.Get());
        Glacier::vscalar(centroid.Get(), 0.25f);

        Glacier::ZVector3 localCentroid;
        collisionBox->GetLocalPoint(localCentroid.Get(), centroid.Get());

        Glacier::SExtendedImpactInfo impact;
        impact.fPercent = 1.0f;

        Glacier::ZVector3 localBodyPos;
        collisionBox->GetLocalPoint(localBodyPos.Get(), m_Centroid.Get());

        Glacier::ZVector3 localDir;
        Glacier::vsub(localDir.Get(), localCentroid.Get(), localBodyPos.Get());

        if (!collisionBox->CalcLineCollisionLocal(&impact, localBodyPos.Get(), localDir.Get(), false))
        {
            m_Centroid = centroid;
            return;
        }

        if (impact.fPercent < 0.0f || impact.fPercent > 1.0f)
            return;

        if (!HasStatus(m_Status, 0x20) || impact.m_iColiMaterialDescId != static_cast<uint32_t>(g_WaterMaterialDescriptionId))
        {
            Glacier::ZVector3 shift;
            Glacier::vsub(shift.Get(), m_Centroid.Get(), centroid.Get());

            Glacier::vadd(pos1.Get(), shift.Get());
            Glacier::vadd(pos2.Get(), shift.Get());
            Glacier::vadd(pos3.Get(), shift.Get());
            Glacier::vadd(pos4.Get(), shift.Get());

            m_Particles.SetParticleValues(1, pos1, vel1);
            m_Particles.SetParticleValues(2, pos2, vel2);
            m_Particles.SetParticleValues(3, pos3, vel3);
            m_Particles.SetParticleValues(4, pos4, vel4);
        }

        m_pHitAnything = impact.pBaseGeom;
        m_iColiMaterialDescId = impact.m_iColiMaterialDescId;
        PlaySound();
    }

    // ---------------------------------------------------------------------------------------------
    // Simulation
    // ---------------------------------------------------------------------------------------------

    float CRigidBody::Move(float fTimeStep)
    {
        if (!HasStatus(m_Status, 1) || !HasStatus(m_Status, 2))
            return 0.0f;

        float fElevatorDeltaY = 0.0f;
        if (m_rContainingElevator)
            fElevatorDeltaY = GetElevatorDeltaY();

        if (fabs(fElevatorDeltaY) > 0.0f)
        {
            for (int i = 1; i <= 4; ++i)
            {
                Glacier::ZVector3 p;

                m_Particles.GetParticlePos(i, p);
                p.z += fElevatorDeltaY;
                m_Particles.SetParticlePos(i, p);

                m_Particles.GetParticleOKPos(i, p);
                p.z += fElevatorDeltaY;
                m_Particles.SetParticleOKPos(i, p);

                m_Particles.GetParticleOldPos(i, p);
                p.z += fElevatorDeltaY;
                m_Particles.SetParticleOldPos(i, p);

                if (fElevatorDeltaY > 0.0f)
                    m_Centroid.y += fElevatorDeltaY;
            }
        }

        if (m_nTimeOut)
        {
            --m_nTimeOut;
            if (!m_nTimeOut)
            {
                m_Status &= ~2u;
                return 0.0f;
            }
        }

        ZASSERT(m_pBaseGeom);
        m_pBaseGeom->GetRootTM(m_QMat, m_QPos);

        m_Particles.MoveRigidBody(fTimeStep);

        Glacier::ZVector3 pos1, vel1, old1; float mass1;
        Glacier::ZVector3 pos2, vel2, old2; float mass2;
        Glacier::ZVector3 pos3, vel3, old3; float mass3;
        Glacier::ZVector3 pos4, vel4, old4; float mass4;
        m_Particles.GetParticleValues(1, pos1, vel1, mass1, old1);
        m_Particles.GetParticleValues(2, pos2, vel2, mass2, old2);
        m_Particles.GetParticleValues(3, pos3, vel3, mass3, old3);
        m_Particles.GetParticleValues(4, pos4, vel4, mass4, old4);

        if (pos1.z < -10000.0f)
        {
            m_Status &= ~2u;
            return 0.0f;
        }

        ZASSERT(m_nNumVertices);

        // Bounding box around all vertex reps (current + old positions).
        Glacier::ZVector3 vMin, vMax;
        Glacier::vset(vMin.Get(), 1.0e10f, 1.0e10f, 1.0e10f);
        Glacier::vset(vMax.Get(), -1.0e10f, -1.0e10f, -1.0e10f);

        for (uint32_t i = 0; i < m_nNumVertices; ++i)
        {
            const SVertexRep& rep = m_pVertexReps[i];

            Glacier::ZVector3 p, o;
            Glacier::vmuls(p.Get(), pos1.Get(), rep.c[0]);
            Glacier::vaddscalar(p.Get(), p.Get(), pos2.Get(), rep.c[1]);
            Glacier::vaddscalar(p.Get(), p.Get(), pos3.Get(), rep.c[2]);
            Glacier::vaddscalar(p.Get(), p.Get(), pos4.Get(), rep.c[3]);

            Glacier::vmuls(o.Get(), old1.Get(), rep.c[0]);
            Glacier::vaddscalar(o.Get(), o.Get(), old2.Get(), rep.c[1]);
            Glacier::vaddscalar(o.Get(), o.Get(), old3.Get(), rep.c[2]);
            Glacier::vaddscalar(o.Get(), o.Get(), old4.Get(), rep.c[3]);

            Glacier::vmin(vMin.Get(), p.Get());
            Glacier::vmax(vMax.Get(), p.Get());
            Glacier::vmin(vMin.Get(), o.Get());
            Glacier::vmax(vMax.Get(), o.Get());
        }

        Glacier::ZVector3 vCen, vSize;
        Glacier::vadd(vCen.Get(), vMax.Get(), vMin.Get());
        Glacier::vscalar(vCen.Get(), 0.5f);

        Glacier::vsub(vSize.Get(), vMax.Get(), vMin.Get());
        Glacier::vscalar(vSize.Get(), 0.6f);

        m_pHitAnything = nullptr;

        char aMemBuffer[0x10000];
        Glacier::ZCollisionBox* pCollisionBox = Glacier::ZCollisionBase::s_pCollisionBase->LockCollisionBox(aMemBuffer, 0x10000);

        Glacier::ZMat3x3 mat;
        mat.Reset();
        pCollisionBox->SetBox(mat.data, vSize.Get(), vCen.Get());
        pCollisionBox->GetStrips(HasStatus(m_Status, 0x20) ? 2 : 32);

        for (int iter = 0; iter < 4; ++iter)
        {
            m_Particles.ProjectConstraints2(4);

            for (int j = 1; j <= 4; ++j)
            {
                Glacier::ZVector3 pos, vel, oldPos;
                float mass;
                m_Particles.GetParticleValues(j, pos, vel, mass, oldPos);
                Glacier::vsub(vel.Get(), pos.Get(), oldPos.Get());
                m_Particles.SetParticleVel(j, vel);
            }

            CheckCollision4a(pCollisionBox);

            for (int k = 1; k <= 4; ++k)
            {
                Glacier::ZVector3 pos, vel, oldPos;
                float mass;
                m_Particles.GetParticleValues(k, pos, vel, mass, oldPos);
                Glacier::vsub(oldPos.Get(), pos.Get(), vel.Get());
                m_Particles.SetParticleValues(k, pos, vel, mass, oldPos);
            }

            m_Particles.ProjectConstraints2(4);
        }

        CheckCollision4b(pCollisionBox);

        Glacier::ZCollisionBase::s_pCollisionBase->UnlockCollisionBox(pCollisionBox);

        ZASSERT(m_pBaseGeom);
        Glacier::ZMat3x3 rootMat;
        Glacier::ZVector3 rootPos;
        m_pBaseGeom->GetRootTM(rootMat, rootPos);

        SetMatPosFromTetrahedron();

        ZASSERT(m_pBaseGeom);
        m_pBaseGeom->SetRootTM(m_QMat, m_QPos);

        m_OldPos = rootPos;

        Glacier::ZVector3 curCen;
        Glacier::vadd(curCen.Get(), pos1.Get(), pos2.Get());
        Glacier::vadd(curCen.Get(), pos3.Get());
        Glacier::vadd(curCen.Get(), pos4.Get());
        Glacier::vscalar(curCen.Get(), 0.25f);

        Glacier::ZVector3 oldCen;
        Glacier::vadd(oldCen.Get(), old1.Get(), old2.Get());
        Glacier::vadd(oldCen.Get(), old3.Get());
        Glacier::vadd(oldCen.Get(), old4.Get());
        Glacier::vscalar(oldCen.Get(), 0.25f);

        const float fSpeed = Glacier::vdist(curCen.Get(), oldCen.Get()) / (fTimeStep * 50.0f * fTimeStep);
        const float fDamping = 0.02f / fTimeStep;

        for (int i = 1; i <= 4; ++i)
        {
            Glacier::ZVector3 pos, vel, oldPos;
            float mass;
            m_Particles.GetParticleValues(i, pos, vel, mass, oldPos);

            Glacier::ZVector3 v;
            Glacier::vsub(v.Get(), pos.Get(), oldPos.Get());
            Glacier::vscalar(v.Get(), fDamping);
            m_Particles.SetParticleVel(i, v);
        }

        return fSpeed;
    }

    void CRigidBody::FrameUpdate()
    {
        if (!HasStatus(m_Status, 1) || !HasStatus(m_Status, 2))
            return;

        UpdateElevatorState();

        ZASSERT(m_nNumVertices);

        const float fFrameTime = Glacier::g_pSysInterface->DeltaFrameTime;
        float fRemaining = fFrameTime > 0.05f ? 0.05f : fFrameTime;
        float fSpeed = 0.0f;

        while (fRemaining > 0.0f)
        {
            if (0.05f >= fRemaining)
            {
                fSpeed = Move(fRemaining);
                fRemaining = 0.0f;
                break;
            }

            fSpeed = Move(0.05f);
            fRemaining -= 0.05f;
            if (fRemaining < 0.025f)
                break;
        }

        float fEnergy = 0.0f;
        for (int i = 1; i <= 4; ++i)
        {
            Glacier::ZVector3 pos, vel, oldPos;
            float mass;
            m_Particles.GetParticleValues(i, pos, vel, mass, oldPos);
            fEnergy = Glacier::vdot(vel.Get(), vel.Get()) * mass + fEnergy;
        }

        fEnergy *= 0.125f;

        m_fWeightedSpeed = m_fWeightedSpeed * 0.92500001f + fSpeed;

        if (fabs(m_fLastNrg - fEnergy) >= 0.05f || fEnergy >= 2.0f)
        {
            m_iStopCount = 0;
        }
        else if (++m_iStopCount > 0x10u)
        {
            m_Status &= ~2u;
        }

        m_fLastNrg = fEnergy;

        if (!HasStatus(m_Status, 1) || !HasStatus(m_Status, 2) || m_fWeightedSpeed < m_fThreshold)
            DisableRemove(false);

        m_fThreshold *= static_cast<float>(pow(1.001999974250793, fFrameTime * 20.0));

        if (m_pHitAnything)
        {
            Glacier::COLI coli;

            // The contact is pushed into the Glacier::COLI: its centroid (lp), the body's Q-space movement
            // (ln = m_QPos - m_OldPos), the Q-space position (cp) and the kinetic energy (t).
            Glacier::ZVector3 vCentroid;
            coli.t = KineticEnergy(&vCentroid);
            coli.lp = vCentroid;
            coli.ln = m_QPos - m_OldPos;
            coli.cp = m_QPos;
            coli.m_iColiMaterialDescId = m_iColiMaterialDescId;
            coli.ColiRef = Glacier::ZGeomBuffer::m_Instance->GeomPtrToRef(m_pHitAnything);

            ZASSERT(m_pHitAnything);
            m_pHitAnything->SendCommand(m_msgCollision, &coli, nullptr);

            ZASSERT(m_pBaseGeom);
            m_pBaseGeom->SendCommand(m_msgCollision, &coli, nullptr);
        }
    }

    // ---------------------------------------------------------------------------------------------
    // Elevator
    // ---------------------------------------------------------------------------------------------

    void CRigidBody::UpdateElevatorState()
    {
        ZASSERT(m_pBaseGeom);
        const Glacier::ZREF rRef = m_pBaseGeom->GetRef();

        Hitman::ZHM3GameData* pGameData = static_cast<Hitman::ZHM3GameData*>(Glacier::g_pGameData);
        const Glacier::ZREF rClosest = pGameData ? pGameData->FindClosestElevator(rRef, 1000.0f) : 0;
        if (!rClosest)
        {
            m_rContainingElevator = 0;
            return;
        }

        Glacier::ZGEOM* pElevator = Glacier::ZGEOM::RefToPtr(rClosest);
        if (!pElevator)
        {
            m_rContainingElevator = 0;
            return;
        }

        Glacier::ZVector3 vSize;
        Glacier::ZMat3x3 mMat;
        Glacier::ZVector3 vPos;
        pElevator->GetSize(vSize);
        pElevator->GetRootTM(mMat, vPos);

        // Ask the elevator for its hatch geometry (extends the vertical extent).
        int32_t lHatch = 0;
        ZASSERT(m_msgGetHatchGeom);
        pElevator->SendCommand(m_msgGetHatchGeom, &lHatch, nullptr);

        vPos.y += vSize.y;
        if (lHatch)
            vSize.y += 100.0f;

        if (m_rContainingElevator && fabs(GetElevatorDeltaY()) > 0.0f)
        {
            vSize.x += 20.0f;
            vSize.z += 20.0f;
        }

        ZASSERT(m_pBaseGeom);
        Glacier::ZMat3x3 mBodyMat;
        Glacier::ZVector3 vBodyPos;
        m_pBaseGeom->GetRootTM(mBodyMat, vBodyPos);

        Glacier::ZVector3 local = vBodyPos - vPos;
        Glacier::vmtmul(local.Get(), mMat.data);

        if (fabs(local.x) > vSize.x || fabs(local.y) > vSize.y || fabs(local.z) > vSize.z)
        {
            if (m_rContainingElevator)
                DetachFromElevator();

            m_rContainingElevator = 0;
        }
        else
        {
            if (!m_rContainingElevator)
                AttachToElevator(rClosest);

            m_rContainingElevator = rClosest;
        }
    }

    float CRigidBody::GetElevatorDeltaY()
    {
        float fDeltaY = 0.0f;
        if (m_rContainingElevator)
        {
            Glacier::ZGEOM* pElevator = Glacier::ZGEOM::RefToPtr(m_rContainingElevator);
            if (pElevator)
            {
                ZASSERT(m_msgRequestDeltaY);
                m_pBaseGeom->SendCommand(pElevator, m_msgRequestDeltaY, &fDeltaY);
            }
        }
        return fDeltaY;
    }

    void CRigidBody::AttachToElevator(uint32_t rElevator)
    {
        ZASSERT(m_pBaseGeom);
        Glacier::ZMat3x3 mat;
        Glacier::ZVector3 pos;
        m_pBaseGeom->GetRootTM(mat, pos);

        Glacier::ZGEOM* pElevator = Glacier::ZGEOM::RefToPtr(rElevator);
        if (pElevator)
        {
            m_pBaseGeom->MakeDynamic(false);
            static_cast<Glacier::ZGROUP*>(pElevator)->AttachGeom(m_pBaseGeom, true);
        }

        m_pBaseGeom->SetRootTM(mat, pos);
        m_rContainingElevator = rElevator;
    }

    void CRigidBody::DetachFromElevator()
    {
        ZASSERT(m_pBaseGeom);
        Glacier::ZMat3x3 mat;
        Glacier::ZVector3 pos;
        m_pBaseGeom->GetRootTM(mat, pos);

        Glacier::ZGROUP* pParent = m_pBaseGeom->Parent();
        if (pParent)
            pParent->DetachGeom(m_pBaseGeom->m_baseGeom, false);

        if (Glacier::ZROOT)
            Glacier::ZROOT->AttachGeom(m_pBaseGeom, true);

        m_pBaseGeom->MakeDynamic(true);
        m_pBaseGeom->SetAutoRoomAssign(true);
        m_pBaseGeom->SetRootTM(mat, pos);

        m_rContainingElevator = 0;
    }
}
