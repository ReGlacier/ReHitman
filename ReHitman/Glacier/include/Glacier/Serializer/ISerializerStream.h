#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZList.h>
#include <Glacier/ZSTL/ZBitfield.h>
#include <Glacier/Serializer/ZTokenCache.h>
#include <Glacier/Serializer/ZToken.h>
#include <Glacier/Runtime/ZEnum.h>
#include <cstdint>

namespace Glacier
{
	struct ZSerializable;

	struct ISerializerStream
	{
		struct IREFConverter
		{
			IREFConverter() = default;
			
			virtual ZREF GetRef();
			virtual void SetRef(ZREF ref);
		};

		enum EType : int32_t
		{
			TYPE_ILLEGAL = 0xFF,
			TYPE_DataOnly = 0x0,
			TYPE_Tags = 0x1,
			TYPE_Dictionary = 0x2,
			TYPE_TagsAndDictionary = 0x3,
			TYPE_StringEnums = 0x4,
			TYPE_StringTable = 0x8,
			TYPE_LevelFileDebug = 0xF,
			TYPE_LevelFile = 0xD,
			TYPE_LevelFileRelease = 0xC,
			TYPE_SavedGame = 0x8,
		};

		enum EContent : int32_t
		{
			CONTENT_LevelFile = 0x0,
			CONTENT_SavedGame = 0x1,
			CONTENT_SimpleRepack = 0x2,
			CONTENT_ILLEGAL = 0x3,
		};

		enum EPropertyType : int32_t
		{
			PT_RawData = 0x0,
			PT_Char = 0x1,
			PT_Bool = 0x2,
			PT_Int8 = 0x3,
			PT_UInt8 = 0x4,
			PT_Int16 = 0x5,
			PT_UInt16 = 0x6,
			PT_Int32 = 0x7,
			PT_UInt32 = 0x8,
			PT_Float32 = 0x9,
			PT_Float64 = 0xA,
			PT_String = 0xB,
			PT_Enum = 0xC,
			PT_Bitfield = 0xD,
			PT_Object = 0xE,
			PT_Reference = 0xF,
			PT_Container = 0x10,
		};

		// vtbl
		virtual ~ISerializerStream();
		virtual void Skip();
		virtual void End();
		virtual bool IsSaving();
		virtual void ExchangeContainer(const ZToken, unsigned int*);
		virtual void ExchangeRaw(const ZToken, void*, const unsigned int);
		virtual void ExchangeREF(const ZToken, IREFConverter *);		
		virtual void ExchangeReftab(const ZToken, REFTAB*);
		virtual ZToken GetToken(ZToken* result, const char*);
		virtual void BeginArray(const ZToken, const unsigned int);
		virtual void EndArray();
		virtual void ExchangeHeader(const ZToken, const EPropertyType);
		virtual void ExchangeFooter(const EPropertyType);
		virtual void ExchangeData(ZSerializable*);
		virtual void ExchangeData(ZBitfieldBase *, const ZEnumInfo *);
		virtual void ExchangeData(void *, const ZEnumInfo *);
		virtual void ExchangeData(zstring *);
		virtual void ExchangeData(const char **);
		virtual void ExchangeData(int64_t *);
		virtual void ExchangeData(float *);
		virtual void ExchangeData(uint8_t *);
		virtual void ExchangeData(int8_t *);
		virtual void ExchangeData(uint16_t *);
		virtual void ExchangeData(int16_t *);
		virtual void ExchangeData(uint32_t *);
		virtual void ExchangeData(int32_t *);
		virtual void ExchangeData(bool *);

		// data
		EType m_Type;
		EContent m_Content;
		bool m_Status;
		bool m_Finished;
		RE_ADD_PADDING(2);
		ZList<ZTokenCache, false, 0> m_Cache; //size 0x8
	};
	RE_VERIFY_SIZE(ISerializerStream, 0x18);
}