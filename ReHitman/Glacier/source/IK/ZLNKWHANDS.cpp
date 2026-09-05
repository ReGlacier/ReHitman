#include <Glacier/IK/ZLNKWHANDS.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Materials/BS_Runtime.h>
#include <Glacier/Items/ITEMSTATE.h>
#include <Glacier/Items/ZItem.h>
#include <Glacier/Items/ZItemWeapon.h>
#include <Glacier/Items/ZItemContainer.h>
#include <Glacier/Items/ZItemTemplateWeapon.h>
#include <Glacier/ZSTL/StringUtils.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/GameBase/Action/ZActionChamberItem.h>
#include <Glacier/GameBase/Action/ZActionSwapItems.h>
#include <Glacier/GameBase/Action/ZActionReloadItem.h>
#include <Glacier/GameBase/Action/ZActionPutItem.h>
#include <Glacier/GameBase/Action/ZActionPickupItem.h>
#include <Glacier/GameBase/Action/ZActionGetItem.h>
#include <Glacier/GameBase/Action/ZActionDropItem.h>
#include <Glacier/GameBase/ZCheckVisible.h>
#include <Glacier/GameBase/ZPlayer.h>
#include <Glacier/GameBase/ZActor.h>
#include <Glacier/IK/ZBoneModifyBase.h>
#include <Glacier/Audio/ZSoundDllBase.h>
#include <Glacier/Audio/ZSoundObject.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Animation/Model.h>
#include <Glacier/Com/CCom.h>
#include <Glacier/ZSTL/REFTAB32.h>
#include <Glacier/Geom/ZTreeGroup.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/Physics/COLI.h>
#include <Glacier/Physics/ZCollisionBase.h>

#include <cmath>
#include <cstring>


namespace Glacier
{
    namespace
    {
        void CreateAAColiBox(
            const ZMat3x3& fromMat,
            const ZVector3& fromPos,
            const ZMat3x3& toMat,
            const ZVector3& toPos,
            const ZVector3& boxSize,
            ZVector3& resultCenter,
            ZVector3& resultSize)
        {
            ZVector3 pathMin {
                std::min(fromPos.x, toPos.x),
                std::min(fromPos.y, toPos.y),
                std::min(fromPos.z, toPos.z)
            };
            ZVector3 pathMax {
                std::max(fromPos.x, toPos.x),
                std::max(fromPos.y, toPos.y),
                std::max(fromPos.z, toPos.z)
            };

            ZVector3 fromCorners[8];
            ZVector3 toCorners[8];
            for (uint32_t i = 0; i < 8; ++i)
            {
                ZVector3 corner {
                    (i & 1) ? -boxSize.x : boxSize.x,
                    (i & 2) ? -boxSize.y : boxSize.y,
                    (i & 4) ? -boxSize.z : boxSize.z
                };
                vmmul(fromCorners[i], corner, fromMat);
                vmmul(toCorners[i], fromCorners[i], toMat);
            }

            ZVector3 fromMin = fromCorners[0];
            ZVector3 toMax = toCorners[0];
            for (uint32_t i = 1; i < 8; ++i)
            {
                fromMin.x = std::min(fromMin.x, fromCorners[i].x);
                fromMin.y = std::min(fromMin.y, fromCorners[i].y);
                fromMin.z = std::min(fromMin.z, fromCorners[i].z);
                toMax.x = std::max(toMax.x, toCorners[i].x);
                toMax.y = std::max(toMax.y, toCorners[i].y);
                toMax.z = std::max(toMax.z, toCorners[i].z);
            }

            pathMin += fromMin;
            pathMax += toMax;
            resultSize = (pathMax - pathMin) * 0.5f;
            resultCenter = pathMin + resultSize;

            ZASSERT(resultSize.x >= 0.0f);
            ZASSERT(resultSize.y >= 0.0f);
            ZASSERT(resultSize.z >= 0.0f);
            ZASSERT(vlen(resultSize) < 1000000.0f);
            ZASSERT(vlen(resultCenter) < 1000000.0f);
        }

        void InterpolateRotation(
            ZMat3x3& result,
            const ZMat3x3& from,
            const ZMat3x3& to,
            float factor)
        {
            if (factor < 0.00012207031f)
            {
                result = from;
                return;
            }
            if (factor > 0.99987793f)
            {
                result = to;
                return;
            }

            for (uint32_t i = 0; i < 6; ++i)
                result.data[i] = from.data[i] + (to.data[i] - from.data[i]) * factor;

            if (vnorm(result.data) < 0.00012207031f || vnorm(result.data + 3) < 0.00012207031f)
            {
                result = from;
                return;
            }

            vcross(result.data + 6, result.data + 3, result.data);
            if (vnorm(result.data + 6) < 0.00012207031f)
            {
                result = from;
                return;
            }
            vcross(result.data + 3, result.data, result.data + 6);
        }
    }

    ZLNKWHANDS::ZLNKWHANDS(const char* psName, ZBaseGeom* pBaseGeom)
        : ZCTRLIKLNKOBJ(psName, pBaseGeom)
        , m_bInMotion(false)
        , m_rSpeechSound(0)
        , m_SoundDef()
        , m_MaterialDef()
        , m_ContactMaterialDescID(1)
        , m_rMaterial(0)
        , m_fRecoil(0.0f)
        , m_pszHitAnim(nullptr)
        , m_HitAnimHandle()
        , m_pPullItemRight()
        , m_pPickupItemLeft()
        , m_pPickupItemRight()
        , m_pSwapItems()
        , m_pReloadRevolverLeft()
        , m_pReloadPistolRight()
        , m_pReloadRifle()
        , m_pReloadSubMachineGunRight()
        , m_pReloadShotgun()
        , m_pReloadPumpgun()
        , m_pReloadRPG()
        , m_pReloadGunOneHand()
        , m_AimTarget()
        , m_LHand(0)
        , m_RHand(0)
        , m_pNearItems(nullptr)
    {
        m_LHand.m_msgInventorySetActive = g_pEngineData->RegisterZMsg("MSG_INVERTORYSETACTIVELEFT", 0, __FILE__, __LINE__);
        m_LHand.m_lBoneId = 20;
        m_LHand.m_vItemOffset = { 0.0f, -4.0f, 0.0f };
        m_RHand.m_msgInventorySetActive = g_pEngineData->RegisterZMsg("MSG_INVERTORYSETACTIVERIGHT", 0, __FILE__, __LINE__);
        m_RHand.m_lBoneId = 25;
        m_Mask1 &= 0xFC;
        ZLNKWHANDS::s_msgCutSequenceEnd = g_pEngineData->RegisterZMsg("CutSequence_End", 0, __FILE__, __LINE__);

        m_AttachBones[0].m_Quat = {};
        m_AttachBones[0].m_Pos = {};
        m_AttachBones[0].m_Blend = 0.0;
        m_AttachBones[0].m_MagicNr = -1;
        m_AttachBones[1].m_Quat = {};
        m_AttachBones[1].m_Pos = { 0.f };
        m_AttachBones[1].m_Blend = 0.0;
        m_AttachBones[1].m_MagicNr = -1;

        ZLNKWHANDS::m_MaterialProperty_SoundContact = BS_Runtime::ZMaterialDescriptionDB::Instance().GetAudioPropertyId("ContactSound");
        ZLNKWHANDS::m_MaterialProperty_SoundMaterial = BS_Runtime::ZMaterialDescriptionDB::Instance().GetAudioPropertyId("SoundMaterial");

        m_bFootStepSoundsEnabled = false;
        m_bFootStepEventsEnabled = false;
        m_lFootStepEvent = 0;
    }

    ZLNKWHANDS::~ZLNKWHANDS()
    {
        if (m_pNearItems)
            ZUniMemory::Delete(m_pNearItems);

        m_pNearItems = nullptr;
        ZLNKWHANDS::bInitializedStaticAnimsZLNKWHANDS = false;
    }

