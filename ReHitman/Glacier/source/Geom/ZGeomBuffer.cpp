#include <Glacier/EventBase/ZEventBuffer.h>
#include <Glacier/Runtime/ZGEOMCLASSINFO.h>
#include <Glacier/Geom/ZGeomListTypeUtils.h>
#include <Glacier/Geom/ZGeomEventList.h>
#include <Glacier/Geom/ZGeomBuffer.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/Serializer/IOutputSerializerStream.h>
#include <Glacier/Debug/ZMemReadOut.h>
#include <Glacier/ZSTL/ZOffsetAlloc.h>
#include <Glacier/ZUniAssert.h>
#include <cstring>


namespace Glacier
{
	ZGeomBuffer::~ZGeomBuffer()
	{
		assert(m_Instance==this);
		m_Instance = nullptr;

		if (m_BaseGeomMemoryManager)
		{
			ZUniMemory::Delete(m_BaseGeomMemoryManager);
			m_BaseGeomMemoryManager = nullptr;
		}

		if (m_pBaseGeomLists)
		{
			ZUniMemory::Delete(m_pBaseGeomLists);
			m_pBaseGeomLists = nullptr;
		}

		if (m_pExtraGeomElems)
		{
			ZUniMemory::Delete(m_pExtraGeomElems);
			m_pExtraGeomElems = nullptr;
		}

		ZUniMemory::Delete(m_pGeomResourcesHeaders);
		ZUniMemory::Delete(m_pGeomResources);
		ZUniMemory::Free(m_pRoomListsBuffer);
		ZUniMemory::Free(m_pFreeRoomLists);

		if (m_pEventBuffer)
		{
			ZUniMemory::Delete(m_pEventBuffer);
			m_pEventBuffer = nullptr;
		}

		ZGeomEventList::Destroy_Geom_Event_Buffer_Lists();
	}

	void ZGeomBuffer::LoadObject(IInputSerializerStream& stream)
	{
		uint32_t lNumberOfGeoms = m_BaseGeomMemoryManager->Count();
		stream.Exchange("NumberOfGeoms", lNumberOfGeoms);

		for (uint32_t i = 0; i < lNumberOfGeoms; ++i)
		{
			bool bNameIsStatic = false;
			stream.Exchange("NameIsStatic", bNameIsStatic);

			const char* szName = nullptr;
			if (bNameIsStatic)
			{
				uint32_t lNameIndex = 0;
				stream.Exchange("NameIndex", lNameIndex);
				ZASSERT(false && "Static geom names are not supported yet");
			}
			else
			{
				stream.Exchange("Name", szName);
			}

			uint32_t lRef = 0;
			stream.Exchange("REF", lRef);

			uint32_t lGeomType = 0;
			stream.Exchange("GeomType", lGeomType);

			ZBaseGeom* pBaseGeom = GeomRefToBasePtr(lRef);
			if (!pBaseGeom)
			{
				pBaseGeom = AllocBaseGeomDirect(lRef);
			}

			AllocGeom(szName, lGeomType, pBaseGeom);
			stream.Exchange("Prim", pBaseGeom->m_lPrim);
		}

		ZToken token = stream.GetToken("EventBuffer");
		stream.ExchangeHeader(token, ISerializerStream::PT_Object);
		stream.ExchangeData(m_pEventBuffer);
		stream.ExchangeFooter(ISerializerStream::PT_Object);
	}

	void ZGeomBuffer::SaveObject(IOutputSerializerStream& stream)
	{
		uint32_t lNumberOfGeoms = m_BaseGeomMemoryManager->Count();
		stream.Exchange("NumberOfGeoms", lNumberOfGeoms);

		for (auto it = m_BaseGeomMemoryManager->Begin(); it != m_BaseGeomMemoryManager->End(); ++it)
		{
			ZBaseGeom* pBaseGeom = *it;
			ZGEOM* pGeom = pBaseGeom->GetGeom();
			ZASSERT(pGeom);

			bool bNameIsStatic = false;
			stream.Exchange("NameIsStatic", bNameIsStatic);

			const char* szName = pBaseGeom->Name();
			stream.Exchange("Name", szName);

			uint32_t lRef = GeomPtrToRef(pBaseGeom);
			stream.Exchange("REF", lRef);

			uint32_t lGeomType = pGeom->GetOldClassInfo()->m_lType;
			stream.Exchange("GeomType", lGeomType);

			stream.Exchange("Prim", pBaseGeom->m_lPrim);
		}

		ZToken token = stream.GetToken("EventBuffer");
		stream.ExchangeHeader(token, ISerializerStream::PT_Object);
		stream.ExchangeData(m_pEventBuffer);
		stream.ExchangeFooter(ISerializerStream::PT_Object);
	}

