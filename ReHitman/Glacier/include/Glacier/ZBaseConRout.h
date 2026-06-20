#pragma once


#include <Glacier/EventBase/ZEventBase.h>
#include <Glacier/ZSTL/ZOldTypeInfo.h>
#include <Glacier/ReGlacier.h>


namespace Glacier
{
	struct ZBaseConRoutTypeInfo
	{
		char *m_psName; //0x0000
		uint32_t m_unk4; //0x0004
		uint32_t m_unk8; //0x0008
		uint32_t m_unkC; //0x000C
		uint32_t m_unk10; //0x0010
		uint32_t m_unk14; //0x0014
		uint32_t m_unk18; //0x0018
		uint32_t m_unk1C; //0x001C
		char *m_psClassName; //0x0020
		char *m_psParentName; //0x0024
		uint32_t m_unk28; //0x0028
		uint32_t m_unk2C; //0x002C
		uint32_t m_unk30; //0x0030
		uint32_t m_unk34; //0x0034
		uint32_t m_pProduce; //0x0038
		struct ZBaseConRoutTypeInfo *m_pNext; //0x003C
		uint32_t m_unk40; //0x0040
		uint32_t m_unk44; //0x0044
		uint32_t m_unk48; //0x0048
		uint32_t m_unk4C; //0x004C
	}; //Size: 0x0050

	struct ZCLASSINFO
	{
		const char* m_szClassInfoName;
		int m_iClassInfoType;
		int m_lSceneInstanceCount;
		struct ZCLASSINFO * Parent;
		struct ZCLASSINFO * Prev;
		struct ZCLASSINFO * Next;
	};
	RE_VERIFY_SIZE(ZCLASSINFO, 0x18);

	struct ZNonResourceClassInfo : public ZCLASSINFO
	{
		uint32_t m_lSize;
		bool m_bResourceLinked;
		RE_ADD_PADDING(3);
	};
	RE_VERIFY_SIZE(ZNonResourceClassInfo, 0x20);

	struct ZGEOMCLASINFO : public ZNonResourceClassInfo
	{
		void(* m_pSetTypeIDAndMask)(unsigned int, unsigned int);
		unsigned int m_lType;
		unsigned int m_lGeomCases;
		const char* m_szParentClass;
		unsigned int* m_pClassId;
		unsigned int* m_pMaskId;
		unsigned __int16 m_iClassInfoNr;
		RE_ADD_PADDING(2);
	};
	RE_VERIFY_SIZE(ZGEOMCLASINFO, 0x3C);

	class ZROUTCLASSINFO : public ZNonResourceClassInfo
	{
		const char* m_szRoutName;
		const char* m_szGeomName;
		int m_lPrio;
		unsigned int m_lRoutCases;
		uint16_t m_iClassInfoNr;
		RE_ADD_PADDING(2);
	};
	RE_VERIFY_SIZE(ZROUTCLASSINFO, 0x34);

	class ZBaseConRout : public ZEventBase
	{
	public:
		// Virtual methods
		virtual int InitBaseConRout(Glacier::ZROUTCLASSINFO*);
		virtual void UnknownCommand(Glacier::ZMSGID command, Glacier::ZDATA data);

		// Internal API
		// Static
		static ZBaseConRoutTypeInfo** GetFactory();

		// Members
		ZROUTCLASSINFO* m_pRoutClassInfo;
	};
	RE_VERIFY_SIZE(ZBaseConRout, 0x30);
	RE_VERIFY_OFFSET(ZBaseConRout, m_pRoutClassInfo, 0x2C);
}