    void ZLNKWHANDS::LoadSave(ISerializerStream& stream, bool bSaving)
    {
        ZCTRLIKLNKOBJ::LoadSave(stream, bSaving);

        stream.Exchange("m_fLastUpdatedPosition", m_fLastUpdatedPosition);
        stream.Exchange("m_bInMotion", m_bInMotion);
        stream.Exchange("m_rSpeechSound", m_rSpeechSound);
        stream.Exchange("m_bFootStepSoundsEnabled", m_bFootStepSoundsEnabled);
        stream.Exchange("m_bFootStepEventsEnabled", m_bFootStepEventsEnabled);
        stream.Exchange("m_lFootStepEvent", m_lFootStepEvent);

        int lContactMtlID = m_ContactMaterialDescID.m_Value;
        stream.Exchange("m_ContactMaterialDescID", lContactMtlID);
        m_ContactMaterialDescID.m_Value = lContactMtlID;

        stream.Exchange("m_rMaterial", m_rMaterial);
        stream.Exchange("m_fRecoil", m_fRecoil);

        ZASSERT(IsDerivedFrom<ZPlayer>() || m_AimTarget.GetCallBack() == nullptr);

        m_AimTarget.LoadSave(this, stream, bSaving);
        m_LHand.LoadSave(this, stream, bSaving);
        m_RHand.LoadSave(this, stream, bSaving);

        stream.Exchange("m_Mask1", m_Mask1);
        stream.Exchange("m_fDmgMultDefault", m_fDmgMultDefault);
        stream.Exchange("m_fDmgMultLeftArm", m_fDmgMultLeftArm);
        stream.Exchange("m_fDmgMultRightArm", m_fDmgMultRightArm);
        stream.Exchange("m_fDmgMultLeftLeg", m_fDmgMultLeftLeg);
        stream.Exchange("m_fDmgMultRightLeg", m_fDmgMultRightLeg);
        stream.Exchange("m_fDmgMultTorso", m_fDmgMultTorso);
        stream.Exchange("m_fDmgMultHead", m_fDmgMultHead);
        stream.Exchange("m_fDmgMultFace", m_fDmgMultFace);
        stream.Exchange("m_fDmgMultLeftHand", m_fDmgMultLeftHand);
        stream.Exchange("m_fDmgMultRightHand", m_fDmgMultRightHand);
    }

    const RTP::ZPropertyInfo& ZLNKWHANDS::GetProperties() const
    {
        return ZLNKWHANDS::Info;
    }

    uint32_t ZLNKWHANDS::GetObjectId() const
    {
        return ZLNKWHANDS::m_Id;
    }

