#include <BloodMoney/Game/Items/ZHM3ItemWeapon.h>

#include <Glacier/ZSTL/REFTAB.h>
#include <BloodMoney/Game/Items/ZHM3ItemTemplateWeapon.h>
#include <BloodMoney/Game/Items/ZHM3ItemTool.h>
#include <Glacier/Animation/ActiveAnimation.h>
#include <Glacier/Animation/Header.h>
#include <Glacier/Animation/Manager.h>
#include <Glacier/IK/ZBoneModifyBase.h>
#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/ZUniAssert.h>

namespace Hitman
{
    // PC 0x650260. Sets the recoil defaults and clears the runtime animation/geometry pointers.
    ZHM3ItemWeapon::ZHM3ItemWeapon(const char* psName, Glacier::ZBaseGeom* pBaseGeom)
        : Glacier::ZItemWeapon(psName, pBaseGeom)
        , m_fRecoilDamping(0.8f)
        , m_fRecoilShotImpact(0.1f)
        , m_pAnimReload(nullptr)
        , m_pAnimFire(nullptr)
        , m_pAnimChamber(nullptr)
        , m_pAnimUse(nullptr)
        , m_pGround(nullptr)
        , m_pBoneAnim(nullptr)
    {
    }

    // vtbl slot 12 (RTTI)  PC 0x64A080
    const Glacier::RTP::ZPropertyInfo& ZHM3ItemWeapon::GetProperties() const
    {
        return ZHM3ItemWeapon::Info;
    }

    // vtbl slot 13 (RTTI)  PC 0x64B310
    uint32_t ZHM3ItemWeapon::GetObjectId() const
    {
        return ZHM3ItemWeapon::m_Id;
    }

