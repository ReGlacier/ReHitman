#pragma once

#include <Glacier/Glacier.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/EventBase/ZEventBuffer.h>
#include <Glacier/Geom/ZBaseGeomRoomList.h>
#include <Glacier/Geom/ZBaseGeomLists.h>
#include <Glacier/Geom/ZBaseGeom.h> // ZBaseGeom
#include <Glacier/Serializer/ZSerializable.h>
#include <Glacier/ZSTL/ZFixedSizeMemoryManager.h>
#include <Glacier/ZSTL/ZRTTI.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZSTL/ZQElemsBuffer.h>


namespace Glacier
{
    struct SGeomResourcesHeader
    {
        uint32_t m_lId;
        uint32_t m_lGeomResourcesStart;
        uint32_t m_lNrGeomResources;
    };
    RE_VERIFY_SIZE(SGeomResourcesHeader, 0xC);

    struct SPackedGeomsHeader
    {
        unsigned int StartQuad[8]; // rly weird part, will discuss this later
        unsigned int m_lGeomResourcesHeadersOffset;
        unsigned int m_lGeomResourcesOffset;
        unsigned int m_lRoomEnvironmentListOffset;
        unsigned int m_iCollisionDataOffset;
        unsigned int m_iMaterialDescOffset;
        unsigned int m_lOffsetPathfinder4Data;
        unsigned int m_lPhysicsDataOffset;
        unsigned int m_iHighestGeomNr;
        unsigned int m_iWeaponPrimsOffset;
        unsigned int m_iExcludedAnimNamesOffset;
    };
    RE_VERIFY_SIZE(SPackedGeomsHeader, 0x48);

    struct SGeomResources
    {
        uint32_t m_lGeomType;
        uint32_t m_lNrUnusedGeoms;
    };
    RE_VERIFY_SIZE(SGeomResources, 0x8);

    // Class definition ZGeomBuffer
    class ZGeomBuffer : public ZSerializable // total size is 0x3C
    {
    public:
        // constants
        static constexpr size_t MAX_FREE_ROOMS_NR = 510; // Mini Ninjas - 1024

        // static 
        STATIC_CLASS_VAR(ZGeomBuffer, ZGeomBuffer*, m_Instance);
        STATIC_CLASS_VAR(ZGeomBuffer, uint32_t, m_lDeleteGeomType);

        // vftable
        ~ZGeomBuffer() override;
        void LoadObject(IInputSerializerStream&) override;
        void SaveObject(IOutputSerializerStream&) override;

        // methods
        static ZGeomBuffer& Instance();
        static void SetDeleteGeomType(uint lType);

        ZGeomBuffer(uint32_t lBaseGeomBufferSize, uint32_t lExtraGeomBufferSize, uint32_t lListBufferSize, uint32_t lEventBufferSize);
        void Initialize(uint32_t lBaseGeomBufferSize, uint32_t lExtraGeomBufferSize, uint32_t lListBufferSize, uint32_t lEventBufferSize);
        uint32_t AddGeoms(uint32_t lListID, ZBaseGeom* pBaseGeom1, ZBaseGeom* pBaseGeom2);
        ZBaseGeom* AllocBaseGeom();
        ZBaseGeom* AllocBaseGeomDirect(ZREF ref);
        ZGEOM* AllocExtraGeom(uint32_t lExtraGeomSize);
        ZBaseGeom* AllocGeom(char const* szName, uint32_t lGeomType, ZBaseGeom* pBaseGeom);
        ZBaseGeom* AllocResourceGeom(char const* szName, uint32_t lGeomResourceId, uint32_t lGeomClassType);
        ZBaseGeomRoomList* AllocRoomList();
        ZBaseGeom* BaseGeomBufferPtr();
        bool Exists(uint32_t lListID, ZBaseGeom* pBaseGeom);
        void FreeBaseGeom(ZBaseGeom* pBaseGeom);
        void FreeGeom(ZGEOM* pGeom);
        void FreeRoomList(ZBaseGeomRoomList* pRoomList);
        ZREF GeomPtrToRef(const ZGEOM* pGeom);
        ZREF GeomPtrToRef(const ZBaseGeom* pBaseGeom);
        ZBaseGeom* GeomRefToBasePtr(ZREF ref);
        ZGEOM* GeomRefToPtr(ZREF GeomRef);
        ZBaseGeomRoomList* GetRoomList(uint16_t lRoomListNr);
        uint16_t GetRoomListNr(ZBaseGeomRoomList* pRoomList);
        void InitResourceGeoms(SPackedGeomsHeader* pPackedGeomsHeader);
        void InitRoomListBuffer();
        uint32_t RemoveGeoms(uint32_t lListID, ZBaseGeom* pBaseGeom1, ZBaseGeom* pBaseGeom2);
        bool GeomCreationLocked() const;
        void LockGeomCreation();
        void UnlockGeomCreation();

        // members
        ZEventBuffer* m_pEventBuffer;
        const uint32_t m_MaxNumberOfBaseGeoms;
        ZFixedSizeMemoryManager<ZBaseGeom>* m_BaseGeomMemoryManager;
        ZQElemsBuffer* m_pExtraGeomElems;
        ZBaseGeomLists* m_pBaseGeomLists;
        SGeomResourcesHeader* m_pGeomResourcesHeaders;
        SGeomResources* m_pGeomResources;
        uint32_t m_lNrGeomResourcesHeaders;
        uint32_t m_lNrResources;
        bool m_bGeomCreationLock;
        bool padding[3];
        ZBaseGeomRoomList* m_pRoomListsBuffer;
        uint16_t* m_pFreeRoomLists;
        uint32_t m_lNrRoomLists;
        uint32_t m_lMaxTotalNrBaseGeoms;
    }; // Verified size is 0x3C
    RE_VERIFY_SIZE(ZGeomBuffer, 0x3C); // verified
}
