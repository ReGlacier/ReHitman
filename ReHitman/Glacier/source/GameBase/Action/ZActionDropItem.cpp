#include <Glacier/GameBase/Action/ZActionDropItem.h>
#include <Glacier/Animation/Header.h>
#include <Glacier/IK/ZIKHAND.h>
#include <Glacier/IK/ZLNKWHANDS.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/ZUniAssert.h>
#include <cstring>

namespace Glacier
{
    ZActionDropItem::ZActionDropItem(uint32_t actionId, ZLNKWHANDS* pActor, ZIKHAND* pHand, Animation::Header* pAnimDropItem)
        : ZLnkAction(actionId),
          m_pActor(pActor),
          m_pHand(pHand),
          m_pAnimDropItem(pAnimDropItem)
    {
        ZASSERT(m_pHand);
        ZASSERT(m_pActor);
    }

    ZActionDropItem::~ZActionDropItem() = default;

    bool ZActionDropItem::Execute()
    {
        if (!m_pHand->GetItem() || !m_pAnimDropItem)
            return false;

        return m_pActor->ActivateAnim(m_pAnimDropItem, 1) != nullptr;
    }

    bool ZActionDropItem::CallBack()
    {
        m_pHand->SlipItem(m_pActor);
        return false;
    }

    const char* ZActionDropItem::Name() const
    {
        return "ZActionDropItem";
    }

    bool ZActionDropItem::SupportsLoadSave()
    {
        return (m_lActionId == 9 || m_lActionId == 10) && !strcmp(Name(), "ZActionDropItem");
    }

    void ZActionDropItem::LoadSave(ISerializerStream& stream, bool bSaving)
    {
        ZLnkAction::LoadSave(stream, bSaving);
        stream.Exchange("pActor", m_pActor);
        int32_t handId = m_pHand == &m_pActor->m_LHand ? 1 : 2;
        stream.Exchange("handID", handId);
        m_pHand = handId == 1 ? &m_pActor->m_LHand : &m_pActor->m_RHand;
        stream.Exchange("m_pAnimDropItem", m_pAnimDropItem);
    }

    ZIKHAND* ZActionDropItem::GetHand() const
    {
        return m_pHand;
    }
}
