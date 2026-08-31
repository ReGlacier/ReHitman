#include <Glacier/IK/ZLnkActionQueue.h>
#include <Glacier/IK/ZLnkAction.h>
#include <Glacier/IK/ZIKLNKOBJ.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/ZSTL/LINKREFTAB.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZLnkActionQueue::ZLnkActionQueue(ZIKLNKOBJ* pActor)
        : ZActionDispatcher(pActor)
        , m_pActionQueue(nullptr)
    {
    }

    ZLnkActionQueue::~ZLnkActionQueue()
    {
        // Do nothing?
    }

	bool ZLnkActionQueue::Execute(ZLnkAction* pAction)
	{
	    ZASSERT(pAction);
		if (!m_pActionQueue)
		{
		    m_pActionQueue = ZUniMemory::New<LINKREFTAB>(4, 0);
		}

		m_pActionQueue->Add(reinterpret_cast<uint32_t>(pAction));
	    return true;
	}

	void ZLnkActionQueue::Update()
	{
	    if (!m_pCurrentAction)
			DispatchNextAction();

		ZActionDispatcher::Update();
	}

	bool ZLnkActionQueue::DispatchNextAction()
	{
	    if (m_pActionQueue && m_pActionQueue->Count())
		{
		    ZLnkAction* pAction = *m_pActionQueue->As<ZLnkAction*>().begin();

			m_pActor->m_Active |= pAction->GetActionId();
			return ZActionDispatcher::Execute(pAction);
		}

		return false;
	}

	void ZLnkActionQueue::RemoveCurrentAction()
	{
	    if (m_pCurrentAction)
		{
		    RemoveAction(m_pCurrentAction);
			m_pCurrentAction = nullptr;
		}
	}

	void ZLnkActionQueue::RemoveAction(ZLnkAction* pAction)
	{
	    if (!m_pActionQueue)
			return;

		auto actions = m_pActionQueue->As<ZLnkAction*>();
		auto it = actions.begin();
		if (it != actions.end() && pAction == *it)
		{
		    pAction->Terminate();
			m_pActor->m_Active &= ~pAction->GetActionId();
			it.Erase();
			ZUniMemory::Delete(pAction);
		}
	}

	bool ZLnkActionQueue::CanSaveNow() const
	{
	    if (m_pActionQueue && m_pActionQueue->Count())
		{
		    for (auto* pAction : m_pActionQueue->As<ZLnkAction*>())
			{
			    if (!pAction || !pAction->SupportsLoadSave())
					return false;
			}
		}
		else if (!m_pCurrentAction)
		{
		    return true;
		}

		return !m_pCurrentAction || m_pCurrentAction->SupportsLoadSave();
	}

	void ZLnkActionQueue::LoadSave(ISerializerStream& stream, bool bSaving)
	{
	    ZASSERT(CanSaveNow());

		bool bCurrentActionIsNull = (m_pCurrentAction == nullptr);
		stream.Exchange("currentActionIsNULL", bCurrentActionIsNull);

		int32_t count = 0;
		if (m_pActionQueue)
		{
		    count = m_pActionQueue->Count();
		}
		stream.Exchange("count", count);

		if (bSaving)
		{
		    if (m_pActionQueue)
			{
			    for (auto* pAction : m_pActionQueue->As<ZLnkAction*>())
				{
				    if (pAction)
						LoadSaveLnkAction(stream, bSaving, &pAction);
				}
			}
		}
		else
		{
		    for (int32_t i = 0; i < count; ++i)
			{
			    ZLnkAction* pAction = nullptr;
				LoadSaveLnkAction(stream, bSaving, &pAction);
				ZASSERT(pAction);

				if (!m_pActionQueue)
				{
				    m_pActionQueue = ZUniMemory::New<LINKREFTAB>(4, 0);
				}

				m_pActionQueue->Add(reinterpret_cast<uint32_t>(pAction));
			}
		}

		if (bCurrentActionIsNull)
		{
		    m_pCurrentAction = nullptr;
		}
		else
		{
		    ZASSERT(m_pActionQueue);
			ZASSERT(m_pActionQueue->Count() > 0);

			m_pCurrentAction = *m_pActionQueue->As<ZLnkAction*>().begin();
		}
	}

	bool ZLnkActionQueue::CheckActionQueueForId(uint32_t rActionId)
	{
	    if (!m_pActionQueue || !m_pActionQueue->Count())
			return false;

		for (auto* pAction : m_pActionQueue->As<ZLnkAction*>())
		{
		    if (pAction && pAction->GetActionId() == rActionId)
			{
			    return true;
			}
		}

	    return false;
	}
}