    void ZLNKWHANDS::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZLNKWHANDS::m_Id;
        mask = ZLNKWHANDS::m_Mask;
    }

    ZGEOMCLASSINFO* ZLNKWHANDS::GetOldClassInfo() const
    {
        return ZLNKWHANDS::m_OldClassInfo;
    }

    void ZLNKWHANDS::ClassInit()
    {
        ZCTRLIKLNKOBJ::ClassInit();

        if (GetBoneNrFromId(32) != -1)
        {
            m_LHand.m_lBoneId = 32;
            m_LHand.m_vItemOffset = {};
        }

        if (GetBoneNrFromId(31) != -1)
        {
            m_RHand.m_lBoneId = 31;
        }
    }

    void ZLNKWHANDS::ClassInit2()
    {
        ZCTRLIKLNKOBJ::ClassInit2();
    }

    int32_t ZLNKWHANDS::ClassCommand(ZMSGID Msg, void* pData)
    {
        ZItem* pItem = nullptr;
        ZREF rItem = 0;
        ZGEOM* pItemOwner = nullptr;

        const bool hasItemArgument = m_msgDropItem == Msg
            || m_msgUseItem == Msg
            || m_msgCanPickupItem == Msg
            || m_msgPickupItem == Msg
            || m_msgReloadWeapon == Msg;
        if (m_msgCalcMats != Msg && hasItemArgument)
        {
            auto* pCom = static_cast<CSharedCom*>(pData);
            rItem = CComRead(pCom, "rItem");
            pItem = geom_cast<ZItem>(ZGEOM::RefToPtr(rItem));
            if (pItem)
                pItemOwner = pItem->GetItemOwner();

            if (m_msgCanPickupItem == Msg)
                pCom->SetVal("lGrey", 1);
        }

        if (m_msgDropItem == Msg)
        {
            ControlDrop(pItem);
            return 0;
        }

        if (m_msgUseItem == Msg)
        {
            auto* pCom = static_cast<CSharedCom*>(pData);
            auto* pItems = reinterpret_cast<REFTAB*>(static_cast<uintptr_t>(
                static_cast<uint32_t>(CComRead(pCom, "lItems"))));
            const int lItemType = CComRead(pCom, "lItemType");
            if (!pItems || !pItems->Count())
                return 0;

            const ZREF rFirst = pItems->GetRefNr(0);
            const ZREF rSecond = pItems->Count() == 2 ? pItems->GetRefNr(1) : 0;
            if (lItemType == 2)
            {
                if (rSecond)
                    ControlUseTwo(rFirst, rSecond);
                else
                    ControlUseOneExtra(rFirst);
            }
            else
            {
                ControlUseOneAlone(rFirst);
            }
            return 0;
        }

        if (m_msgEnterItemRange == Msg)
            AddNearItem(static_cast<ZREF>(reinterpret_cast<uintptr_t>(pData)));
        else if (m_msgLeaveItemRange == Msg)
            RemoveNearItem(static_cast<ZREF>(reinterpret_cast<uintptr_t>(pData)));

        if ((m_msgPickupItem == Msg || m_msgCanPickupItem == Msg) && pItemOwner && pItemOwner->IsInitialized())
        {
            ZMat3x3 itemMat;
            ZVector3 itemPos;
            pItem->GetRootTM(itemMat, itemPos);

            ZMat3x3 actorMat;
            ZVector3 actorPos;
            GetIKBoneMatPos(PelvisBoneIndex(), actorMat, actorPos);
            GetRootMatPos(actorMat, actorPos);

            ZVector3 delta;
            vsub(delta, itemPos, actorPos);
            if (std::fabs(itemPos.y - actorPos.y) < 200.0f)
            {
                delta.y = 0.0f;
                if (m_msgCanPickupItem == Msg && vlen2(delta) < 6400.0f)
                {
                    static_cast<CSharedCom*>(pData)->SetVal("lGrey", 0);
                }
                else
                {
                    const HANDSPICKUP pickup = ControlPickup(pItem);
                    if (pickup != BP_NONE)
                    {
                        if (auto* pActor = geom_cast<ZActor>(pItem->GetItemOwner());
                            pActor && (pActor->GetActorState() == ZActor::ACTORSTATE_DEAD ||
                                       pActor->GetActorState() == ZActor::ACTORSTATE_UNCONSCIOUS ||
                                       pActor->GetActorState() == ZActor::ACTORSTATE_SLEEPING))
                        {
                            pItem->SetItemOwner(0, nullptr, true, true);
                            pItem->SetState(IS_HIDE, nullptr);
                        }
                    }

                    switch (pickup)
                    {
                        case BP_LEFT:
                            AddAction(3, rItem);
                            break;
                        case BP_RIGHT:
                            AddAction(4, rItem);
                            break;
                        case BP_LEFTRIGHT:
                            AddAction(delta.x > 0.0f ? 3 : 4, rItem);
                            break;
                        case BP_LEFT_HIDE:
                            AddAction(3, rItem);
                            AddAction(5, 0);
                            break;
                        case BP_RIGHT_HIDE:
                        {
                            auto* pRHandItem = GetRHandItem();
                            if (pRHandItem)
                                AddAction(6, 0);
                            AddAction(4, rItem);
                            AddAction(6, 0);
                            if (pRHandItem)
                                AddAction(8, pRHandItem->GetRef());
                            break;
                        }
                        case BP_RIGHT_SWAP:
                            AddAction(4, rItem);
                            AddAction(11, 0);
                            break;
                        case static_cast<HANDSPICKUP>(7):
                            AddAction(4, rItem);
                            AddAction(6, 0);
                            break;
                        default:
                            break;
                    }
                }
            }
        }

        if (Msg == s_msgCutSequenceEnd)
        {
            if (static_cast<uintptr_t>(m_rCutSequence) == reinterpret_cast<uintptr_t>(pData))
            {
                if (auto* pLHandItem = GetLHandItem())
                    pLHandItem->Hide(false);
            }
        }
        else if (m_msgSoundStartet == Msg)
        {
            auto* pSoundDll = g_pSysInterface->GetSoundDll();
            const auto* pSoundMessage = static_cast<const uint32_t*>(pData);
            if (pSoundDll && pSoundMessage)
            {
                const SREF rSound = pSoundMessage[1];
                if (auto* pSound = g_pEngineData->SRefToPtr(rSound))
                {
                    char* pPoseData = pSound->GetPoseData();
                    if (pPoseData && std::memcmp(pPoseData, "LIP", 3) == 0)
                    {
                        const float multiplier = 1.0f / pSoundDll->GetPitchChange(pSound->GetCalculatedPitch());
                        if (auto* pPoseAnim = ActivatePoseAnim(pPoseData, pSound->GetTimeLeft(), rSound, multiplier))
                            pPoseAnim->GetExpressionID();
                    }
                }
            }
        }
        else if (m_msgSoundStopped == Msg)
        {
            auto* pSoundDll = g_pSysInterface->GetSoundDll();
            const auto* pSoundMessage = static_cast<const uint32_t*>(pData);
            if (pSoundDll && pSoundMessage && g_pEngineData->SRefToPtr(pSoundMessage[0]))
                StopPoseAnim(pSoundMessage[0], false);
        }
        else if (m_msgReloadWeapon == Msg)
        {
            Reload(pItem);
        }

        if (!pItemOwner)
            ZGEOM::ClassCommand(Msg, pData);
        return 0;
    }

    void ZLNKWHANDS::OnMoving()
    {
        if (m_iVisionID != 0xFF)
        {
            m_bInMotion = true;
            m_fLastUpdatedPosition = static_cast<float>(g_pSysInterface->FrameTime);
        }
    }

    void ZLNKWHANDS::OnMoved()
    {
        if (m_iVisionID != 0xFF)
        {
            ZCheckVisible::Instance().UpdateSeeableActor(this);
            m_bInMotion = false;
        }
    }

    void ZLNKWHANDS::CalcAnimRemapNames()
    {
        if (!bInitializedStaticAnimsZLNKWHANDS)
        {
            m_pPullGunRight = GetAnimVariationHandle("/Weapon/PullGun_Right");
            m_pAimRHandGuns = GetAnimVariationHandle("/FirstPerson/UB_1P_Hold_Dual_HardB");
            m_pPutGunLeft = GetAnimVariationHandle("/Weapon/PutGun_Left");
            m_pPutGunRight = GetAnimVariationHandle("/Weapon/PutGun_Right");
            m_pPutItemLeft = GetAnimVariationHandle("/Weapon/PutGun_Left");
            m_pPutItemRight = GetAnimVariationHandle("/Weapon/PutGun_Right");
            m_pDropItemLeft = GetAnimVariationHandle("/PickPut/UB_DropItem_Left");
            m_pDropItemRight = GetAnimVariationHandle("/PickPut/UB_DropItem_Right");
            m_pChamberGrenade = GetAnimVariationHandle("/Reload/Gun_OneHand");
            m_pChamberRPG = GetAnimVariationHandle("/Reload/Gun_OneHand");
            m_pListen[0] = GetAnimVariationHandle("/Talk_Listen/Listen_01");
            m_pListen[1] = GetAnimVariationHandle("/Talk_Listen/Listen_02");
            m_pTalk[0] = GetAnimVariationHandle("/Talk_Listen/Talk_01");
            m_pTalk[1] = GetAnimVariationHandle("/Talk_Listen/Talk_02");

            ZAnimVariationHandle talkFallback = m_pTalk[0];
            ZAnimVariationHandle listenFallback = m_pListen[0];
            if (talkFallback.IsValid())
            {
                if (!listenFallback.IsValid())
                    listenFallback = talkFallback;
            }
            else
            {
                talkFallback = listenFallback;
            }

            if (!m_pTalk[0].IsValid()) m_pTalk[0] = talkFallback;
            if (!m_pTalk[1].IsValid()) m_pTalk[1] = talkFallback;
            if (!m_pListen[0].IsValid()) m_pListen[0] = listenFallback;
            if (!m_pListen[1].IsValid()) m_pListen[1] = listenFallback;
            bInitializedStaticAnimsZLNKWHANDS = true;
        }

        m_pPickupItemLeft = GetAnimVariationHandle("/PickPut/Left");
        m_pPickupItemRight = GetAnimVariationHandle("/PickPut/Right");
        m_pPullItemRight = GetAnimVariationHandle("/Weapon/PullGun_Right");
        m_pReloadRevolverLeft = GetAnimVariationHandle("/Reload/Magnum");
        m_pReloadRevolverRight = GetAnimVariationHandle("/Reload/Magnum");
        m_pReloadPistolRight = GetAnimVariationHandle("/Reload/Hardballer");
        m_pReloadSubMachineGunRight = GetAnimVariationHandle("/Reload/Mp5");
        m_pReloadRifle = GetAnimVariationHandle("/Reload/Rifle_AK");
        m_pReloadShotgun = GetAnimVariationHandle("/Reload/Buckmark");
        m_pReloadPumpgun = GetAnimVariationHandle("/Reload/Ruger");
        m_pReloadGunOneHand = GetAnimVariationHandle("/Reload/Gun_OneHand");
    }

    void ZLNKWHANDS::Reset()
    {
        ZCTRLIKLNKOBJ::Reset();
        m_AimTarget.Clear();
        m_Mask &= ~4;
        m_fRecoil = 0.f;
    }

    ZLnkAction* ZLNKWHANDS::CreateLnkAction(uint32_t lActionId)
    {
        const auto getAnim = [this](ZAnimVariationHandle handle)
        {
            return GetAnimHeaderFromVariation(handle, m_iAnimVariationFlags,
                g_pSysInterface->FRand(const_cast<char*>(__FILE__), __LINE__));
        };

        switch (lActionId)
        {
            case 3: return ZUniMemory::New<ZActionPickupItem>(3, this, &m_LHand, getAnim(m_pPickupItemLeft));
            case 4: return ZUniMemory::New<ZActionPickupItem>(4, this, &m_RHand, getAnim(m_pPickupItemRight));
            case 5: return ZUniMemory::New<ZActionPutItem>(5, this, &m_LHand, getAnim(m_pPutGunLeft), getAnim(m_pPutItemLeft), true);
            case 6: return ZUniMemory::New<ZActionPutItem>(6, this, &m_RHand, getAnim(m_pPutGunRight), getAnim(m_pPutItemRight), true);
            case 7: return ZUniMemory::New<ZActionGetItem>(7, this, &m_LHand, getAnim(m_pPullItemRight), getAnim(m_pPullGunRight), true);
            case 8: return ZUniMemory::New<ZActionGetItem>(8, this, &m_RHand, getAnim(m_pPullItemRight), getAnim(m_pPullGunRight), true);
            case 9: return ZUniMemory::New<ZActionDropItem>(9, this, &m_LHand, getAnim(m_pDropItemLeft));
            case 10: return ZUniMemory::New<ZActionDropItem>(10, this, &m_RHand, getAnim(m_pDropItemRight));
            case 11: return ZUniMemory::New<ZActionSwapItems>(11, this, &m_RHand, &m_LHand, getAnim(m_pSwapItems));
            case 12: return ZUniMemory::New<ZActionReloadItem>(12, this, &m_LHand, getAnim(m_pReloadRevolverLeft), getAnim(m_pReloadPistolRight), getAnim(m_pReloadSubMachineGunRight), getAnim(m_pReloadRifle), getAnim(m_pReloadShotgun), getAnim(m_pReloadPumpgun), getAnim(m_pReloadRPG));
            case 13: return ZUniMemory::New<ZActionReloadItem>(13, this, &m_RHand, getAnim(m_pReloadRevolverRight), getAnim(m_pReloadPistolRight), getAnim(m_pReloadSubMachineGunRight), getAnim(m_pReloadRifle), getAnim(m_pReloadShotgun), getAnim(m_pReloadPumpgun), getAnim(m_pReloadRPG));
            case 14: return ZUniMemory::New<ZActionChamberItem>(14, this, &m_RHand, getAnim(m_pChamberRPG), getAnim(m_pChamberPumpgun), getAnim(m_pChamberShotgun), getAnim(m_pChamberRifle), getAnim(m_pChamberGunRight), getAnim(m_pChamberGunLeft));
            case 15: return ZUniMemory::New<ZActionChamberItem>(15, this, &m_LHand, getAnim(m_pChamberGrenade), getAnim(m_pChamberPumpgun), getAnim(m_pChamberShotgun), getAnim(m_pChamberRifle), getAnim(m_pChamberGunRight), getAnim(m_pChamberGunLeft));
            default: return nullptr;
        }
    }

    void ZLNKWHANDS::UpdateTargets()
    {
        ZCTRLIKLNKOBJ::UpdateTargets();

        UpdateAimTarget();

        m_LHand.Update(this);
        m_RHand.Update(this);
    }

    void ZLNKWHANDS::ResetTargets()
    {
        ZIKLNKOBJ::ResetTargets();
    }

    void ZLNKWHANDS::RemoveTargets()
    {
        ZIKLNKOBJ::RemoveTargets();
    }

    void ZLNKWHANDS::AddNearItem(ZREF rItem)
    {
        if (!m_pNearItems)
        {
            m_pNearItems = ZUniMemory::New<REFTAB>(4, 0);
        }

        if (ZGEOM::RefToPtr(rItem)->Active())
        {
            m_pNearItems->Add(rItem);
        }
    }

    void ZLNKWHANDS::RemoveNearItem(ZREF rItem)
    {
        if (m_pNearItems && m_pNearItems->Exists(rItem))
        {
            m_pNearItems->Remove(rItem);
        }
    }

    void ZLNKWHANDS::OnBoidPushing(ZLNKWHANDS* pBoid)
    {
    }

    void ZLNKWHANDS::OnBoidPushed(ZLNKWHANDS* pBoid)
    {
    }

    bool ZLNKWHANDS::CheckRHandFireTarget()
    {
        if (!m_RHand.m_HandInfo.m_bIKItemEnabled)
            return false;

        auto* pWeapon = GetRHandWeapon();
        if (!pWeapon)
            return false;

        auto* pMuzzleExitPos = pWeapon->GetMuzzleExitPos();
        auto* pTarget = ZGEOM::RefToPtr(m_AimTarget.Geom());
        if (!pMuzzleExitPos || !pTarget)
            return false;

        ZVector3 vMuzzlePos;
        pMuzzleExitPos->BaseGeom()->GetPos(vMuzzlePos);
        pWeapon->GetRootPoint(vMuzzlePos);

        ZVector3 vTargetPos {};
        pTarget->GetRootPoint(vTargetPos);
        vsub(vTargetPos, vMuzzlePos);

        COLI coli;
        coli.lp = vMuzzlePos;
        coli.ln = vTargetPos;
        ZROOT->ChkLineColi(&coli, eGlobalTreeType::GT_StdObjs, 2, -1, true, false);

        return coli.ColiRef == m_AimTarget.Geom();
    }

    ZREF ZLNKWHANDS::GetRHandFireTarget()
    {
        if (!m_RHand.m_HandInfo.m_bIKItemEnabled)
            return 0;

        auto* pWeapon = GetRHandWeapon();
        if (!pWeapon)
            return 0;

        ZVector3 vTargetPos;
        if (!m_AimTarget.GetPos(vTargetPos))
            return 0;

        GetRootPoint(vTargetPos);
        pWeapon->SetTarget(&vTargetPos);

        auto* pMuzzleExitPos = pWeapon->GetMuzzleExitPos();
        if (!pMuzzleExitPos)
            return 0;

        ZVector3 vMuzzlePos;
        pMuzzleExitPos->BaseGeom()->GetPos(vMuzzlePos);
        pWeapon->GetRootPoint(vMuzzlePos);

        ZVector3 vFireDirection;
        vsub(vFireDirection, vTargetPos, vMuzzlePos);
        vsetlen(vFireDirection, 10000.0f);

        COLI coli;
        coli.lp = vMuzzlePos;
        coli.ln = vFireDirection;
        if (ZROOT->ChkLineColi(&coli, eGlobalTreeType::GT_StdObjs, 2, -1, true, false))
            return coli.ColiRef;

        return 0;
    }

    void ZLNKWHANDS::SetAimInPosition(bool bValue)
    {
        m_bAimInPosition = bValue;
    }

    void ZLNKWHANDS::AddAction(int lType, ZREF rTarget)
    {
        auto* pTarget = ZGEOM::RefToPtr(rTarget);
        ZLnkAction* pLnkAction = nullptr;

        switch (lType)
        {
            case 3:
            case 4:
                pLnkAction = CreateLnkAction(lType);
                static_cast<ZActionPickupItem*>(pLnkAction)->SetItem(geom_cast<ZItem>(pTarget));
            break;

            case 7:
            case 8:
                pLnkAction = CreateLnkAction(lType);
                static_cast<ZActionGetItem*>(pLnkAction)->SetItem(geom_cast<ZItem>(pTarget));
            break;

            case 5:
            case 6:
            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
                pLnkAction = CreateLnkAction(lType);
            break;

            default:
            {
                // Unsupported
                ZASSERT(false);
            }
        }

        RunLnkAction(pLnkAction);
    }

    const char* ZLNKWHANDS::GetAnimNameFromCollision(SIKBoneCollision* pCol, bool bBigForce, ZItemTemplateWeapon* pITW)
    {
        WEAPONTYPE weaponType = WT_PISTOL;
        if (pITW)
            weaponType = static_cast<WEAPONTYPE>(pITW->GetWeaponType());

        if (IsDerivedFrom<ZPlayer>())
            bBigForce = false;

        if (weaponType == WT_PIANO)
            return striwcmp(pITW->Name(), "*Poison*") ? "Die_Carrot" : "Die_Poison";
        if (weaponType == WT_KNIFE && bBigForce)
            return "Die_Carrot";

        const char* pszAnimName = nullptr;
        switch (pCol->cBodyPart)
        {
            case 0: pszAnimName = bBigForce ? (pCol->bFront ? "*HitShoulderLeftFrontBig" : "*HitShoulderLeftBackBig") : (pCol->bFront ? "*HitShoulderLeftFrontSmall" : "*HitShoulderLeftBackSmall"); break;
            case 1: pszAnimName = bBigForce ? (pCol->bFront ? "*HitTorsoFrontBig" : "*HitTorsoBackBig") : (pCol->bFront ? "*HitTorsoFrontSmall" : "*HitTorsoBackSmall"); break;
            case 2:
            case 3: pszAnimName = "*HitHeadFrontSmall"; break;
            case 4:
            case 5: pszAnimName = bBigForce ? (pCol->bFront ? "*HitShoulderLeftFrontBig" : "*HitShoulderLeftBackBig") : (pCol->bFront ? "*HitShoulderLeftFront" : "*HitShoulderLeftBack"); break;
            case 6: pszAnimName = bBigForce ? (pCol->bFront ? "*HitLegLeftFrontBig" : "*HitLegLeftBackBig") : (pCol->bFront ? "*HitLegLeftFrontSmall" : "*HitLegLeftBackSmall"); break;
            case 7:
            case 8: pszAnimName = bBigForce ? (pCol->bFront ? "*HitShoulderRightFrontBig" : "*HitShoulderRightBackBig") : (pCol->bFront ? "*HitShoulderRightBack" : "*HitShoulderRightfRONT"); break;
            case 9: pszAnimName = bBigForce ? (pCol->bFront ? "*HitLegRightFrontBig" : "*HitLegRightBackBig") : (pCol->bFront ? "*HitLegRightFrontSmall" : "*HitLegRightBackSmall"); break;
        }

        if (!pszAnimName || !GetAnim(pszAnimName))
            return pCol->bFront ? "*Hit*FrontSmall" : "*Hit*BackSmall";
        return pszAnimName;
    }

    void ZLNKWHANDS::ReloadItem(ZItem* pItem)
    {
        ZASSERT(pItem && pItem->IsDerivedFrom<ZItemWeapon>());
        auto* pWeapon = static_cast<ZItemWeapon*>(pItem);
        ZASSERT(pWeapon->GetAmmoTemplate());

        const int available = GetAmmoFor(pWeapon);
        const int loaded = pWeapon->GetProjectilesInMagazine();
        const int capacity = pWeapon->GetProjectilesPerMagazine();
        const int newLoaded = available + loaded < capacity ? available + loaded : capacity;
        SetAmmoFor(pWeapon, available - (newLoaded - loaded));
        pWeapon->SetProjectilesInMagazine(newLoaded);
        pWeapon->SetBulletInChamber(true);
    }

    void ZLNKWHANDS::LetItemFall(ZItem* pItem)
    {
        PerformDrop(pItem, true, false);
    }

    bool ZLNKWHANDS::ShowReloadAnim(ZItemWeapon* pWeapon) const
    {
        return true;
    }

    bool ZLNKWHANDS::ShowChamberAnim(ZItemWeapon* pWeapon) const
    {
        return true;
    }

    HANDSPICKUP ZLNKWHANDS::ControlPickup(ZItem* pItem)
    {
        ZASSERT(pItem);
        if (CurrentLnkActionId() == 4 || CurrentLnkActionId() == 3)
            return BP_NONE;

        pItem->GetItemTemplate();
        if (pItem->IsDerivedFrom<ZItemContainer>())
        {
            if (CurrentLnkActionId() == 3)
                return BP_NONE;

            auto* pLHandItem = GetLHandItem();
            if (!pLHandItem)
                return BP_LEFT;

            auto* pTemplate = pLHandItem->GetItemTemplate();
            if (pTemplate->GetItemSize() == ITEMSIZE_LARGE)
            {
                if (CurrentLnkActionId() == 9)
                    return BP_NONE;
                AddAction(9, 0);
            }
            else
            {
                if (CurrentLnkActionId() == 5)
                    return BP_NONE;
                AddAction(5, 0);
            }
            return BP_LEFT;
        }

        if (CurrentLnkActionId() != 4)
            return BP_NONE;

        if (auto* pRHandItem = GetRHandItem())
        {
            auto* pTemplate = pRHandItem->GetItemTemplate();
            if (pTemplate->GetItemHands() == IH_TWOHANDED)
            {
                if (CurrentLnkActionId() == 10)
                    return BP_NONE;
                AddAction(10, 0);
            }
            else if (pTemplate->GetItemSize() == eITEMSIZE_SMALL)
            {
                if (CurrentLnkActionId() == 6)
                    return BP_NONE;
                AddAction(6, 0);
            }
        }
        return BP_RIGHT;
    }

    void ZLNKWHANDS::ControlUseOneExtra(ZREF rRef)
    {
        ZASSERT(false);
    }

    void ZLNKWHANDS::ControlUseOneAlone(ZREF rRef)
    {
        auto* pItem = ZGEOM::RefToPtr(rRef);
        auto* pRHandItem = GetRHandItem();
        auto* pLHandItem = GetLHandItem();
        if (pItem == pRHandItem)
            return;
        if (pItem && pItem == pLHandItem)
        {
            AddAction(11, 0);
            return;
        }

        if (pRHandItem)
        {
            auto* pTemplate = pRHandItem->GetItemTemplate();
            if (pTemplate->GetItemSize() == ITEMSIZE_LARGE)
            {
                if (pTemplate->GetItemHands() == IH_TWOHANDED || pLHandItem)
                    AddAction(10, 0);
                else
                    AddAction(11, 0);
            }
            else
            {
                AddAction(6, 0);
            }
        }

        if (rRef)
            AddAction(8, rRef);
    }

    void ZLNKWHANDS::ControlUseTwo(ZREF rFirst, ZREF rSecond)
    {
        ZASSERT(false);
    }

    void ZLNKWHANDS::ControlDrop(ZItem* pItem)
    {
        if (pItem)
        {
            if (GetLHandItem() == pItem)
            {
                if (CurrentLnkActionId() != 9)
                    AddAction(9, 0);
                return;
            }
            if (GetRHandItem() != pItem)
                return;
        }

        if (CurrentLnkActionId() != 10)
            AddAction(10, 0);
    }

    void ZLNKWHANDS::ControlPushButton(ZGEOM* pGeom)
    {
        ZASSERT(false);
    }

    int32_t ZLNKWHANDS::GetPickupAction(HANDPICKUP& lResLHand, HANDPICKUP& lResRHand, HANDSPICKUP& lResBoth, ITEMHANDS lLHand, ITEMHANDS lRHand, ITEMHANDS lPickup, bool bPickupFit) const
    {
        return 0;
    }

    void ZLNKWHANDS::ModifyPickupAction(ITEMHANDS& lLHand, ITEMHANDS& lResRHand, ITEMHANDS& lPickup, ZItemTemplate* pItemTemplate) const
    {
    }

    Animation::Header* ZLNKWHANDS::GetAimAnim(Animation::Header** pRecoil, uint32_t& lExtraFlags) const
    {
        *pRecoil = nullptr;
        lExtraFlags = 0;

        auto* pRHandWeapon = GetRHandWeapon();
        if (!pRHandWeapon)
            return nullptr;
        auto* pRTemplate = pRHandWeapon->GetWeaponTemplate();
        const auto weaponType = static_cast<WEAPONTYPE>(pRTemplate->GetWeaponType());
        if (weaponType != WT_PISTOL && weaponType != WT_REVOLVER)
            return nullptr;

        auto* pLHandWeapon = GetLHandWeapon();
        if (!pLHandWeapon || pLHandWeapon->GetWeaponTemplate()->GetWeaponType() != pRTemplate->GetWeaponType())
            return nullptr;

        return GetAnimHeaderFromVariation(m_pAimRHandGuns, m_iAnimVariationFlags,
            g_pSysInterface->FRand(const_cast<char*>(__FILE__), __LINE__));
    }

    void ZLNKWHANDS::GetPoseEffects(float* poseWeights, int16_t* poseIdToIndex)
    {
        if (!m_pPoseAnim)
            return;

        for (int i = m_pPoseAnim->StartCount(); i < m_pPoseAnim->Count(); ++i)
        {
            const int poseId = static_cast<uint8_t>(m_pPoseAnim->GetPoseName(i));
            const float weight = m_pPoseAnim->GetPoseWeight(i, static_cast<float>(g_pSysInterface->m_fRealTime));
            if (weight <= -5.0f || weight >= 5.0f || poseId >= 126)
                continue;

            const int poseIndex = m_Model->m_Poses.idToPosLookup()[poseId];
            if (poseIndex != -1)
                m_Model->m_PoseWeights[poseIndex] = weight * 0.5f;
        }
    }

    void ZLNKWHANDS::SetHandTarget(int handIndex, Glacier::ZVector3& vWorldPos, float fWeight)
    {
        if (m_Model)
        {
            auto& target = m_Model->m_Targets[handIndex + 3];
            target.m_Pos2 = vWorldPos;
            target.m_Weight2 = fWeight;
        }
    }

    int ZLNKWHANDS::DontAnimateAttachers()
    {
        return 0;
    }

    void ZLNKWHANDS::RemoveLHandIKTarget(float fTime, ZIKCALLBACK cb)
    {
        m_LHand.Remove(this, fTime, cb);
    }

    void ZLNKWHANDS::RemoveRHandIKTarget(float fTime, ZIKCALLBACK cb)
    {
        m_RHand.Remove(this, fTime, cb);
    }

    bool ZLNKWHANDS::UpdateAimTarget()
    {
        if (!m_AimTarget.IsEnabled())
        {
            m_Mask1 &= ~4u;
            m_fRecoil = 0.0f;
            return false;
        }

        if (m_Active & 0x400000u)
        {
            m_fRecoil = 0.0f;
            return m_bAimInPosition;
        }

        const float timePrc = m_AimTarget.GetTimePrc();
        if (timePrc == 1.0f)
            m_bAimInPosition = true;

        ZVector3 target;
        if (!GetAimTarget(target))
            target.Reset();

        auto* pWeapon = GetRHandWeapon();
        if (!pWeapon)
            pWeapon = GetLHandWeapon();
        if (pWeapon)
            pWeapon->SetTarget(&target);

        if (!m_bCutSequence)
        {
            ZVector3 localTarget = target;
            GetLocalPoint(localTarget);
            m_pBoneModify->m_fAimTimePrc = timePrc;
            m_pBoneModify->m_vAimTarget = localTarget;
        }

        m_Model->m_Targets[1].m_Pos2 = target;
        m_Model->m_Targets[1].m_Data[4] = m_fRecoil;
        m_AimTarget.CallBackIfTime(this);
        return m_bAimInPosition;
    }

    void ZLNKWHANDS::SetAimTarget(ZREF rGeom, const ZVector3& vP0, float fTime, ZIKCALLBACK cb, int lBoneId)
    {
        if (m_bAimDisabled)
            return;

        m_pBoneModify->m_fAimBlendSpeed = fTime;
        if (IsAiming() && rGeom && m_AimTarget.Geom() == rGeom)
        {
            m_Active &= ~0x400000u;
            return;
        }

        ZVector3 target = vP0;
        GetRootPoint(target);
        m_Active &= ~0x400000u;
        m_AimTarget.SetMatPosGeom(nullptr, &target, rGeom);
        m_AimTarget.SetBoneId(lBoneId);
        m_AimTarget.SetCallBack(cb);
        if (!m_AimTarget.IsEnabled())
            m_AimTarget.SetTime(fTime, false);
        if ((!m_AimTarget.IsEnabled() || m_AimTarget.Removing() || rGeom) && m_AimTarget.GetTimePrc() != 1.0f)
            m_AimTarget.SetTime(fTime, false);
        m_AimTarget.Enable(true);
    }

    void ZLNKWHANDS::SetAimTarget(const ZVector3& vP0, float fTime, ZIKCALLBACK cb)
    {
        SetAimTarget(0, vP0, fTime, cb, 4);
    }

    void ZLNKWHANDS::RemoveAimTarget(float fTime, ZIKCALLBACK cb)
    {
        if (m_bAimDisabled)
            return;

        if ((GetBoneControl(RHandBoneIndex()) & 0x100) != 0)
            fTime = 0.0f;

        if (auto* pWeapon = GetRHandWeapon())
        {
            ZVector3 target;
            pWeapon->GetTarget(&target);
        }

        m_AimTarget.SetCallBack(cb);
        if (fTime == 0.0f)
        {
            m_AimTarget.SetTime(0.0f, true);
            UpdateAimTarget();
        }
        else if (!m_AimTarget.Removing())
        {
            m_AimTarget.SetTime(fTime, true);
        }
    }

    void ZLNKWHANDS::FreezeAimTarget()
    {
        if (m_AimTarget.IsEnabled())
            m_Active |= 0x400000u;
    }

    void ZLNKWHANDS::DisableAim()
    {
        ZIKCALLBACK cb{};
        cb.lCallBackId = -1;
        RemoveAimTarget(0.0f, cb);
        m_bAimDisabled = true;
    }

    void ZLNKWHANDS::EnableAim()
    {
        m_bAimDisabled = false;
    }

    bool ZLNKWHANDS::GetAimTarget(ZVector3& vTarget)
    {
        if (auto* pGeom = ZGEOM::RefToPtr(m_AimTarget.Geom()))
        {
            pGeom->GetRootPoint(vTarget);
            return true;
        }

        if (!m_AimTarget.GetPos(vTarget))
            return false;
        GetRootPoint(vTarget);
        return true;
    }

    bool ZLNKWHANDS::GetAimTarget(ZREF& rRef) const
    {
        rRef = m_AimTarget.Geom();
        return rRef != 0;
    }

    ZREF ZLNKWHANDS::GetAimTarget() const
    {
        return m_AimTarget.Geom();
    }

    float ZLNKWHANDS::GetDrawWeaponSpeed()
    {
        return g_pSysInterface->FRand(const_cast<char*>(__FILE__), __LINE__) * 0.4f + 0.8f;
    }

    bool ZLNKWHANDS::IsAiming() const
    {
        return m_AimTarget.IsEnabled();
    }

    void ZLNKWHANDS::SetAimAnimPrc(float, float, float)
    {
    }

    void ZLNKWHANDS::FireShotNotify()
    {
        if (m_fRecoil < 0.3f)
            m_fRecoil = 1.0f;
    }

    void ZLNKWHANDS::SetHitAnimation(char const* szAnimName)
    {
        m_pszHitAnim = szAnimName;
    }

    void ZLNKWHANDS::SetHitAnimHandle(ZAnimVariationHandle& hAnim)
    {
        m_HitAnimHandle = hAnim;
    }

    void ZLNKWHANDS::UseItem(ZItem* pItem, int lType)
    {
        CCom com;
        REFTAB32 items;
        items.Add(pItem ? pItem->GetRef() : 0);
        com.SetVal("lItems", &items, CCOM_TYPE_GEOMREFTAB);
        com.SetVal("lItemType", lType);
        ZASSERT(m_msgUseItem);
        ClassCommand(static_cast<ZMSGID>(m_msgUseItem), &com);
    }

    void ZLNKWHANDS::DropItem(ZItem* pItem)
    {
        CCom com;
        com.SetVal("rItem", pItem->GetRef(), CCOM_TYPE_GEOMREF);
        ZASSERT(m_msgDropItem);
        ClassCommand(static_cast<ZMSGID>(m_msgDropItem), &com);
    }

    void ZLNKWHANDS::PickupItem(ZItem* pItem)
    {
        CCom com;
        com.SetVal("rItem", pItem->GetRef(), CCOM_TYPE_GEOMREF);
        ZASSERT(m_msgPickupItem);
        ClassCommand(static_cast<ZMSGID>(m_msgPickupItem), &com);
    }

    void ZLNKWHANDS::Reload(ZItem* pItem)
    {
        ZASSERT(pItem && pItem->IsDerivedFrom<ZItemWeapon>());
        auto* pWeapon = static_cast<ZItemWeapon*>(pItem);
        if (!GetAmmoFor(pWeapon))
            return;

        if (pItem == GetRHandItem())
        {
            if (CurrentLnkActionId() != 13)
                AddAction(13, pItem->GetRef());
        }
        else if (pItem == GetLHandItem() && CurrentLnkActionId() != 12)
        {
            AddAction(12, pItem->GetRef());
        }
    }

    void ZLNKWHANDS::Chamber(ZItem* pItem)
    {
        if (pItem == GetRHandItem())
        {
            if (CurrentLnkActionId() != 14)
                AddAction(14, pItem->GetRef());
        }
        else if (pItem == GetLHandItem() && CurrentLnkActionId() != 15)
        {
            AddAction(15, pItem->GetRef());
        }
    }

    void ZLNKWHANDS::PerformDrop(ZItem* pItem, bool bAddRigidBody, bool bSilent)
    {
        pItem->SetState(eIS_NORMAL, nullptr);
        pItem->BaseGeom()->ForceCalcMaxMin();

        ZMat3x3 itemMat;
        ZVector3 itemPos;
        pItem->GetItemRootTM(itemMat.data, itemPos.Get());

        ZVector3 itemCenter;
        vmmul(itemCenter, pItem->BaseGeom()->m_vCen, itemMat);
        itemPos += itemCenter;

        ZMat3x3 actorMat;
        ZVector3 actorPos;
        GetRootCenter(actorMat, actorPos);

        ZMat3x3 fromMat;
        fromMat.data[0] = actorMat.data[6];
        fromMat.data[1] = actorMat.data[7];
        fromMat.data[2] = actorMat.data[8];
        fromMat.data[3] = actorMat.data[0];
        fromMat.data[4] = actorMat.data[1];
        fromMat.data[5] = actorMat.data[2];
        fromMat.data[6] = actorMat.data[3];
        fromMat.data[7] = actorMat.data[4];
        fromMat.data[8] = actorMat.data[5];

        ZVector3 collisionCenter;
        ZVector3 collisionSize;
        CreateAAColiBox(fromMat, actorPos, itemMat, itemPos, pItem->BaseGeom()->m_vSize, collisionCenter, collisionSize);

        char collisionBuffer[51200];
        auto* pCollision = ZCollisionBase::s_pCollisionBase;
        auto* pCollisionBox = pCollision->LockCollisionBox(collisionBuffer, sizeof(collisionBuffer));
        ZMat3x3 identity;
        identity.Reset();
        pCollisionBox->SetBox(identity, collisionCenter, collisionSize);
        pCollisionBox->GetStrips(0x20);

        float lastClearFactor = 0.0f;
        ZMat3x3 testMat = fromMat;
        ZVector3 testPos = actorPos;
        for (float factor = 0.1f; factor <= 1.0f; factor += 0.1f)
        {
            InterpolateRotation(testMat, fromMat, itemMat, factor);
            testPos = actorPos + (itemPos - actorPos) * factor;
            if (pCollisionBox->CheckBoxCollision(testMat, testPos, pItem->BaseGeom()->m_vSize))
                break;
            lastClearFactor = factor;
        }
        pCollision->UnlockCollisionBox(pCollisionBox);

        const ZMat3x3 targetMat = itemMat;
        InterpolateRotation(itemMat, fromMat, targetMat, lastClearFactor);
        itemPos = actorPos + (itemPos - actorPos) * lastClearFactor;
        pItem->SetItemOwner(0, GetOwner(false), true, true);

        vmmul(itemCenter, pItem->BaseGeom()->m_vCen, itemMat);
        itemPos -= itemCenter;
        pItem->SetRootTM(itemMat, itemPos);
    }

    ZIKHAND* ZLNKWHANDS::GetRHand()
    {
        return &m_RHand;
    }

    ZIKHAND* ZLNKWHANDS::GetLHand()
    {
        return &m_LHand;
    }

    ZItem* ZLNKWHANDS::GetLHandItem() const
    {
        return m_LHand.GetItem();
    }

    ZItem* ZLNKWHANDS::GetRHandItem() const
    {
        return m_RHand.GetItem();
    }

    ZItemWeapon* ZLNKWHANDS::GetLHandWeapon() const
    {
        auto* pItem = m_LHand.GetItem();
        return pItem && pItem->IsDerivedFrom<ZItemWeapon>()
            ? static_cast<ZItemWeapon*>(pItem)
            : nullptr;
    }

    ZItemWeapon* ZLNKWHANDS::GetRHandWeapon() const
    {
        auto* pItem = m_RHand.GetItem();
        return pItem && pItem->IsDerivedFrom<ZItemWeapon>()
            ? static_cast<ZItemWeapon*>(pItem)
            : nullptr;
    }

    ITEMHANDS ZLNKWHANDS::GetRHandItemType() const
    {
        return m_RHand.GetItemType();
    }

    ITEMHANDS ZLNKWHANDS::GetLHandItemType() const
    {
        return m_LHand.GetItemType();
    }

    void ZLNKWHANDS::SetRHandItemState(ITEMSTATE lState)
    {
        if (auto* pItem = GetRHandItem())
            pItem->SetState(lState, nullptr);
    }

    void ZLNKWHANDS::SetLHandItemState(ITEMSTATE lState)
    {
        if (auto* pItem = GetLHandItem())
            pItem->SetState(lState, nullptr);
    }

    ZREF ZLNKWHANDS::AttachItemToHand(ZIKHAND* pHand, ZREF rItem)
    {
        if (pHand == &m_LHand)
            return AttachLHandItem(rItem);
        if (pHand == &m_RHand)
            return AttachRHandItem(rItem);

        ZASSERT(false);
        return 0;
    }

    ZREF ZLNKWHANDS::AttachRHandItem(ZREF rItem)
    {
        return m_RHand.AttachItem(this, rItem);
    }

    ZREF ZLNKWHANDS::AttachLHandItem(ZREF rItem)
    {
        return m_LHand.AttachItem(this, rItem);
    }

    REFTAB* ZLNKWHANDS::GetNearItems() const
    {
        return m_pNearItems;
    }

    REFTAB* ZLNKWHANDS::GetNearItems()
    {
        return m_pNearItems;
    }

    void ZLNKWHANDS::SetDialog(bool bEnable)
    {
        m_bDialog = bEnable;
    }

    bool ZLNKWHANDS::GetDialog() const
    {
        return m_bDialog;
    }

    void ZLNKWHANDS::DialogListen(int lType)
    {
        if (lType < 0) lType = 0;
        if (lType > 1) lType = 1;
        auto* pAnim = GetAnimHeaderFromVariation(m_pListen[lType], m_iAnimVariationFlags,
            g_pSysInterface->FRand(const_cast<char*>(__FILE__), __LINE__));
        PlayAnimSegment(pAnim, 5, 0.0f, -1.0f, 1.0f);
    }

    void ZLNKWHANDS::DialogTalk(int lType)
    {
        if (lType < 0) lType = 0;
        if (lType > 1) lType = 1;
        auto* pAnim = GetAnimHeaderFromVariation(m_pTalk[lType], m_iAnimVariationFlags,
            g_pSysInterface->FRand(const_cast<char*>(__FILE__), __LINE__));
        PlayAnimSegment(pAnim, 5, 0.0f, -1.0f, 1.0f);
    }

    void ZLNKWHANDS::SlipRHandItem()
    {
        m_RHand.SlipItem(this);
    }

    void ZLNKWHANDS::SlipLHandItem()
    {
        m_LHand.SlipItem(this);
    }

    void ZLNKWHANDS::ForceLHandItem(ZItem* pZItem, bool bBruteForce)
    {
        const ITEMHANDS itemType = GetLHandItemType();
        if (bBruteForce || itemType == IH_NONE)
        {
            if (pZItem)
            {
                AttachLHandItem(pZItem->GetRef());
                pZItem->SetState(eIS_NORMAL, nullptr);
            }
            else
            {
                AttachLHandItem(0);
            }
            return;
        }

        if (itemType == IH_ONEHANDED)
        {
            if (auto* pCurrentItem = m_RHand.GetItem())
            {
                pCurrentItem->SetState(IS_HIDE, nullptr);
                m_RHand.Remove(this, 0.0f, {});
                m_RHand.AttachItem(this, 0);
            }
        }
        else if (auto* pCurrentItem = m_LHand.GetItem())
        {
            m_LHand.Remove(this, 0.0f, {});
            LetItemFall(pCurrentItem);
        }
    }

    void ZLNKWHANDS::ForceRHandItem(ZItem* pZItem, bool bBruteForce)
    {
        const ITEMHANDS itemType = GetRHandItemType();
        if (pZItem == m_RHand.GetItem())
            return;

        if (bBruteForce || itemType == IH_NONE)
        {
            if (pZItem)
            {
                AttachRHandItem(pZItem->GetRef());
                pZItem->SetState(eIS_NORMAL, nullptr);
            }
            else
            {
                AttachRHandItem(0);
            }
            return;
        }

        if (itemType == IH_ONEHANDED)
        {
            if (auto* pCurrentItem = m_RHand.GetItem())
            {
                pCurrentItem->SetState(IS_HIDE, nullptr);
                m_RHand.Remove(this, 0.0f, {});
                m_RHand.AttachItem(this, 0);
            }
        }
        else if (auto* pCurrentItem = m_RHand.GetItem())
        {
            m_RHand.Remove(this, 0.0f, {});
            LetItemFall(pCurrentItem);
        }
    }

    void ZLNKWHANDS::DestroyRHandItem()
    {
        auto* pItem = GetRHandItem();
        ForceRHandItem(nullptr, true);
        pItem->SetState(ITEMSTATE::eIS_NORMAL, nullptr);
        pItem->SetItemOwner(0, nullptr, true, true);
        pItem->Delete();
    }

    void ZLNKWHANDS::DestroyLHandItem()
    {
        auto* pItem = GetLHandItem();
        ForceLHandItem(nullptr, true);
        pItem->SetState(ITEMSTATE::eIS_NORMAL, nullptr);
        pItem->SetItemOwner(0, nullptr, true, true);
        pItem->Delete();
    }

    void ZLNKWHANDS::EmptyHands()
    {
        ForceLHandItem(nullptr, false);
        ForceRHandItem(nullptr, false);
    }

    int32_t ZLNKWHANDS::GetAmmoFor(ZItemWeapon* pWeapon)
    {
        ZASSERT(false);
        return 0;
    }

    void ZLNKWHANDS::SetAmmoFor(ZItemWeapon* pWeapon, int lAmount)
    {
        ZASSERT(false);
    }

    void ZLNKWHANDS::SetLHandIKTarget(const ZMat3x3& m0, const ZVector3& v0, float fTime, ZIKCALLBACK cb)
    {
        m_LHand.SetTarget(this, m0, v0, fTime, cb);
    }

    void ZLNKWHANDS::SetRHandIKTarget(const ZMat3x3& m0, const ZVector3& v0, float fTime, ZIKCALLBACK cb)
    {
        m_RHand.SetTarget(this, m0, v0, fTime, cb);
    }

    void ZLNKWHANDS::UpdateItemActions()
    {
        ZASSERT(false);
    }

    void ZLNKWHANDS::CallBackItemAction()
    {
        ZASSERT(false);
    }

    SREF ZLNKWHANDS::PlaySpeech(uint32_t lDefinitionIndex, int lSoundEvent, bool bUseFilter)
    {
        if (!g_pSysInterface->GetSoundDll())
            return 0;
        return PlaySpeechResource(m_SoundDef.GetSoundFromEnumIndex(static_cast<uint8_t>(lDefinitionIndex)), lSoundEvent);
    }

    SREF ZLNKWHANDS::PlaySpeechResource(uint32_t lSoundIndex, int lSoundEvent)
    {
        auto* pSoundDll = g_pSysInterface->GetSoundDll();
        if (!pSoundDll)
            return 0;

        if (m_rSpeechSound)
        {
            if (g_pEngineData->SRefToPtr(m_rSpeechSound))
            {
                StopPoseAnim(0, true);
                pSoundDll->DeleteSnd(m_rSpeechSound);
            }
            m_rSpeechSound = 0;
        }

        if (!lSoundIndex)
            return 0;

        ZMat3x3 headMat;
        ZVector3 headPos;
        GetIKBoneMatPos(HeadBoneIndex(), headMat, headPos);
        auto* pGroup = BaseGeom()->ParentGroup();
        if (!pGroup)
            return 0;

        m_rSpeechSound = pGroup->AddSound3d(headMat.data, headPos.Get(), lSoundIndex, lSoundEvent, GetRef(), 0);
        if (auto* pSound = g_pEngineData->SRefToPtr(m_rSpeechSound))
        {
            pSound->SetPitch(static_cast<int>(m_fActorPitch));
            pSound->AddNotifyStarted(GetRef());
            pSound->AddNotifyTarget(GetRef());
        }
        return m_rSpeechSound;
    }

    SREF ZLNKWHANDS::PlayFootStep(EFootSide PlayFootStep)
    {
        auto* pSoundDll = g_pSysInterface->GetSoundDll();
        if (!pSoundDll)
            return 0;

        ZMat3x3 footMat;
        ZVector3 footPos;
        if (PlayFootStep == CENTER)
        {
            EmitFootDustParticle(static_cast<float>(g_pSysInterface->FrameTime), -1);
            GetMatPos(footMat, footPos);
        }
        else
        {
            GetIKBoneMatPos(PlayFootStep == LEFT ? LToeBoneIndex() : RToeBoneIndex(), footMat, footPos);
        }

        vaddscalar(footPos.Get(), footPos.Get(), footMat.ZAxis().Get(), 10.0f);
        if (!m_SoundMappingMaterial || m_MaterialProperty_SoundMaterial.m_Value <= 0)
            return 0;

        const int soundMaterial = BS_Runtime::ZMaterialDescriptionDB::Instance().GetAudioResourceProperty(
            m_ContactMaterialDescID, m_MaterialProperty_SoundMaterial, TEnumID{0});
        int soundIndex = static_cast<int>(pSoundDll->GetMapping(m_SoundMappingMaterial, soundMaterial));
        if (!soundIndex)
            return 0;

        int soundEvent = m_lFootStepEvent;
        if (!m_bFootStepSoundsEnabled) soundIndex = 0;
        if (!m_bFootStepEventsEnabled) soundEvent = 0;

        auto* pGroup = BaseGeom()->ParentGroup();
        return pGroup ? pGroup->AddSound3d(footMat.data, footPos.Get(), soundIndex, soundEvent, GetRef(), 0) : 0;
    }

    bool ZLNKWHANDS::IsFirstPersonCamera() const
    {
        return false;
    }

    float ZLNKWHANDS::GetCombatStrength()
    {
        return 0.0f;
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        static RTP::ZDataProperty<ZAnimVariationHandle> NamespaceItem_1790 {
            .m_Node = { nullptr, "m_pReloadGunOneHand", 2 },
            .m_VirtualTable = &RTP::VirtualTables::Data_ZAnimVariationHandle,
            .m_Offset = CLASS_PROPERTY(ZLNKWHANDS, m_pReloadGunOneHand)
        };

        static RTP::ZDataProperty<ZAnimVariationHandle> NamespaceItem_1789 {
            .m_Node = { NamespaceItem_1790, "m_pReloadRPG", 2 },
            .m_VirtualTable = &RTP::VirtualTables::Data_ZAnimVariationHandle,
            .m_Offset = CLASS_PROPERTY(ZLNKWHANDS, m_pReloadRPG)
        };

        static RTP::ZDataProperty<ZAnimVariationHandle> NamespaceItem_1788 {
            .m_Node = { NamespaceItem_1789, "m_pReloadPumpgun", 2 },
            .m_VirtualTable = &RTP::VirtualTables::Data_ZAnimVariationHandle,
            .m_Offset = CLASS_PROPERTY(ZLNKWHANDS, m_pReloadPumpgun)
        };

        static RTP::ZDataProperty<ZAnimVariationHandle> NamespaceItem_1787 {
            .m_Node = { NamespaceItem_1788, "m_pReloadShotgun", 2 },
            .m_VirtualTable = &RTP::VirtualTables::Data_ZAnimVariationHandle,
            .m_Offset = CLASS_PROPERTY(ZLNKWHANDS, m_pReloadShotgun)
        };

        static RTP::ZDataProperty<ZAnimVariationHandle> NamespaceItem_1786 {
            .m_Node = { NamespaceItem_1787, "m_pReloadSubMachineGunRight", 2 },
            .m_VirtualTable = &RTP::VirtualTables::Data_ZAnimVariationHandle,
            .m_Offset = CLASS_PROPERTY(ZLNKWHANDS, m_pReloadSubMachineGunRight)
        };

        static RTP::ZDataProperty<ZAnimVariationHandle> NamespaceItem_1785 {
            .m_Node = { NamespaceItem_1786, "m_pReloadRifle", 2 },
            .m_VirtualTable = &RTP::VirtualTables::Data_ZAnimVariationHandle,
            .m_Offset = CLASS_PROPERTY(ZLNKWHANDS, m_pReloadRifle)
        };

        static RTP::ZDataProperty<ZAnimVariationHandle> NamespaceItem_1784 {
            .m_Node = { NamespaceItem_1785, "m_pReloadPistolRight", 2 },
            .m_VirtualTable = &RTP::VirtualTables::Data_ZAnimVariationHandle,
            .m_Offset = CLASS_PROPERTY(ZLNKWHANDS, m_pReloadPistolRight)
        };

        static RTP::ZDataProperty<ZAnimVariationHandle> NamespaceItem_1783 {
            .m_Node = { NamespaceItem_1784, "m_pReloadRevolverRight", 2 },
            .m_VirtualTable = &RTP::VirtualTables::Data_ZAnimVariationHandle,
            .m_Offset = CLASS_PROPERTY(ZLNKWHANDS, m_pReloadRevolverRight)
        };

        static RTP::ZDataProperty<ZAnimVariationHandle> NamespaceItem_1782 {
            .m_Node = { NamespaceItem_1783, "m_pReloadRevolverLeft", 2 },
            .m_VirtualTable = &RTP::VirtualTables::Data_ZAnimVariationHandle,
            .m_Offset = CLASS_PROPERTY(ZLNKWHANDS, m_pReloadRevolverLeft)
        };

        static RTP::ZDataProperty<float> NamespaceItem_1781 {
            .m_Node = { NamespaceItem_1782, "m_fActorPitch", 3 },
            .m_VirtualTable = VirtualTable_DP__11,
            .m_Offset = CLASS_PROPERTY(ZLNKWHANDS, m_fActorPitch)
        };

        static RTP::ZDataProperty<ZSDOwner> NamespaceItem_1780 {
            .m_Node = { NamespaceItem_1781, "m_MaterialDef", 3 },
            .m_VirtualTable = &RTP::VirtualTables::Data_ZSDOwner,
            .m_Offset = CLASS_PROPERTY(ZLNKWHANDS, m_MaterialDef)
        };

        static RTP::ZDataProperty<ZSDOwner> NamespaceItem_1779 {
            .m_Node = { NamespaceItem_1780, "m_SoundDef", 3 },
            .m_VirtualTable = &RTP::VirtualTables::Data_ZSDOwner,
            .m_Offset = CLASS_PROPERTY(ZLNKWHANDS, m_SoundDef)
        };
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZLNKWHANDS,
        ZCTRLIKLNKOBJ,
        0x0099C630,
        "ZLNKWHANDS",
        0x00771574,
        cProperties::NamespaceItem_1779,
        0x00809D94,
        0x0099C5DC,
        0x0099C5E0
    );
