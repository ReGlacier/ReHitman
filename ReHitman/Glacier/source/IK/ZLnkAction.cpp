#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/IK/ZLnkAction.h>


namespace Glacier
{
    ZLnkAction::ZLnkAction(uint32_t lActionId)
	    : m_lActionId(lActionId)
	{
	}

    ZLnkAction::~ZLnkAction()
    {
        // Do nothing
    }

    uint32_t ZLnkAction::GetActionId() const
    {
        return m_lActionId;
    }

	bool ZLnkAction::Execute()
	{
		return false;
	}

	bool ZLnkAction::CallBack()
	{
		return true;
	}

	bool ZLnkAction::Update()
	{
		return true;
	}

	void ZLnkAction::Terminate()
	{
	    // Do nothing
	}

	void ZLnkAction::Save(struct ZSaveGame*)
	{
	    // Do nothing
	}

	void ZLnkAction::Load(struct ZLoadGame*)
	{
	    // Do nothing
	}

	const char* ZLnkAction::Name() const
	{
	    return "N/A";
	}

	bool ZLnkAction::DeleteOnExit() const
	{
	    return true;
	}

	bool ZLnkAction::AnimEnd(Animation::ActiveAnimation* pAnim)
	{
		return false;
	}

	bool ZLnkAction::OnMetaKey(Animation::ActiveAnimation* pAnim, Animation::ZMetaKey* pMetaData, const char* pMetaDataString)
	{
		return true;
	}

	bool ZLnkAction::SupportsLoadSave()
	{
		return false;
	}

	void ZLnkAction::LoadSave(ISerializerStream& stream, bool bSaving)
	{
	    stream.Exchange("m_lActionId", m_lActionId);
	}

	uint8_t ZLnkAction::s_LinkActionBuffer[0x8000] { 0 }; // PC 0097BF08
	ZPoolAllocator ZLnkAction::s_LinkActionPool { (char*)s_LinkActionBuffer, 0x8000u, "ZLnkAction::s_LinkActionPool", false }; // PC 00983F08
}
