#pragma once

#include <Glacier/EventBase/ZEventBase.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/GlacierFWD.h>

namespace Hitman::BloodMoney
{
	class ZCheatMenu : public Glacier::ZEventBase
	{
	public:
		//custom entries
		enum ECheatMenuEntryType : int
		{
			MENU_GENERIC_INTVAL_TOGGLE = 0x0, // Int value, 0 or 1 (toggle mode)
			MENU_GENERIC_FLOAT_TOGGLE = 0x1, // Float value 0.0f or 1.0f (toggle mode)
			MENU_GENERIC_FLOAT_EDITABLE = 0x2, // Float value, inc 0.1f
			MENU_GENERIC_FLOAT_READ_ONLY = 0x3, // Float value, read only
			MENU_TOGGLE_CALLABLE_OPTION = 0x4, // pDataPtr should be a pointer to the function
			MENU_TOGGLE_OPTION_2 = 0x5, // I'm not sure that this option works
			MENU_ENGINE_DATABASE_FLOAT_B28 = 0x8, // Always ptr to ZEngineDatabase::m_fTimeMultiplier value!
			MENU_TOGGLE_BOOL = 0x7, // Bool value
		};

		union UEntryOption {
			void(__cdecl *pFunction)(void);
			int* pInt;
			float* pFloat;
			bool* pBool;
		};

		struct SEntry
		{
			ECheatMenuEntryType eType;
			const char* pName;
			UEntryOption uOption;
		};

		//vftable (no changes)

		//data (total size is 0x78, base size is 0x30)
		int m_field30;
		int m_field34;
		int m_field38;
		int m_field3C;
		char field_40;
		char field_41;
		char field_42;
		char field_43;
		ZCHAROBJ* m_pTextObj;
		void *field_48;
		void *field_4C;
		void *field_50;
		__int16 m_MSG_ShowCheatsMenu;
		__int16 field_56;
		void *field_58;
		Glacier::REFTAB m_options;
	};
}