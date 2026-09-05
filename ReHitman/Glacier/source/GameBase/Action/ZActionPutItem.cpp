#include <Glacier/GameBase/Action/ZActionPutItem.h>
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
    ZActionPutItem::ZActionPutItem(uint32_t actionId, ZLNKWHANDS* pActor, ZIKHAND* pHand, Animation::Header* pAnimPutItem, Animation::Header* pAnimPutWeapon, bool bMirrorLHandAnims)
        : ZLnkAction(actionId),
          m_pActor(pActor),
          m_pHand(pHand),
          m_pAnimPutItem(pAnimPutItem),
          m_pAnimPutWeapon(pAnimPutWeapon),
          m_bMirrorLHandAnims(bMirrorLHandAnims),
          m_fHideItemFrame(0.f),
          m_pBoneAnim(nullptr)
    {
        ZASSERT(m_pHand);
        ZASSERT(m_pActor);
    }

    ZActionPutItem::~ZActionPutItem() = default;

    bool ZActionPutItem::Execute()
    {
        ZItem* pItem = m_pHand->GetItem();
        ZASSERT(pItem);
        ZASSERT(pItem->GetItemTemplate());

        Animation::Header* pAnim = pItem->GetItemTemplate()->GetItemHands() == ITEMHANDS::IH_ONEHANDED
            ? m_pAnimPutWeapon
            : m_pAnimPutItem;
        if (!pAnim)
            return ZLnkAction::CallBack();

        m_pBoneAnim = m_pActor->ActivateAnim(pAnim, m_bMirrorLHandAnims ? 0x8001 : 1);
        if (!m_pBoneAnim)
            return ZLnkAction::CallBack();

        m_fHideItemFrame = 11.f;
        return true;
    }

    bool ZActionPutItem::CallBack()
    {
        ZASSERT(m_pHand->GetItem());
        m_pHand->SlipItem(m_pActor);
        return false;
    }

    bool ZActionPutItem::Update()
    {
        if (m_fHideItemFrame != 0.f && m_pBoneAnim && m_pBoneAnim->frame >= m_fHideItemFrame)
        {
            m_fHideItemFrame = 0.f;
            if (ZItem* pItem = m_pHand->GetItem())
                pItem->SetState(ITEMSTATE::IS_HIDE, nullptr);
        }
        return true;
    }

    const char* ZActionPutItem::Name() const
    {
        return "ZActionPutItem";
    }

    bool ZActionPutItem::SupportsLoadSave()
    {
        return (m_lActionId == 5 || m_lActionId == 6) && !strcmp(Name(), "ZActionPutItem");
    }

    void ZActionPutItem::LoadSave(ISerializerStream& stream, bool bSaving)
    {
        ZLnkAction::LoadSave(stream, bSaving);
        stream.Exchange("pActor", m_pActor);
        int32_t handId = m_pHand == &m_pActor->m_LHand ? 1 : 2;
        stream.Exchange("handID", handId);
        m_pHand = handId == 1 ? &m_pActor->m_LHand : &m_pActor->m_RHand;
        stream.Exchange("m_pAnimPutItem", m_pAnimPutItem);
        stream.Exchange("m_pAnimPutWeapon", m_pAnimPutWeapon);
        stream.Exchange("m_bMirrorLHandAnims", m_bMirrorLHandAnims);
        stream.Exchange("m_fHideItemFrame", m_fHideItemFrame);
        int32_t activeAnimIndex = -1;
        if (bSaving && m_pBoneAnim)
            activeAnimIndex = m_pBoneAnim->sequenceId;
        stream.Exchange("activeAnimIndex", activeAnimIndex);
        if (!bSaving)
            m_pBoneAnim = activeAnimIndex == -1 ? nullptr : m_pActor->IsAnimationRunning(activeAnimIndex);
    }

    ZIKHAND* ZActionPutItem::GetHand() const
    {
        return m_pHand;
    }
}
