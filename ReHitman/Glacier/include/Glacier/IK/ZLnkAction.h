#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZPoolAllocRefTab.h>
#include <Glacier/GlacierFWD.h>
#include <cstdint>


namespace Glacier
{
	class ZLnkAction
	{
	public:
	    // static
		static uint8_t s_LinkActionBuffer[0x8000];
		static ZPoolAllocator s_LinkActionPool;

		// vtbl
		virtual ~ZLnkAction();

		virtual uint32_t GetActionId() const;
		virtual bool Execute();
		virtual bool CallBack();
		virtual bool Update();
		virtual void Terminate();
		virtual void Save(struct ZSaveGame*);
		virtual void Load(struct ZLoadGame*);
		virtual const char* Name() const;
		virtual bool DeleteOnExit() const;
		virtual bool AnimEnd(Animation::ActiveAnimation* pAnim);
		virtual bool OnMetaKey(Animation::ActiveAnimation* pAnim, Animation::ZMetaKey* pMetaData, const char* pMetaDataString);
		virtual bool SupportsLoadSave();
		virtual void LoadSave(ISerializerStream& stream, bool bSaving);

		// methods
		ZLnkAction(uint32_t lActionId);

		// members
		uint32_t m_lActionId;
	};
	RE_VERIFY_SIZE(ZLnkAction, 0x8);
}
