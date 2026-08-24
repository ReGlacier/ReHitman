#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZPoolAllocRefTab.h> // ZPoolAllocator
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Geom/ZTreeGroup.h>
#include <Glacier/RTP/PropertyTypes.h>
#include <Glacier/Runtime/Macro.h>
#include <cstdint>


namespace Glacier
{
    class ZROOM : public ZTreeGroup
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZROOM, 0x100021u);

        // static
        static uint8_t s_DynamicGeomsBuffer[32768];
        static ZPoolAllocator s_DynamicGeomsAllocator;

        // constants
        static constexpr uint32_t ZROOM_NOT_IN_TREE = 0x4;

        // types
        struct ZExit
        {
            ZVector3 p1;
            ZVector3 p2;
            ZVector3 p3;
            ZVector3 p4;
            ZROOM* m_pNeighbor;
            uint16_t m_lNeighborExitIndex;
            uint8_t m_lControl;
            uint8_t m_lType;
        };
        RE_VERIFY_SIZE(ZExit, 0x38);

        struct ZNeighborRoom
        {
            ZROOM* m_pNeighbor;
            uint32_t m_lNrGeomsInExit;
            uint32_t* m_pGeomsInExit;
        };
        RE_VERIFY_SIZE(ZNeighborRoom, 0xC);

        enum ENoiseLevel : uint32_t
        {
            eSuperQuiet = 0,
            eQuiet = 1,
            eNormal = 2,
            eNoisy = 3,
            eVeryNoisy = 4,
            eNoEvents = 5
        };

        enum ELocation : uint32_t
        {
            eUNDEFINED = 0,
            eOUTSIDE = 1,
            eINSIDE = 2,
            eBOTH = 3
        };

#if 0
        // This definition was taken from PS2 build, in Mini Ninjas & K&L it's only fwd declared
        // I'm not sure about this decl because ZGeom is not a type. It's fwd decl too (and it's not ZGEOM*)
        struct ZTempRoom {
            ZRoomViewEntry* m_pRoomViewEntry; // +0x00
            ZExit* m_pFirstExit;             // +0x04
            ZGeom* m_pFirstGeom;             // +0x08
            int m_lNrExits;                  // +0x0C
            bool m_bIsInside;                // +0x10
        };
#endif

        // vtbl
        ~ZROOM() override;

        // ZSerializable
        bool PostLoad(ISerializerStream& stream) override;
        void LoadSave(ISerializerStream& stream, bool bSaving) override;

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void ClassInit() override;
        void PostClassInit2() override;
        void CopyData(const ZGEOM* Source) override;

        // ZGROUP

        // ZTreeGroup
        void SetStaticContainer(const bool& static_container) override;

        // ZROOM
        virtual ZROOM* CalcBestRoom(ZREF rOldRoom, const ZMat3x3& mMat, const ZVector3& vPos, const ZVector3& s0);
        virtual bool AddDynamicGeomToRoom(ZBaseGeom* pBaseGeom);
        virtual void RemoveDynamicGeomFromRoom(ZBaseGeom* pBaseGeom);
        virtual void SetRoomControl(uint32_t lAdd, uint32_t lRemove);
        virtual uint32_t RoomControl() const;
        virtual ZBaseGeom** GetDynamicLightsInRoom(ZBaseGeom** pDrawGeomsList, ZBaseGeom** pDrawGeomsListEnd);
        virtual ZBaseGeom** GetStaticPrimDrawGeomsListsRecur(ZBaseGeom** pDrawGeomsList, ZBaseGeom** pDrawGeomsListEnd);
        virtual ZBaseGeom** GetStaticCustomDrawGeomsListsRecur(ZBaseGeom** pDrawGeomsList, ZBaseGeom** pDrawGeomsListEnd);
        virtual ZBaseGeom** GetStaticLightsRecur(ZBaseGeom** pDrawGeomsList, ZBaseGeom** pDrawGeomsListEnd);
        virtual bool NotInRoomTree() const;

        // methods
        ZROOM(const char* psName, ZBaseGeom* pBaseGeom);
        void FreeGeomsLists();
        void FreeDynamicGeomList();
        void SetAttachedRoom(ZROOM* pRoom);

        // members
        uint32_t m_lNrExits;
        ZExit* m_pExits;
        uint32_t m_rAttachedDrawBaseGeoms[2];
        uint32_t m_rTriangleList;
        ZNeighborRoom* m_pNeighborRooms;
        uint8_t m_lNrNeighborRooms;
        uint8_t m_padA1[3];
        uint32_t m_lRoomOccl;
        uint32_t m_lFogColor;
        bool m_bIsOnDrawStack;
        bool m_padAD[3];
        void* m_pRoomCache; // Need to check this!
        struct ZTempRoom* m_pTempRoom; // Only fwd decl
        int32_t m_lCacheIndex;
        int32_t m_lSoundGraphId;
        uint32_t m_dwRoomRef;
        int32_t m_lAudioFilter;
        uint32_t m_iLastVisibleFrameCount;
        uint32_t m_iLightState;
        REFTAB m_LightSwitches;
        REFTAB m_FurnitureList;
        REFTAB* m_pActorsAwareOfBrokenLight;
        struct ZPoolAllocRefTab* m_pDynamicGeoms;
        uint32_t m_lRoomControl;
        ZBaseGeom* m_pEnvironment;
        uint32_t m_lStaticGeomsCustomDrawList;
        uint32_t m_lStaticGeomsPrimDrawList;
        uint32_t m_lDynamicGeomsDrawList;
        uint32_t m_lMusicId;
        ZVector3 m_vGeometryCen;
        ZVector3 m_vGeometrySize;
        ENoiseLevel m_eNoiseLevel;
        ELocation m_eLocationState;
        ZRawData m_StaticShadowSampleData;
        ZVector3 m_vWind;
    };
    RE_VERIFY_SIZE(ZROOM, 0x144); // Verified PC alloc
}
