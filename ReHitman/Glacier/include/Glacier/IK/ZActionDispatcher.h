#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Glacier.h> // Animation::* fwds


namespace Glacier
{
	class ZLnkAction;
	class ZIKLNKOBJ;

	class ZActionDispatcher
	{
	public:
	    // vtbl
		virtual ~ZActionDispatcher();
		virtual bool Execute(ZLnkAction* pAction);
		virtual void Update();
		virtual void CallBack();
		virtual void OnMetaKey(Animation::ActiveAnimation* pBoneAnim, Animation::ZMetaKey* MetaData, const char* pszMetaDataString);
		virtual void DropAction();
		virtual void RemoveCurrentAction();
		virtual void AnimEnd(Animation::ActiveAnimation* pBoneAnim);
		virtual void LoadSave(ISerializerStream& stream, bool bSaving);

		// methods
		ZActionDispatcher(ZIKLNKOBJ* pActor);

		// members
		ZLnkAction* m_pCurrentAction;
		ZIKLNKOBJ* m_pActor;

	protected:
		void LoadSaveLnkAction(ISerializerStream& stream, bool bSaving, ZLnkAction** ppAction);
	};
	RE_VERIFY_SIZE(ZActionDispatcher, 0xC); // Verified
}
