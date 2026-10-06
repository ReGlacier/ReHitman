#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Runtime/ZNonResourceClassInfo.h>

namespace Glacier
{
	struct ZGEOMCLASSINFO : public ZNonResourceClassInfo
	{
		// types
		using SetTypeIdAndMaskFunc_t = void(*)(uint32_t, uint32_t);

		// methods
		ZGEOMCLASSINFO(const ZGEOMCLASSINFO& copy);
		ZGEOMCLASSINFO(const char *szClassInfoName, uint32_t lSize, const char *szParentClass, uint32_t lType, uint32_t lGeomCases, const char *szParameters, SetTypeIdAndMaskFunc_t pSetter, uint32_t* pClassId, uint32_t* pMaskId);
		~ZGEOMCLASSINFO();

		bool IsDerivedFrom(const ZGEOMCLASSINFO* pOther) const;
		uint32_t GetClassId() const;
		uint32_t GetMaskId() const;
		const char* ParentClass() const;
		void SetTypeIDAndMask(uint32_t lType, uint32_t lMask);
		uint32_t Type() const;

		template <typename T>
		bool IsDerviedFrom() const { return (GetClassId() & T::m_Mask) == T::m_Id; }

		// members
		SetTypeIdAndMaskFunc_t m_pSetTypeIDAndMask;
		uint32_t m_lType;
		uint32_t m_lGeomCases;
		const char* m_szParentClass;
		uint32_t* m_pClassId;
		uint32_t* m_pMaskId;
		uint16_t m_iClassInfoNr;
		RE_ADD_PADDING(2);
	};
	RE_VERIFY_SIZE(ZGEOMCLASSINFO, 0x4C);
}