	ZGeomBuffer& ZGeomBuffer::Instance()
	{
		ZASSERT(m_Instance);
		return *ZGeomBuffer::m_Instance;
	}

	void ZGeomBuffer::SetDeleteGeomType(uint lType)
	{
		ZGeomBuffer::m_lDeleteGeomType = lType;
	}

	ZGeomBuffer::ZGeomBuffer(uint32_t lBaseGeomBufferSize, uint32_t lExtraGeomBufferSize, uint32_t lListBufferSize, uint32_t lEventBufferSize)
		: m_MaxNumberOfBaseGeoms(lBaseGeomBufferSize / sizeof(ZBaseGeom))
	{
		Initialize(lBaseGeomBufferSize, lExtraGeomBufferSize, lListBufferSize, lEventBufferSize);
	}

	void ZGeomBuffer::Initialize(uint32_t lBaseGeomBufferSize, uint32_t lExtraGeomBufferSize, uint32_t lListBufferSize, uint32_t lEventBufferSize)
	{
		ZGeomEventList::Create_Geom_Event_Buffer_Lists();

		m_BaseGeomMemoryManager = ZUniMemory::New<ZFixedSizeMemoryManager<ZBaseGeom>>(m_MaxNumberOfBaseGeoms);
		InitRoomListBuffer();
		
		m_bGeomCreationLock = false;
		
		uint32_t lAlignedBaseGeomBufferSize = sizeof(ZBaseGeom) * ((lBaseGeomBufferSize + sizeof(ZBaseGeom) - 1) / sizeof(ZBaseGeom));
		m_lMaxTotalNrBaseGeoms = lAlignedBaseGeomBufferSize / sizeof(ZBaseGeom);

		uint32_t lAlignedExtraGeomBufferSize = (lExtraGeomBufferSize + 15) & ~15u;
		m_pExtraGeomElems = ZUniMemory::New<ZQElemsBuffer>(lAlignedExtraGeomBufferSize, nullptr);

		uint32_t lAlignedListBufferSize = (lListBufferSize + 15) & ~15u;
		m_pBaseGeomLists = ZUniMemory::New<ZBaseGeomLists>(lAlignedListBufferSize, nullptr);

		m_pEventBuffer = ZUniMemory::New<ZEventBuffer>(lEventBufferSize);
		m_pGeomResourcesHeaders = nullptr;
		m_pGeomResources = nullptr;
		m_lNrGeomResourcesHeaders = 0;
		m_lNrResources = 0;

		ZGeomBuffer::m_Instance = this;
	}
	
	uint32_t ZGeomBuffer::AddGeoms(uint32_t lListID, ZBaseGeom* pBaseGeom1, ZBaseGeom* pBaseGeom2)
	{
		const uint32_t lGeomListType = static_cast<uint32_t>(GetBaseGeomListType(pBaseGeom1));
		return m_pBaseGeomLists->AddBaseGeoms(lListID, pBaseGeom1, pBaseGeom2, lGeomListType, 8u);
	}

	ZBaseGeom* ZGeomBuffer::AllocBaseGeom()
	{
		auto* pBaseGeom = m_BaseGeomMemoryManager->Alloc();
		if (pBaseGeom && ZMemReadOut::Exists())
		{
			ZMemReadOut::Instance().OverrideMemColors((char*)pBaseGeom, sizeof(ZBaseGeom), GEOMBASE_MEM_COLOR);
		}

		return pBaseGeom;
	}

	ZBaseGeom* ZGeomBuffer::AllocBaseGeomDirect(ZREF ref)
	{
		auto* pBaseGeom = m_BaseGeomMemoryManager->AllocDirect(ref);
		if (pBaseGeom && ZMemReadOut::Exists())
		{
			ZMemReadOut::Instance().OverrideMemColors((char*)pBaseGeom, sizeof(ZBaseGeom), GEOMBASE_MEM_COLOR);
		}

		return pBaseGeom;
	}

	ZGEOM* ZGeomBuffer::AllocExtraGeom(uint32_t lExtraGeomSize)
	{
		ZASSERT(lExtraGeomSize);

		uint32_t lAllocSize = (lExtraGeomSize + 3) & ~3u;
		uint32_t lOffset = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(m_pExtraGeomElems->Add(lAllocSize, true)));
		ZGEOM* pGeom = static_cast<ZGEOM*>(m_pExtraGeomElems->GetPtr(lOffset));

		std::memset(pGeom, 0, lAllocSize);
		pGeom->m_baseGeom = nullptr;

