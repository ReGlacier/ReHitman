#include <Glacier/GameBase/Action/ZActionGetItem.h>
#include <Glacier/Animation/ActiveAnimation.h>
#include <Glacier/Animation/Header.h>
#include <Glacier/IK/ZIKHAND.h>
#include <Glacier/IK/ZLNKWHANDS.h>
#include <Glacier/Items/ZItem.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/ZUniAssert.h>
#include <cstring>

namespace Glacier
{
    ZActionGetItem::ZActionGetItem(uint32_t actionId, ZLNKWHANDS* pActor, ZIKHAND* pHand, Animation::Header* pAnimGetItem, Animation::Header* pAnimGetWeapon, bool bMirrorLHandAnims)
        : ZLnkAction(actionId),
          m_pItem(nullptr),
          m_pActor(pActor),
          m_pHand(pHand),
          m_bMirrorLHandAnims(bMirrorLHandAnims),
          m_pAnimGetItem(pAnimGetItem),
          m_pAnimGetWeapon(pAnimGetWeapon),
          m_pActiveAnim(reinterpret_cast<Animation::ActiveAnimation*>(-1))
    {
        ZASSERT(m_pHand);
        ZASSERT(m_pActor);
    }

    ZActionGetItem::~ZActionGetItem() = default;

    bool ZActionGetItem::Execute()
    {
        if (!m_pItem)
            return false;

        ZASSERT(m_pItem->GetItemTemplate());
        Animation::Header* pAnim = m_pAnimGetItem;
        if (m_pItem->GetItemTemplate()->GetItemHands() == ITEMHANDS::IH_ONEHANDED)
            pAnim = m_pAnimGetWeapon;

        if (!pAnim)
            return ZLnkAction::CallBack();

        int control = 1;
        if (m_pHand == m_pActor->GetRHand() && m_bMirrorLHandAnims)
            control |= 0x8000;

        m_pActiveAnim = m_pActor->ActivateAnim(pAnim, control);
        if (m_pActiveAnim)
        {
            const auto callback = reinterpret_cast<Animation::ActiveAnimation::CallBack_t>(
                &ZIKLNKOBJ::CallBackLnkActionBone);
            m_pActiveAnim->AddCallBack(12.f, callback, 0, 0, 0);
            return true;
        }

        m_pActiveAnim = reinterpret_cast<Animation::ActiveAnimation*>(-1);
        return true;
    }

    bool ZActionGetItem::CallBack()
    {
        ZASSERT(m_pItem);
        m_pHand->AttachItem(m_pActor, m_pItem->GetRef());
        return true;
    }

    bool ZActionGetItem::AnimEnd(Animation::ActiveAnimation* pAnim)
    {
        return m_pActiveAnim == pAnim;
    }

    const char* ZActionGetItem::Name() const
    {
        return "ZActionGetItem";
    }

    bool ZActionGetItem::SupportsLoadSave()
    {
        return (m_lActionId == 7 || m_lActionId == 8) && !strcmp(Name(), "ZActionGetItem");
    }

    void ZActionGetItem::LoadSave(ISerializerStream& stream, bool bSaving)
    {
        ZLnkAction::LoadSave(stream, bSaving);
        stream.Exchange("pActor", m_pActor);
        int32_t handId = m_pHand == &m_pActor->m_LHand ? 1 : 2;
        stream.Exchange("handID", handId);
        m_pHand = handId == 1 ? &m_pActor->m_LHand : &m_pActor->m_RHand;
        stream.Exchange("pItem", m_pItem);
        stream.Exchange("m_bMirrorLHandAnims", m_bMirrorLHandAnims);
        stream.Exchange("m_pAnimGetItem", m_pAnimGetItem);
        stream.Exchange("m_pAnimGetWeapon", m_pAnimGetWeapon);
        uint32_t sequenceId = m_pActiveAnim == reinterpret_cast<Animation::ActiveAnimation*>(-1)
            ? static_cast<uint32_t>(-1)
            : static_cast<uint32_t>(m_pActiveAnim->sequenceId);
        stream.Exchange("m_SequenceID", sequenceId);
        if (!bSaving)
            m_pActiveAnim = sequenceId == static_cast<uint32_t>(-1) ? reinterpret_cast<Animation::ActiveAnimation*>(-1)
                                                                    : m_pActor->IsAnimationRunning(sequenceId);
    }

    void ZActionGetItem::SetItem(ZItem* pItem)
    {
        m_pItem = pItem;
    }

    ZItem* ZActionGetItem::GetItem() const
    {
        return m_pItem;
    }
}
