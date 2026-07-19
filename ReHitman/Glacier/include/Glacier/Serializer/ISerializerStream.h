#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZSimpleArray.h>
#include <Glacier/ZSTL/ZBitfield.h>
#include <Glacier/ZSTL/ZList.h>
#include <Glacier/Serializer/ZTokenCache.h>
#include <Glacier/Serializer/ZToken.h>
#include <Glacier/Runtime/ZEnum.h>
#include <cstdint>

namespace Glacier
{
	struct ZSerializable;
	struct ISerializerVisitor;

	struct ISerializerStream
	{
		struct IREFConverter
		{
			IREFConverter() = default;
			
			virtual ZREF GetRef() = 0;
			virtual void SetRef(ZREF ref) = 0;
		};

		template <typename T>
		struct ZREFConverter : public IREFConverter
		{
			// methods
			ZREFConverter(T*& pObject) : m_Object{pObject} 
			{
			}

			// vtbl
			ZREF GetRef() override 
			{
				return T::GetRef(m_Object);
			}

			void SetRef(ZREF ref) override
			{
				m_Object = T::RefToPtr(ref);
			}

			// members
			T*& m_Object;
		};
		// ^ known instances: ZREFConverter<ZGEOM>, ZREFConverter<ZBaseGeom> & ZREFConverter<Animation::Header*>

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
			TYPE_SavedGame = 0x8, // It's correct, but weird. Somebody in IOI fucked up (in Kane & Lynch 2 it still wrong)
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

		// vtbl (since ExchangeData(ZSerializable*) it's flipped in PS2 build)
		virtual ~ISerializerStream();
		virtual void Skip() = 0;
		virtual void End();
		virtual bool IsSaving() const = 0;
		virtual void ExchangeContainer(const ZToken, unsigned int*) = 0;
		virtual void ExchangeRaw(const ZToken, void*, const unsigned int) = 0;
		virtual void ExchangeREF(const ZToken, IREFConverter *) = 0;
		virtual void ExchangeReftab(const ZToken, REFTAB*) = 0;
		virtual ZToken GetToken(const char* psToken) = 0;
		virtual void BeginArray(const ZToken, const unsigned int) = 0;
		virtual void EndArray() = 0;
		virtual void ExchangeHeader(const ZToken, const EPropertyType) = 0;
		virtual void ExchangeFooter(const EPropertyType) = 0;
		virtual void ExchangeData(ZSerializable*) = 0;
		virtual void ExchangeData(ZBitfieldBase * data, const ZEnumInfo * description) = 0;
		virtual void ExchangeData(void * data, const ZEnumInfo * description) = 0;
		virtual void ExchangeData(zstring&) = 0;		
		virtual void ExchangeData(const char *&) = 0;
		virtual void ExchangeData(double &) = 0;
		virtual void ExchangeData(float &) = 0;
		virtual void ExchangeData(uint8_t &) = 0;
		virtual void ExchangeData(int8_t &) = 0;
		virtual void ExchangeData(uint16_t &) = 0;
		virtual void ExchangeData(int16_t &) = 0;
		virtual void ExchangeData(uint32_t &) = 0;
		virtual void ExchangeData(int32_t &) = 0;
		virtual void ExchangeData(bool &) = 0;

		// methods
		ISerializerStream();
		ISerializerStream(EType eType, EContent eContent);
		ZToken GetToken(ZTokenCache* pTokenCache);
		ZToken GetToken(ZTokenCache& tokenCache);
		EPropertyType GetPropertyType(int8_t&) const { return EPropertyType::PT_Int8; };
		EPropertyType GetPropertyType(uint8_t&) const { return EPropertyType::PT_UInt8; };
		EPropertyType GetPropertyType(int16_t&) const { return EPropertyType::PT_Int16; };
		EPropertyType GetPropertyType(uint16_t&) const { return EPropertyType::PT_UInt16; };
		EPropertyType GetPropertyType(int32_t&) const { return EPropertyType::PT_Int32; };
		EPropertyType GetPropertyType(uint32_t&) const { return EPropertyType::PT_UInt32; };
		EPropertyType GetPropertyType(const char*&) const { return EPropertyType::PT_String; };
		EPropertyType GetPropertyType(char&) const { return EPropertyType::PT_Char; };
		EPropertyType GetPropertyType(bool&) const { return EPropertyType::PT_Bool; };
		EPropertyType GetPropertyType(float&) const { return EPropertyType::PT_Float32; };
		EPropertyType GetPropertyType(double&) const { return EPropertyType::PT_Float64; };
		EPropertyType GetPropertyType(ZSerializable&) const { return EPropertyType::PT_Object; };
		bool IsLoading() const;
		bool TestStreamFilter(uint32_t lStreamFilter) const;
		bool CheckType(uint32_t lTypeFilter) const;