    // vtbl slot 14 (RTTI)  PC 0x64B320
    void ZHM3ItemWeapon::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZHM3ItemWeapon::m_Id;
        mask = ZHM3ItemWeapon::m_Mask;
    }

    // vtbl slot 15 (RTTI)  PC 0x64A090 -> &ZHM3ItemWeapon::m_OldClassInfo
    Glacier::ZGEOMCLASSINFO* ZHM3ItemWeapon::GetOldClassInfo() const
    {
        return ZHM3ItemWeapon::m_OldClassInfo;
    }

    // vtbl slot 84  PC 0x64DA90
    void ZHM3ItemWeapon::PostClassInit()
    {
        Glacier::ZItem::PostClassInit();

        m_pGround = static_cast<Glacier::ZLNKOBJ*>(FindGeom("Ground*", nullptr));
        if (m_pGround == nullptr)
            return;

        m_pAnimReload = m_pGround->GetAnimHeaderFromHandleName("/Movement/Reload");
        m_pAnimFire = m_pGround->GetAnimHeaderFromHandleName("/Movement/Fire");
        m_pAnimChamber = m_pGround->GetAnimHeaderFromHandleName("/Movement/swap");
        m_pAnimUse = m_pGround->GetAnimHeaderFromHandleName("/Movement/Use");

        // The PC selects the bullet bone "w_bone_bullet_01" for the revolver family
        // (HM3 type 17/18/19, six-shooter) and the clip bone "w_bone_clip" otherwise.
        const EHM3ItemType eHM3Type = ZHM3ItemTool::GetHM3Type(this);
        if (eHM3Type == EHM3ItemType::eHM3Gun_SixShooter_01 ||
            eHM3Type == EHM3ItemType::eHM3Gun_StubNosed_01 ||
            eHM3Type == EHM3ItemType::eHM3Gun_Taurus_01)
        {
            m_bSixShooter = true;
            m_iClipBoneNr = m_pGround->GetBoneNrFromName("w_bone_bullet_01");
        }
        else
        {
            m_bSixShooter = false;
            m_iClipBoneNr = m_pGround->GetBoneNrFromName("w_bone_clip");
        }
    }

    // vtbl slot 206  PC 0x64BAF0
    EHM3ItemType ZHM3ItemWeapon::GetHM3ItemType()
    {
        Glacier::ZItemTemplate* pTemplate = GetItemTemplate();
        ZASSERT(pTemplate != nullptr);
        ZASSERT((pTemplate->GetObjectId() & ZHM3ItemTemplateWeapon::m_Mask) == ZHM3ItemTemplateWeapon::m_Id);

        return static_cast<ZHM3ItemTemplateWeapon*>(pTemplate)->GetHM3ItemType();
    }

    // vtbl slot 207  PC 0x64A2D0
    void ZHM3ItemWeapon::ChamberActivateAnimation()
    {
        if (m_pAnimChamber == nullptr)
            return;

        ZASSERT(m_pGround != nullptr);
        if (m_pGround != nullptr)
            m_pGround->ActivateAnim(m_pAnimChamber, 1);
    }

    // vtbl slot 208  PC 0x64A2F0
    void ZHM3ItemWeapon::UseItemActivateAnimation()
    {
        if (m_pGround == nullptr || m_pAnimUse == nullptr)
            return;

        m_pGround->ActivateAnim(m_pAnimUse, 1);
    }

    // vtbl slot 209  PC 0x64BC40
    const char* ZHM3ItemWeapon::GetAnimNameActorReload()
    {
        Glacier::ZItemTemplate* pTemplate = GetItemTemplate();
        ZASSERT(pTemplate != nullptr);
        ZASSERT((pTemplate->GetObjectId() & ZHM3ItemTemplateWeapon::m_Mask) == ZHM3ItemTemplateWeapon::m_Id);

        // The PC build forwards to the template getter with no argument.
        return static_cast<ZHM3ItemTemplateWeapon*>(pTemplate)->GetAnimNameActorReload(0);
    }

    // vtbl slot 210  PC 0x64BC90
    const char* ZHM3ItemWeapon::GetAnimNameActorChamber()
    {
        Glacier::ZItemTemplate* pTemplate = GetItemTemplate();
        ZASSERT(pTemplate != nullptr);
        ZASSERT((pTemplate->GetObjectId() & ZHM3ItemTemplateWeapon::m_Mask) == ZHM3ItemTemplateWeapon::m_Id);

        return static_cast<ZHM3ItemTemplateWeapon*>(pTemplate)->GetAnimNameActorChamber();
    }

    // vtbl slot 211  PC 0x64A0A0
    int ZHM3ItemWeapon::GetClipBoneNr()
    {
        return m_iClipBoneNr;
    }

    // vtbl slot 212  PC 0x64BB60
    void ZHM3ItemWeapon::InitializeMetaKeyEvents(Glacier::Animation::Header* pAnim)
    {
        if (m_pGround == nullptr || pAnim == nullptr)
            return;

        Glacier::Animation::Manager* pManager = Glacier::Animation::instance;
        if (pManager == nullptr)
            return;

        const Glacier::Animation::ZMetaKey* pMetaKeys = pManager->GetMetaKeyData(pAnim->m_MetaDataOffset);
        const uint32_t lMetaKeyCount = pManager->GetMetaKeyDataLength(pAnim->m_MetaDataOffset);

        for (uint32_t i = 0; i < lMetaKeyCount; ++i)
        {
            const Glacier::Animation::ZMetaKey& rKey = pMetaKeys[i];
            if (rKey.lValue < 0xB || rKey.lValue > 0xD)
                continue;

            if (m_MetaKeyEvents.Count() > 4)
                return;

            const int32_t lValue = static_cast<int32_t>(rKey.lValue);
            m_MetaKeyEvents.Add(&lValue, static_cast<float>(rKey.lFrame));
        }
    }

    // vtbl slot 213  PC 0x64DBB0
    void ZHM3ItemWeapon::UpdateMetaKeyEvents()
    {
        if (m_pBoneAnim == nullptr)
            return;

        while (m_MetaKeyEvents.Count() > 0)
        {
            // Wait until the playing bone animation has reached the earliest queued event.
            if (m_pBoneAnim->frame < m_MetaKeyEvents.GetSortValue(m_MetaKeyEvents.Count() - 1))
                break;

            const int32_t lEvent = m_MetaKeyEvents.Pop();
            switch (lEvent)
            {
            case 11: // Hide the clip / bullets.
                m_pGround->HideBone(static_cast<uint8_t>(m_iClipBoneNr), true);
                if (m_bSixShooter)
                {
                    for (int i = 2; i < 7; ++i)
                    {
                        char szBoneName[32];
                        Glacier::g_pSysInterface->BeforeFormat()->SPrintF(szBoneName, "w_bone_bullet_0%1d", i);
                        const uint8_t lBoneNr = static_cast<uint8_t>(m_pGround->GetBoneNrFromName(szBoneName));
                        m_pGround->HideBone(lBoneNr, true);
                    }
                }
                break;
            case 12: // Show the clip / bullets.
                m_pGround->HideBone(static_cast<uint8_t>(m_iClipBoneNr), false);
                if (m_bSixShooter)
                {
                    for (int i = 2; i < 7; ++i)
                    {
                        char szBoneName[32];
                        Glacier::g_pSysInterface->BeforeFormat()->SPrintF(szBoneName, "w_bone_bullet_0%1d", i);
                        const uint8_t lBoneNr = static_cast<uint8_t>(m_pGround->GetBoneNrFromName(szBoneName));
                        m_pGround->HideBone(lBoneNr, false);
                    }
                }
                break;
            case 13:
            {
                // Shell/clip spawn. The template asserts a ZHM3ItemTemplateWeapon and reads its
                // clip particle control; each round (six for a six-shooter) launches one casing
                // from its own bullet bone. Every shell after the first reuses the first shell's
                // firing direction.
                Glacier::ZItemTemplate* pBaseTemplate = GetItemTemplate();
                ZASSERT(pBaseTemplate != nullptr);
                ZASSERT((ZHM3ItemTemplateWeapon::m_Mask & pBaseTemplate->GetObjectId()) == ZHM3ItemTemplateWeapon::m_Id);

                auto* pWeaponTemplate = static_cast<ZHM3ItemTemplateWeapon*>(pBaseTemplate);
                if (pWeaponTemplate->GetClipParticleControl(true) != nullptr)
                {
                    Glacier::ZBoneModifyBase* pBoneModifier = m_pGround->GetBoneModifier();
                    ZASSERT(pBoneModifier != nullptr);

                    const int lShells = m_bSixShooter ? 6 : 1;
                    Glacier::ZVector3 vSavedDir;
                    for (int k = 0; k < lShells; ++k)
                    {
                        uint8_t lBoneNr;
                        if (m_bSixShooter && k != 0)
                        {
                            char szBoneName[32];
                            Glacier::g_pSysInterface->BeforeFormat()->SPrintF(szBoneName, "w_bone_bullet_0%1d", k + 1);
                            lBoneNr = static_cast<uint8_t>(m_pGround->GetBoneNrFromName(szBoneName));
                        }
                        else
                        {
                            lBoneNr = static_cast<uint8_t>(m_iClipBoneNr);
                        }

                        Glacier::ZMat3x3 matBone;
                        Glacier::ZVector3 vBonePos;
                        pBoneModifier->GetIKBoneMatPos(matBone, vBonePos, lBoneNr, m_pGround, nullptr);

                        Glacier::ZMat3x3 matGround;
                        Glacier::ZVector3 vGroundPos;
                        m_pGround->GetMatPos(matGround, vGroundPos);
                        Glacier::mmmul(matBone.data, matGround.data);
                        Glacier::TransformRootVector(vBonePos, matGround);
                        vBonePos += vGroundPos;

                        Glacier::ZMat3x3 matRoot;
                        Glacier::ZVector3 vRootPos;
                        GetMainItemRootTM(reinterpret_cast<float*>(&matRoot), reinterpret_cast<float*>(&vRootPos));
                        Glacier::mmmul(matBone.data, matRoot.data);
                        Glacier::TransformRootVector(vBonePos, matRoot);
                        vBonePos += vRootPos;

                        // Direction from the stored clip position towards the bullet bone.
                        Glacier::ZVector3 vDir = m_vClipPosOld;
                        Glacier::TransformRootVector(vDir, matRoot);
                        vDir += vRootPos;
                        vDir -= vBonePos;
                        Glacier::vnorm(vDir.Get());

                        if (m_bSixShooter && k == 0)
                            vSavedDir = vDir;

                        auto* pClipControl = static_cast<ZHM3ClipParticleControl*>(pWeaponTemplate->GetClipParticleControl(true));
                        pClipControl->SpawnClipAtMatPosSpeed(matBone, vBonePos,
                            (m_bSixShooter && k != 0) ? vSavedDir : vDir, this, m_bSixShooter);
                    }
                }

                m_pGround->HideBone(static_cast<uint8_t>(m_iClipBoneNr), true);
                break;
            }
            default:
                break;
            }
        }

        // The PC rebuilds m_vClipPosOld through ZBoneModifyBase::GetIKBoneMatPos +
        // ZGEOM::GetMatPos + TransformRootVector.
        if (m_MetaKeyEvents.Count() > 1)
        {
            Glacier::ZBoneModifyBase* pBoneModifier = m_pGround->GetBoneModifier();
            ZASSERT(pBoneModifier != nullptr);

            Glacier::ZMat3x3 matBone;
            Glacier::ZMat3x3 mat;
            Glacier::ZVector3 vPos;
            pBoneModifier->GetIKBoneMatPos(matBone, m_vClipPosOld, static_cast<uint8_t>(m_iClipBoneNr), m_pGround, nullptr);
            m_pGround->GetMatPos(mat, vPos);
            TransformRootVector(m_vClipPosOld, mat);
            m_vClipPosOld += vPos;
        }
    }

    // vtbl slot 214  PC 0x64A1C0
    void ZHM3ItemWeapon::UpdateReloadShellAnim()
    {
        UpdateReloadShellAnimMetaKeys();
    }

    // vtbl slot 215  PC 0x64A1D0
    void ZHM3ItemWeapon::UpdateReloadShellAnimMetaKeys()
    {
        if (m_pBoneAnim == nullptr)
            return;

        const int32_t lFrame = static_cast<int32_t>(m_pBoneAnim->frame);
        if (lFrame == m_nCurFrame)
            return;

        m_nCurFrame = lFrame;

        if (m_pAnimReload == nullptr || m_pGround == nullptr)
            return;

        Glacier::Animation::Manager* pManager = Glacier::Animation::instance;
        if (pManager == nullptr)
            return;

        const Glacier::Animation::ZMetaKey* pMetaKeys = pManager->GetMetaKeyData(m_pAnimReload->m_MetaDataOffset);
        const uint32_t lMetaKeyCount = pManager->GetMetaKeyDataLength(m_pAnimReload->m_MetaDataOffset);

        for (uint32_t i = 0; i < lMetaKeyCount; ++i)
        {
            const Glacier::Animation::ZMetaKey& rKey = pMetaKeys[i];
            if (static_cast<int32_t>(rKey.lFrame) != lFrame)
                continue;

            const int32_t lBoneNr = m_pGround->GetBoneNrFromName("w_bone_shell");
            if (lBoneNr == 0)
                return;

            if (rKey.lValue == 11)
            {
                m_pGround->HideBone(static_cast<uint8_t>(lBoneNr), true);
            }
            else if (rKey.lValue == 12)
            {
                m_pGround->HideBone(static_cast<uint8_t>(lBoneNr), false);
            }
        }
    }

    // vtbl slot 216  PC 0x64E600
    void ZHM3ItemWeapon::FireAnimCallback(Glacier::Animation::ActiveAnimation* pAnim, float frame, float deltaFrame, unsigned int lBoneNr)
    {
        if (lBoneNr == 0)
            return;

        ZASSERT(m_pGround != nullptr);
        Glacier::ZBoneModifyBase* pBoneModifier = m_pGround->GetBoneModifier();
        ZASSERT(pBoneModifier != nullptr);

        // First call after the bullet bone was reset: seed m_vClipPosOld from the bone matrix.
        if (m_vClipPosOld.x == 0.0f && m_vClipPosOld.y == 0.0f && m_vClipPosOld.z == 0.0f)
        {
            Glacier::ZMat3x3 matBone;
            Glacier::ZVector3 vPos;
            pBoneModifier->GetIKBoneMatPos(matBone, vPos, static_cast<uint8_t>(lBoneNr), m_pGround, nullptr);

            Glacier::ZMat3x3 mat;
            Glacier::ZVector3 vGroundPos;
            m_pGround->GetMatPos(mat, vGroundPos);
            Glacier::mmmul(matBone.data, mat.data);
            TransformRootVector(vPos, mat);
            vPos += vGroundPos;
            m_vClipPosOld = vPos;
            return;
        }

        ZHM3ItemTemplateWeapon* pTemplate = ZHM3ItemTool::GetHM3ItemTemplateWeapon(this);
        ZASSERT(pTemplate != nullptr);
        if (pTemplate->GetClipParticleControl(false) != nullptr)
        {
            // Far end of the bullet bone: derive the casing matrix/position from the bone and
            // launch it from the stored clip position (m_vClipPosOld).
            Glacier::ZMat3x3 matBone;
            Glacier::ZVector3 vBonePos;
            pBoneModifier->GetIKBoneMatPos(matBone, vBonePos, static_cast<uint8_t>(lBoneNr), m_pGround, nullptr);

            Glacier::ZMat3x3 matGround;
            Glacier::ZVector3 vGroundPos;
            m_pGround->GetMatPos(matGround, vGroundPos);
            Glacier::mmmul(matBone.data, matGround.data);
            Glacier::TransformRootVector(vBonePos, matGround);
            vBonePos += vGroundPos;

            Glacier::ZMat3x3 matRoot;
            Glacier::ZVector3 vRootPos;
            GetMainItemRootTM(reinterpret_cast<float*>(&matRoot), reinterpret_cast<float*>(&vRootPos));
            Glacier::mmmul(matBone.data, matRoot.data);
            Glacier::TransformRootVector(vBonePos, matRoot);
            vBonePos += vRootPos;

            Glacier::ZVector3 vDir = m_vClipPosOld;
            Glacier::TransformRootVector(vDir, matRoot);
            vDir += vRootPos;
            vDir -= vBonePos;
            Glacier::vnorm(vDir.Get());

            auto* pClipControl = static_cast<ZHM3ClipParticleControl*>(pTemplate->GetClipParticleControl(false));
            pClipControl->SpawnClipAtMatPosSpeed(matBone, vBonePos, vDir, this, false);
        }

        m_pGround->HideBone(static_cast<uint8_t>(lBoneNr), true);

        (void)pAnim;
        (void)frame;
        (void)deltaFrame;
    }

    // vtbl slot 217  PC 0x580400
    bool ZHM3ItemWeapon::IsDetectable()
    {
        return true;
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        // The PC chain is laid out tail-first; the head (m_fRecoilDamping) is the FirstProperty
        // passed to DECLARE_GEOM_CLASS_IMPL. Game offset -> class offset = +4
        // (ZHM3ItemWeapon::Info.First is 0x80FA10). The PC m_Name pointers are all null, so the
        // names below come from the PS2 records (0x94C560..), which share the same field order for
        // this class. m_bSixShooter has no PS2 counterpart and keeps its project member name.
        static Glacier::RTP::ZDataProperty<uint32_t> CurrentSound{
            .m_Node = {.m_Next = nullptr, .m_Name = "m_rCurrentSound", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_uint,
            .m_Offset = reinterpret_cast<uint32_t*>(CLASS_PROPERTY(ZHM3ItemWeapon, m_rCurrentSound))};

        static Glacier::RTP::ZDataProperty<bool> SixShooter{
            .m_Node = {.m_Next = CurrentSound, .m_Name = "m_bSixShooter", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_bool,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemWeapon, m_bSixShooter)};

        static Glacier::RTP::ZDataProperty<bool> BeingReloaded{
            .m_Node = {.m_Next = SixShooter, .m_Name = "m_bBeingReloaded", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_bool,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemWeapon, m_bBeingReloaded)};

        static Glacier::RTP::ZDataProperty<bool> DualWeapon{
            .m_Node = {.m_Next = BeingReloaded, .m_Name = "m_bDualWeapon", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_bool,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemWeapon, m_bDualWeapon)};

        static Glacier::RTP::ZDataProperty<float[3]> ClipPosOld{
            .m_Node = {.m_Next = DualWeapon, .m_Name = "m_vClipPosOld", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_float_3,
            .m_Offset = reinterpret_cast<float(*)[3]>(CLASS_PROPERTY(ZHM3ItemWeapon, m_vClipPosOld))};

        static Glacier::RTP::ZDataProperty<int> CurFrame{
            .m_Node = {.m_Next = ClipPosOld, .m_Name = "m_nCurFrame", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_int,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemWeapon, m_nCurFrame)};

        static Glacier::RTP::ZDataProperty<int> ClipBoneNr{
            .m_Node = {.m_Next = CurFrame, .m_Name = "m_iClipBoneNr", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_int,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemWeapon, m_iClipBoneNr)};

        static Glacier::RTP::ZDataProperty<float> RecoilMasterValue{
            .m_Node = {.m_Next = ClipBoneNr, .m_Name = "m_fRecoilMasterValue", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_float,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemWeapon, m_fRecoilMasterValue)};

        static Glacier::RTP::ZDataProperty<float> RecoilShotImpact{
            .m_Node = {.m_Next = RecoilMasterValue, .m_Name = "m_fRecoilShotImpact", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_float,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemWeapon, m_fRecoilShotImpact)};

        static Glacier::RTP::ZDataProperty<float> RecoilDamping{
            .m_Node = {.m_Next = RecoilShotImpact, .m_Name = "m_fRecoilDamping", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_float,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemWeapon, m_fRecoilDamping)};
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZHM3ItemWeapon,        // ClassName
        Glacier::ZItemWeapon,  // BaseClass
        0x009B16A8,            // OldClassInfoAddr
        "ZHM3ItemWeapon",      // FactoryName
        0x0,                   // FactoryNameAddr (documentation only; not bound by the macro)
        cProperties::RecoilDamping, // FirstProperty
        0x0080FA24,            // PropertiesAddr (ZHM3ItemWeapon::Info)
        0x009B1508,            // IdAddr (ZHM3ItemWeapon::m_Id)
        0x009B150C             // MaskAddr (ZHM3ItemWeapon::m_Mask)
    );
#   pragma endregion
}
