#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Glacier.h> // Animation::* fwds


namespace Glacier
{
	struct ZLnkAction;
	struct ZIKLNKOBJ;

	struct ZActionDispatcher
	{
		virtual ~ZActionDispatcher();
		virtual bool Execute(ZLnkAction*);
		virtual void Update();
		virtual void CallBack();
		virtual void OnMetaKey(Animation::ActiveAnimation*, Animation::ZMetaKey*, const char*);
		virtual void DropAction();
		virtual void RemoveCurrentAction();
		virtual void AnimEnd(int);
		virtual void LoadSave(ISerializerStream&, bool);

		ZLnkAction* m_pCurrentAction;
		ZIKLNKOBJ* m_pActor;
	};

	RE_VERIFY_SIZE(ZActionDispatcher, 0xC);
}