		void Exchange(const char* psName, uint16_t& data);
		void Exchange(const ZToken token, uint16_t& data);
		void Exchange(const char* psName, uint32_t& data);
		void Exchange(const ZToken token, uint32_t& data);
		void Exchange(const char* psName, uint8_t& data);
		void Exchange(const ZToken token, uint8_t& data);
		void Exchange(const char* psName, int16_t& data);
		void Exchange(const ZToken token, int16_t& data);
		void Exchange(const char* psName, int32_t& data);
		void Exchange(const ZToken token, int32_t& data);
		void Exchange(const char* psName, float& data);
		void Exchange(const ZToken token, float& data);
		void Exchange(const char* psName, char& data);
		void Exchange(const ZToken token, char& data);
		void Exchange(const char* psName, const char*& data);
		void Exchange(ZTokenCache& tokenCache, const char*& data);
		void Exchange(const ZToken token, const char*& data);
		void Exchange(const char* psName, bool& data);
		void Exchange(const ZToken token, bool& data);
		void Exchange(const ZToken token, ZEventBase& data);

		template <size_t N>
		void Exchange(const ZToken token, int16_t (&data)[N]) { ExchangeArray(token, data, static_cast<uint32_t>(N)); }
		template <size_t N>
		void Exchange(const char* psName, int16_t (&data)[N]) { Exchange(GetToken(psName), data); }
		template <size_t N>
		void Exchange(const ZToken token, float (&data)[N]) { ExchangeArray(token, data, static_cast<uint32_t>(N)); }
		template <size_t N>
		void Exchange(const char* psName, float (&data)[N]) { Exchange(GetToken(psName), data); }

		void ExchangeArray(const char* psName, uint32_t* data, uint32_t count);
		void ExchangeArray(const ZToken token, uint32_t* data, uint32_t count);
		void ExchangeArray(const char* psName, uint8_t* data, uint32_t count);
		void ExchangeArray(const ZToken token, uint8_t* data, uint32_t count);
		void ExchangeArray(const char* psName, int16_t* data, uint32_t count);
		void ExchangeArray(const ZToken token, int16_t* data, uint32_t count);
		void ExchangeArray(const char* psName, int32_t* data, uint32_t count);
		void ExchangeArray(const ZToken token, int32_t* data, uint32_t count);
		void ExchangeArray(const char* psName, float* data, uint32_t count);
		void ExchangeArray(const ZToken token, float* data, uint32_t count);
		void ExchangeArray(const char* psName, const char** data, uint32_t count);
		void ExchangeArray(const ZToken token, const char** data, uint32_t count);
		void ExchangeArray(const char* psName, REFTAB32* data, uint32_t count);
		void ExchangeArray(const ZToken token, REFTAB32* data, uint32_t count);
		void ExchangeArray(const ZToken token, REFTAB* data, uint32_t count);

		void ExchangeRaw(const char* psName, void* data, uint32_t size);
		void ExchangeEnum(const char* psName, void* data, uint32_t count, const ZEnumInfo& enumInfo);
		void ExchangeEnum(const ZToken token, void* data, uint32_t count, const ZEnumInfo& enumInfo);
		void ExchangeContainer(const char* psName, uint32_t& count);
		void ExchangeContainer(ZTokenCache& tokenCache, uint32_t& count);
		void ExchangeBitfield(const ZToken token, ZBitfieldBase& bitfield, const ZEnumInfo& enumInfo);

		// TODO: serializer overloads still pending, see CodeContext.md for current status.

		// members
		int32_t m_Type; // It's masked?
		EContent m_Content;
		bool m_Status;
		bool m_Finished;
		RE_ADD_PADDING(2);
		ZList<ZTokenCache, false, 0> m_Cache; //size 0x8
	};
	RE_VERIFY_SIZE(ISerializerStream, 0x18);

#	if 0
	// TODOLIST: pending generic serializer overloads.
	ISerializerStream::Exchange<ZSoundDllBase>(char const*,ZSoundDllBase &)
	ISerializerStream::Exchange<ZSoundDllBase>(ZToken,ZSoundDllBase &)
	
	ISerializerStream::Exchange<ZROOM>(char const*,ZROOM &)
	ISerializerStream::Exchange<ZROOM>(ZToken,ZROOM &)
	
	ISerializerStream::Exchange<ZHM3CameraSpring>(char const*,ZHM3CameraSpring &)
	ISerializerStream::Exchange<ZHM3CameraSpring>(ZToken,ZHM3CameraSpring &)
	
	ISerializerStream::Exchange<ZHM3CameraScope>(char const*,ZHM3CameraScope &)
	ISerializerStream::Exchange<ZHM3CameraScope>(ZToken,ZHM3CameraScope &)
	
	ISerializerStream::Exchange<ZHM3CameraPositional>(char const*,ZHM3CameraPositional &)
	ISerializerStream::Exchange<ZHM3CameraPositional>(ZToken,ZHM3CameraPositional &)
	
	ISerializerStream::Exchange<ZHM3CameraObservation>(char const*,ZHM3CameraObservation &)
	ISerializerStream::Exchange<ZHM3CameraObservation>(ZToken,ZHM3CameraObservation &)
	