		return pGeom;
	}

	ZBaseGeom* ZGeomBuffer::AllocGeom(char const* szName, uint32_t lGeomType, ZBaseGeom* pBaseGeom)
	{
		static bool bAllocGeomLock = false;
		ZASSERT(!GeomCreationLocked());
		ZASSERT(!bAllocGeomLock);

		bAllocGeomLock = true;

		if (!pBaseGeom)
		{
			pBaseGeom = AllocBaseGeom();
		}

		g_pGeomName = szName;
		g_pBaseGeom = pBaseGeom;


		ZGEOM* pGeom = ZGEOM::GetFactory().Create(lGeomType);
		if (pGeom)
		{
			ZASSERT(pGeom->m_baseGeom == pBaseGeom);
		}

		bAllocGeomLock = false;
		return pGeom ? pBaseGeom : nullptr;
	}
	
	ZBaseGeom* ZGeomBuffer::AllocResourceGeom(char const* szName, uint32_t lGeomResourceId, uint32_t lGeomClassType)
	{
		std::ignore = lGeomResourceId;

		return AllocGeom(szName, lGeomClassType, nullptr);
	}
	
	ZBaseGeomRoomList* ZGeomBuffer::AllocRoomList()
	{
		uint32_t lNrRoomLists = m_lNrRoomLists;

		if (lNrRoomLists >= MAX_FREE_ROOMS_NR)
		{
			return nullptr;
		}

		uint16_t lRoomListNr = m_pFreeRoomLists[lNrRoomLists];
		m_lNrRoomLists = lNrRoomLists + 1;

		ZBaseGeomRoomList* pRoomList = &m_pRoomListsBuffer[lRoomListNr];
		pRoomList->Init();
		return pRoomList;
	}
	
	ZBaseGeom* ZGeomBuffer::BaseGeomBufferPtr()
	{
		return m_BaseGeomMemoryManager->GetBufferPtr();
	}
	
	bool ZGeomBuffer::Exists(uint32_t lListID, ZBaseGeom* pBaseGeom)
	{
		const uint32_t lBaseGeomListType = static_cast<uint32_t>(GetBaseGeomListType(pBaseGeom));
		return m_pBaseGeomLists->Exists(lListID, pBaseGeom, lBaseGeomListType);
	}
	
	void ZGeomBuffer::FreeBaseGeom(ZBaseGeom* pBaseGeom)
	{
		m_BaseGeomMemoryManager->Free(pBaseGeom);
	}
	
	void ZGeomBuffer::FreeGeom(ZGEOM* pGeom)
	{
		const uint32_t lGeomAllocSizeAligned = (pGeom->GetOldClassInfo()->m_lSize + 3) & ~3u;

		m_pExtraGeomElems->Remove(pGeom, lGeomAllocSizeAligned);

		if (ZMemReadOut::Exists())
		{
			// Idk about color constant, need investigate
			ZMemReadOut::Instance().OverrideMemColors((char*)pGeom, lGeomAllocSizeAligned, 0x7F0000u);
		}
	}

	void ZGeomBuffer::FreeRoomList(ZBaseGeomRoomList* pRoomList)
	{
		if (pRoomList)
		{
			uint32_t lRoomListNr = m_lNrRoomLists - 1;
			m_lNrRoomLists = lRoomListNr;
			m_pFreeRoomLists[lRoomListNr] = static_cast<uint16_t>(pRoomList - m_pRoomListsBuffer);
		}
	}

	ZREF ZGeomBuffer::GeomPtrToRef(const ZGEOM* pGeom)
	{
		if (!pGeom)
		{
			return 0;
		}

		return GeomPtrToRef(pGeom->m_baseGeom);
	}

	ZREF ZGeomBuffer::GeomPtrToRef(const ZBaseGeom* pBaseGeom)
	{
		if (!pBaseGeom)
		{
			return 0;
		}

		return m_BaseGeomMemoryManager->Ptr2REF((void*)pBaseGeom) + 1;
	}

	ZBaseGeom* ZGeomBuffer::GeomRefToBasePtr(ZREF ref)
	{
		if ((ref & 0x40000000) != 0)
		{
			return m_BaseGeomMemoryManager->Index2Address((ref & 0x3FFFFFFF) / sizeof(ZBaseGeom));
		}
		else if (ref)
		{
			return m_BaseGeomMemoryManager->REF2Ptr(ref - 1);
		}

		return nullptr;
	}
	
	ZGEOM* ZGeomBuffer::GeomRefToPtr(ZREF GeomRef)
	{
		auto* pBaseGeom = ZGeomBuffer::GeomRefToBasePtr(GeomRef);
		if (!pBaseGeom)
		{
			return nullptr;
		}

		auto* geom = pBaseGeom->GetGeom();
		ZASSERT(geom);

		return geom;
	}

	ZBaseGeomRoomList* ZGeomBuffer::GetRoomList(uint16_t lRoomListNr)
	{
		if (lRoomListNr)
		{
			return &m_pRoomListsBuffer[lRoomListNr - 1];
		}

		return nullptr;
	}

	uint16_t ZGeomBuffer::GetRoomListNr(ZBaseGeomRoomList* pRoomList)
	{
		if (pRoomList)
		{
			return static_cast<uint16_t>(pRoomList - m_pRoomListsBuffer + 1);
		}

		return 0;
	}
	
	void ZGeomBuffer::InitResourceGeoms(SPackedGeomsHeader* pPackedGeomsHeader)
	{
		m_lNrGeomResourcesHeaders = 0;
		m_lNrResources = 0;

		uint32_t lGeomResourcesHeadersOffset = pPackedGeomsHeader->m_lGeomResourcesHeadersOffset;
		uint32_t lGeomResourcesOffset = pPackedGeomsHeader->m_lGeomResourcesOffset;

		if (lGeomResourcesHeadersOffset)
		{
			const uint8_t* pPackedData = reinterpret_cast<const uint8_t*>(pPackedGeomsHeader);
			uint32_t lNrGeomResourcesHeaders = *reinterpret_cast<const uint32_t*>(pPackedData + lGeomResourcesHeadersOffset);

			m_pGeomResourcesHeaders = static_cast<SGeomResourcesHeader*>(ZUniMemory::Allocate(sizeof(SGeomResourcesHeader) * lNrGeomResourcesHeaders));
			m_lNrGeomResourcesHeaders = lNrGeomResourcesHeaders;

			ZASSERT(m_lNrGeomResourcesHeaders < 255);
			std::memcpy(m_pGeomResourcesHeaders, pPackedData + lGeomResourcesHeadersOffset + sizeof(uint32_t), sizeof(SGeomResourcesHeader) * lNrGeomResourcesHeaders);
		}

		if (lGeomResourcesOffset)
		{
			const uint8_t* pPackedData = reinterpret_cast<const uint8_t*>(pPackedGeomsHeader);
			uint32_t lNrResources = *reinterpret_cast<const uint32_t*>(pPackedData + lGeomResourcesOffset);

			m_pGeomResources = static_cast<SGeomResources*>(ZUniMemory::Allocate(sizeof(SGeomResources) * lNrResources));
			m_lNrResources = lNrResources;

			std::memcpy(m_pGeomResources, pPackedData + lGeomResourcesOffset + sizeof(uint32_t), sizeof(SGeomResources) * lNrResources);
		}
	}
	
	void ZGeomBuffer::InitRoomListBuffer()
	{
		m_lNrRoomLists = 0;
		m_pRoomListsBuffer = static_cast<ZBaseGeomRoomList*>(ZUniMemory::Allocate(sizeof(ZBaseGeomRoomList) * MAX_FREE_ROOMS_NR));
		m_pFreeRoomLists = static_cast<uint16_t*>(ZUniMemory::Allocate(sizeof(uint16_t) * MAX_FREE_ROOMS_NR));

		uint16_t iRoomIdx = 0;
		uint16_t iRoomToPlace = 0;

		do
		{
			m_pFreeRoomLists[iRoomIdx] = iRoomToPlace;
			iRoomToPlace = ++iRoomIdx;
		}
		while (iRoomIdx != MAX_FREE_ROOMS_NR);
	}

	uint32_t ZGeomBuffer::RemoveGeoms(uint32_t lListID, ZBaseGeom* pBaseGeom1, ZBaseGeom* pBaseGeom2)
	{
		const uint32_t lBaseType = static_cast<uint32_t>(GetBaseGeomListType(pBaseGeom1));
		return m_pBaseGeomLists->RemoveBaseGeoms(lListID, pBaseGeom1, pBaseGeom2, lBaseType);
	}

	bool ZGeomBuffer::GeomCreationLocked() const
	{
		return m_bGeomCreationLock;
	}

	void ZGeomBuffer::LockGeomCreation()
	{
		m_bGeomCreationLock = true;
	}

	void ZGeomBuffer::UnlockGeomCreation()
	{
		m_bGeomCreationLock = false;
	}

	STATIC_CLASS_VAR_IMPL(ZGeomBuffer, ZGeomBuffer*, m_Instance, 0x008BA0C0, nullptr); // addr from PC
	STATIC_CLASS_VAR_IMPL(ZGeomBuffer, uint32_t, m_lDeleteGeomType, 0x008BA0BC, 0); // addr from PC
}
