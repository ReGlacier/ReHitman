#include <Glacier/GameBase/Action/ZActionPickupItem.h>
#include <Glacier/Animation/Header.h>
#include <Glacier/Items/ZItem.h>
#include <Glacier/IK/ZLNKWHANDS.h>
#include <Glacier/IK/ZIKHAND.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <cstring>


namespace Glacier
{
    ZActionPickupItem::ZActionPickupItem(uint32_t actionId, ZLNKWHANDS* pActor, ZIKHAND* pHand, Animation::Header* pAnimPickupItem)
        : ZLnkAction(actionId)
        , m_pActor(pActor)
        , m_pItem(nullptr)
        , m_pHand(pHand)
        , m_pAnimPickupItem(pAnimPickupItem)
    {
        ZASSERT(m_pHand);
        ZASSERT(m_pActor);
        ZASSERT(m_pAnimPickupItem);
    }

    ZActionPickupItem::~ZActionPickupItem() = default;

    bool ZActionPickupItem::Execute()
    {
        if (!m_pItem || !m_pAnimPickupItem)
            return false;

        return m_pActor->ActivateAnim(m_pAnimPickupItem, 1) != nullptr;
    }

	bool ZActionPickupItem::CallBack()
	{
	    if (m_pItem)
	        m_pHand->AttachItem(m_pActor, m_pItem->GetRef());
	    return false;
	}

    const char* ZActionPickupItem::Name() const
    {
        return "ZActionPickupItem";
    }

    bool ZActionPickupItem::SupportsLoadSave()
    {
        return (m_lActionId == 3 || m_lActionId == 4) && !strcmp(Name(), "ZActionPickupItem");
    }

    void ZActionPickupItem::LoadSave(ISerializerStream& stream, bool bSaving)
    {
        ZLnkAction::LoadSave(stream, bSaving);
        stream.Exchange("pActor", m_pActor);
        int32_t handId = m_pHand == &m_pActor->m_LHand ? 1 : 2;
        stream.Exchange("handID", handId);
        m_pHand = handId == 1 ? &m_pActor->m_LHand : &m_pActor->m_RHand;
        stream.Exchange("pItem", m_pItem);
        stream.Exchange("m_pAnimPickupItem", m_pAnimPickupItem);
    }

    void ZActionPickupItem::SetItem(ZItem* pItem)
    {
        m_pItem = pItem;
    }

    ZItem* ZActionPickupItem::GetItem() const
    {
        return m_pItem;
    }

    ZLNKWHANDS* ZActionPickupItem::GetActor() const
    {
        return m_pActor;
    }
}