	ISerializerStream::Exchange<ZHM3CameraDialog>(char const*,ZHM3CameraDialog &)
	ISerializerStream::Exchange<ZHM3CameraDialog>(ZToken,ZHM3CameraDialog &)
	
	ISerializerStream::Exchange<ZHM3CameraConsole>(char const*,ZHM3CameraConsole &)
	ISerializerStream::Exchange<ZHM3CameraConsole>(ZToken,ZHM3CameraConsole &)
	
	ISerializerStream::Exchange<ZHM3CameraColi>(char const*,ZHM3CameraColi &)
	ISerializerStream::Exchange<ZHM3CameraColi>(ZToken,ZHM3CameraColi &)
	
	ISerializerStream::Exchange<ZHM3CameraBullet>(char const*,ZHM3CameraBullet &)
	ISerializerStream::Exchange<ZHM3CameraBullet>(ZToken,ZHM3CameraBullet &)
	
	ISerializerStream::Exchange<ZHM3CameraBinocular>(char const*,ZHM3CameraBinocular &)
	ISerializerStream::Exchange<ZHM3CameraBinocular>(ZToken,ZHM3CameraBinocular &)
	
	ISerializerStream::Exchange<ZHM3Camera1stPerson>(char const*,ZHM3Camera1stPerson &)
	ISerializerStream::Exchange<ZHM3Camera1stPerson>(ZToken,ZHM3Camera1stPerson &)
	
	ISerializerStream::Exchange<ZGeomBuffer>(char const*,ZGeomBuffer &)
	ISerializerStream::Exchange<ZGeomBuffer>(ZToken,ZGeomBuffer &)
	
	ISerializerStream::Exchange<ZGEOM>(char const*,ZGEOM &)
	ISerializerStream::Exchange<ZGEOM>(ZToken,ZGEOM *&)
	ISerializerStream::Exchange<ZGEOM>(ZToken,ZGEOM &)
	ISerializerStream::Exchange<ZGEOM *>(char const*,ZGEOM * &)
	
	ISerializerStream::Exchange<ZEventBuffer>(char const*,ZEventBuffer &)
	ISerializerStream::Exchange<ZEventBuffer>(ZToken,ZEventBuffer &)
	ISerializerStream::Exchange<ZEventBase>(ZToken,ZEventBase &)
	
	ISerializerStream::Exchange<ZEngineDataBase>(char const*,ZEngineDataBase &)
	ISerializerStream::Exchange<ZEngineDataBase>(ZToken,ZEngineDataBase &)

	ISerializerStream::Exchange<ZBitfield<WEAPONOPERATION>>(char const*,ZBitfield<WEAPONOPERATION> &)
	ISerializerStream::Exchange<ZBitfield<ITEMSTATE>>(char const*,ZBitfield<ITEMSTATE> &)
	ISerializerStream::Exchange<ZBitfield<ESecurityZone>>(char const*,ZBitfield<ESecurityZone> &)
	ISerializerStream::Exchange<ZBitfield<EAnimMode>>(char const*,ZBitfield<EAnimMode> &)

	ISerializerStream::Exchange<ZBaseGeom>(ZToken,ZBaseGeom *&)
	ISerializerStream::Exchange<ZBaseGeom *>(char const*,ZBaseGeom * &)

	ISerializerStream::Exchange<ZBaseConRout>(ZToken,ZBaseConRout &)

	ISerializerStream::Exchange<WEAPONOPERATION>(ZToken,ZBitfield<WEAPONOPERATION> &)
	ISerializerStream::Exchange<ITEMSTATE>(ZToken,ZBitfield<ITEMSTATE> &)
	ISerializerStream::Exchange<ESecurityZone>(ZToken,ZBitfield<ESecurityZone> &)
	ISerializerStream::Exchange<EAnimMode>(ZToken,ZBitfield<EAnimMode> &)

	ISerializerStream::Exchange<CRequestElevatorManager>(char const*,CRequestElevatorManager &)
	ISerializerStream::Exchange<CRequestElevatorManager>(ZToken,CRequestElevatorManager &)
#	endif

	struct IInputSerializerStream : public ISerializerStream
	{
		// vtbl
		~IInputSerializerStream() override;
		void Skip() override;
		bool IsSaving() const override;
		void ExchangeREF(const ZToken, IREFConverter*) override;
		void ExchangeReftab(const ZToken, REFTAB*) override;
		virtual bool Visit(ISerializerVisitor* pVisitor) = 0;

		// methods
		IInputSerializerStream();
		void SkipObject();
		void PostProcess();
		void _ExchangeObject(ZSerializable* pSerializable);
		void SetNumberOfObjects(uint32_t lCount);
		bool IsItInDebugMode() const;

		// members
		ZSimpleArray<ZSerializable*> m_ProcessTable;
		uint32_t PostProcessWrite;
		// in PS2 build we have aditional debug filed int32_t m_Debug, see PS2 ctor for details
	};
	RE_VERIFY_SIZE(IInputSerializerStream, 0x24);
}
