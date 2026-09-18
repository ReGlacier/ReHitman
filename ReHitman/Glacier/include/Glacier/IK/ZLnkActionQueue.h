#pragma once

#include <Glacier/IK/ZActionDispatcher.h>


namespace Glacier
{
	class LINKREFTAB;

	class ZLnkActionQueue : public ZActionDispatcher
	{
	public:
	    // vtbl
		~ZLnkActionQueue() override;
		bool Execute(ZLnkAction* pAction) override;
		void Update() override;
		void RemoveCurrentAction() override;
		void LoadSave(ISerializerStream& stream, bool bSaving) override;
		virtual bool CheckActionQueueForId(uint32_t rActionId);

		// methods
		ZLnkActionQueue(ZIKLNKOBJ* pActor);
		bool DispatchNextAction();

	private:
		bool CanSaveNow() const;
		void RemoveAction(ZLnkAction* pAction);

	public:
		// members
		LINKREFTAB* m_pActionQueue;
	};
	RE_VERIFY_SIZE(ZLnkActionQueue, 0x10);
}
