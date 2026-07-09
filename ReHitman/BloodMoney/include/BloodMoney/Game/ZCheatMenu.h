#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <BloodMoney/Game/UI/CMenuWindow.h>

namespace Hitman::BloodMoney
{
	class ZFRAME;
	class ZCHAROBJ;

	enum ECheatType
	{
		ct_btoggle = 0,
		ct_ftoggle = 1,
		ct_fvalue = 2,
		ct_fivalue = 3,
		ct_oneshot = 4,
		ct_console = 5, // PS2 only ; Maybe in Hitman Contracts still in use?
		ct_ivalue = 6,
		ct_uvartoggle = 7,
		ct_timemult = 8,
	};

	class ZCheatMenu : public CMenuWindow
	{
	public:
		//types
		struct SCheat
		{
			ECheatType eCheatType;
			const char* szDisplayName;
			union 
			{
				const char* szCommand;
				float* pFloat;
				bool* pBool;
				int32_t* pInt;
				void* pUvarInt;
				void        (__cdecl* pFn)(); 
				uint32_t    _Dummy;
			};
		};
		RE_VERIFY_SIZE(SCheat, 0xC);

		//vtbl (no changes)
		//data (total size is 0x78, base size is 0x30)
		ZCHAROBJ* m_pText;
		ZFRAME* m_pFrame1;
		ZFRAME* m_pFrame2;
		int32_t m_iSelected;
		Glacier::ZMSGID m_msgOpen;
		int32_t m_iExitCountdown;
		Glacier::REFTAB m_rtCommands;
	};
	RE_VERIFY_SIZE(ZCheatMenu, 0x78);
}