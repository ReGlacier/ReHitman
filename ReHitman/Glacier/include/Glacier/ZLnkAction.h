#pragma once

#include <Glacier/Glacier.h>
#include <Glacier/ReGlaceir.h>


namespace Glacier
{
	struct ZLnkAction
	{
		// vtbl
		virtual ~ZLnkAction();

		virtual unsigned int GetActionId();
		virtual bool Execute();
		virtual void CallBack();
		virtual void Update();
		virtual void Terminate();
		virtual void Save(struct ZSaveGame*);
		virtual void Load(struct ZLoadGame*);
		virtual const char* Name();
		virtual bool DeleteOnExit();
		virtual bool AnimEnd(Animation::ActiveAnimation* anim);
		virtual bool OnMetaKey(Animation::ActiveAnimation*, Animation::ZMetaKey*, const char*);
		virtual bool SupportsLoadSave();
		virtual void LoadSave(ISerializerStream& stream, bool flag);

		// members
		unsigned int m_lActionId;
	};
	RE_VERIFY_SIZE(ZLnkAction, 0x8);
}