#   pragma endregion

    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, ZMSGID, s_msgCutSequenceEnd, 0x0099C5D8, 0);
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, TAudioPropertyID, m_MaterialProperty_SoundContact, 0x0099C6B8, {});
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, TAudioPropertyID, m_MaterialProperty_SoundMaterial, 0x0099C6BC, {});
    STATIC_CLASS_VAR_ARRAY_IMPL(ZLNKWHANDS, ZAnimVariationHandle, m_pTalk, 2, 0x007FC9D0);
    STATIC_CLASS_VAR_ARRAY_IMPL(ZLNKWHANDS, ZAnimVariationHandle, m_pListen, 2, 0x007FC9CC);
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, ZMessageResolver, m_msgCalcMats, 0x0099C634, {"MSG_CALCMATS"});
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, ZMessageResolver, m_msgDropItem, 0x0099C640, {"MSG_DROPITEM"});
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, ZMessageResolver, m_msgUseItem, 0x0099C64C, {"MSG_USEITEM"});
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, ZMessageResolver, m_msgCanPickupItem, 0x0099C664, {"MSG_CANPICKUPITEM"});
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, ZMessageResolver, m_msgPickupItem, 0x0099C658, {"MSG_PICKUPITEM"});
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, ZMessageResolver, m_msgReloadWeapon, 0x0099C688, {"MSG_RELOADWEAPON"});
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, ZMessageResolver, m_msgEnterItemRange, 0x0099C670, {"MSG_ENTERITEMRANGE"});
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, ZMessageResolver, m_msgLeaveItemRange, 0x0099C67C, {"MSG_LEAVEITEMRANGE"});
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, ZMessageResolver, m_msgSoundStartet, 0x0099C6AC, {"MSG_SOUNDNOTIFYSTARTED"});
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, ZMessageResolver, m_msgSoundStopped, 0x0099C6A0, {"MSG_SOUNDNOTIFYSTOPPED"});
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, float, m_fDmgMultDefault, 0x007FC984, 1.0f);
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, float, m_fDmgMultLeftArm, 0x007FC988, 2.0f);
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, float, m_fDmgMultRightArm, 0x007FC98C, 2.0f);
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, float, m_fDmgMultLeftLeg, 0x007FC990, 2.0f);
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, float, m_fDmgMultRightLeg, 0x007FC994, 2.0f);
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, float, m_fDmgMultTorso, 0x007FC998, 4.0f);
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, float, m_fDmgMultHead, 0x007FC99C, 10.0f);
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, float, m_fDmgMultFace, 0x007FC9A0, 15.0f);
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, float, m_fDmgMultLeftHand, 0x007FC9A4, 1.0f);
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, float, m_fDmgMultRightHand, 0x007FC9A8, 1.0f);
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, bool, bInitializedStaticAnimsZLNKWHANDS, 0x0099C5E4, false);
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, ZAnimVariationHandle, m_pPullGunRight, 0x007FC9AC, {});
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, ZAnimVariationHandle, m_pPutItemLeft, 0x007FC9B8, {});
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, ZAnimVariationHandle, m_pPutItemRight, 0x007FC9BC, {});
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, ZAnimVariationHandle, m_pPutGunLeft, 0x007FC9B0, {});
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, ZAnimVariationHandle, m_pPutGunRight, 0x007FC9B4, {});
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, ZAnimVariationHandle, m_pDropItemLeft, 0x007FC9C0, {});
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, ZAnimVariationHandle, m_pDropItemRight, 0x007FC9C4, {});
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, ZAnimVariationHandle, m_pAimRHandGuns, 0x007FC9C8, {});
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, ZAnimVariationHandle, m_pChamberGunLeft, 0x007FC9EC, {});
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, ZAnimVariationHandle, m_pChamberGunRight, 0x007FC9E8, {});
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, ZAnimVariationHandle, m_pChamberRifle, 0x007FC9E4, {});
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, ZAnimVariationHandle, m_pChamberShotgun, 0x007FC9E0, {});
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, ZAnimVariationHandle, m_pChamberPumpgun, 0x007FC9DC, {});
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, ZAnimVariationHandle, m_pChamberRPG, 0x007FC9D8, {});
    STATIC_CLASS_VAR_IMPL(ZLNKWHANDS, ZAnimVariationHandle, m_pChamberGrenade, 0x007FC9D4, {});
}
