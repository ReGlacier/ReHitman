#include <Glacier/IK/ZActionDispatcher.h>
#include <Glacier/IK/ZLnkAction.h>
#include <Glacier/IK/ZIKLNKOBJ.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZActionDispatcher::ZActionDispatcher(ZIKLNKOBJ* pActor)
        : m_pCurrentAction(nullptr)
        , m_pActor(pActor)
    {
    }

    ZActionDispatcher::~ZActionDispatcher()
    {
        if (m_pCurrentAction && m_pCurrentAction->DeleteOnExit())
        {
            ZUniMemory::Delete(m_pCurrentAction);
        }

        m_pCurrentAction = nullptr;
    }

	bool ZActionDispatcher::Execute(ZLnkAction* pAction)
	{
	    ZASSERT(!m_pCurrentAction);
	    m_pCurrentAction = pAction;

	    if (pAction->Execute())
	    {
			return true;
		}

	    RemoveCurrentAction();
	    return false;
	}

	void ZActionDispatcher::Update()
	{
	    if (m_pCurrentAction && !m_pCurrentAction->Update())
	        RemoveCurrentAction();
	}

	void ZActionDispatcher::CallBack()
	{
	    if (m_pCurrentAction && !m_pCurrentAction->CallBack())
	        RemoveCurrentAction();
	}

	void ZActionDispatcher::OnMetaKey(Animation::ActiveAnimation* pBoneAnim, Animation::ZMetaKey* MetaData, const char* pszMetaDataString)
	{
	    if (m_pCurrentAction && !m_pCurrentAction->OnMetaKey(pBoneAnim, MetaData, pszMetaDataString))
	        RemoveCurrentAction();
	}

	void ZActionDispatcher::DropAction()
	{
	    if (m_pCurrentAction)
	        RemoveCurrentAction();
	}

	void ZActionDispatcher::RemoveCurrentAction()
	{
	    ZASSERT(m_pCurrentAction);

	    ZLnkAction* pAction = m_pCurrentAction;
	    m_pCurrentAction = nullptr;

	    pAction->Terminate();

	    if (pAction->DeleteOnExit())
	        ZUniMemory::Delete(pAction);
	}

	void ZActionDispatcher::AnimEnd(Animation::ActiveAnimation* pBoneAnim)
	{
	    if (m_pCurrentAction && m_pCurrentAction->AnimEnd(pBoneAnim))
	        RemoveCurrentAction();
	}

	void ZActionDispatcher::LoadSave(ISerializerStream& stream, bool bSaving)
	{
	    bool bHasAnAction = m_pCurrentAction != nullptr;
	    if (bSaving && m_pCurrentAction && !m_pCurrentAction->SupportsLoadSave())
	        bHasAnAction = false;

	    stream.Exchange("hasAnAction", bHasAnAction);

	    if (bHasAnAction)
		{
		    LoadSaveLnkAction(stream, bSaving, &m_pCurrentAction);
		}
	}

	void ZActionDispatcher::LoadSaveLnkAction(ISerializerStream& stream, bool bSaving, ZLnkAction** ppAction)
	{
	    if (bSaving)
	    {
	        ZASSERT((*ppAction)->SupportsLoadSave());

	        uint32_t actionId = (*ppAction)->GetActionId();
	        stream.Exchange("actionID", actionId);
	    }
	    else
	    {
	        uint32_t actionId = 0;
	        stream.Exchange("actionID", actionId);

	        *ppAction = m_pActor->CreateLnkAction(actionId);
	        ZASSERT(*ppAction);
	        ZASSERT((*ppAction)->GetActionId() == actionId);
	    }

	    if (*ppAction)
	        (*ppAction)->LoadSave(stream, bSaving);
	}
}
