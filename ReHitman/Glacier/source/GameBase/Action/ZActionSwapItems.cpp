#include <Glacier/GameBase/Action/ZActionSwapItems.h>
#include <Glacier/Animation/Header.h>
#include <Glacier/IK/ZIKHAND.h>
#include <Glacier/IK/ZLNKWHANDS.h>
#include <Glacier/Items/ZItem.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/ZUniAssert.h>
#include <cstring>

namespace Glacier
{
    ZActionSwapItems::ZActionSwapItems(uint32_t actionId, ZLNKWHANDS* pActor, ZIKHAND* pRHand, ZIKHAND* pLHand, Animation::Header* pAnimSwapItems)
        : ZLnkAction(actionId),
          m_pActor(pActor),
          m_pRHand(pRHand),
          m_pLHand(pLHand),
          m_pAnimSwapItems(pAnimSwapItems)
    {
        ZASSERT(m_pActor);
        ZASSERT(m_pRHand);
        ZASSERT(m_pLHand);
    }

    ZActionSwapItems::~ZActionSwapItems() = default;

    bool ZActionSwapItems::Execute()
    {
        if (!m_pAnimSwapItems)
            return ZLnkAction::CallBack();

        m_pActor->ActivateAnim(m_pAnimSwapItems, 1);
        return true;
    }

    bool ZActionSwapItems::CallBack()
    {
        ZItem* pRItem = m_pRHand->GetItem();
        ZItem* pLItem = m_pLHand->GetItem();
        m_pRHand->SlipItem(m_pActor);
        m_pLHand->SlipItem(m_pActor);
        if (pRItem)
            m_pLHand->AttachItem(m_pActor, pRItem->GetRef());
        if (pLItem)
            m_pRHand->AttachItem(m_pActor, pLItem->GetRef());
        return false;
    }

    const char* ZActionSwapItems::Name() const
    {
        return "ZActionSwapItem";
    }

    bool ZActionSwapItems::SupportsLoadSave()
    {
        return m_lActionId == 11 && !strcmp(Name(), "ZActionSwapItem");
    }

    void ZActionSwapItems::LoadSave(ISerializerStream& stream, bool bSaving)
    {
        ZLnkAction::LoadSave(stream, bSaving);
        stream.Exchange("pActor", m_pActor);
        int32_t rightHandId = m_pRHand == &m_pActor->m_LHand ? 1 : 2;
        stream.Exchange("handID", rightHandId);
        m_pRHand = rightHandId == 1 ? &m_pActor->m_LHand : &m_pActor->m_RHand;
        int32_t leftHandId = m_pLHand == &m_pActor->m_LHand ? 1 : 2;
        stream.Exchange("handID", leftHandId);
        m_pLHand = leftHandId == 1 ? &m_pActor->m_LHand : &m_pActor->m_RHand;
        stream.Exchange("m_pAnimSwapItems", m_pAnimSwapItems);
    }

    ZIKHAND* ZActionSwapItems::GetLHand() const
    {
        return m_pLHand;
    }
}
