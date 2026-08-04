#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/Geom/ZTreeGroup.h>
#include <Glacier/RTP/PropertyTypes.h>

namespace Glacier
{
    class ZROOM : public ZTreeGroup
    {
    public:
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

        //vftable
        virtual ZROOM* CalcBestRoom(unsigned int, float const*, float const*, float const*);
        virtual bool AddDynamicGeomToRoom(ZBaseGeom*);
        virtual void RemoveDynamicGeomFromRoom(ZBaseGeom*);
        virtual void SetRoomControl(unsigned int, unsigned int);
        virtual unsigned int RoomControl();
        virtual ZBaseGeom** GetDynamicLightsInRoom(ZBaseGeom**, ZBaseGeom**);
        virtual ZBaseGeom** GetStaticPrimDrawGeomsListsRecur(ZBaseGeom**, ZBaseGeom**);
        virtual ZBaseGeom** GetStaticCustomDrawGeomsListsRecur(ZBaseGeom**, ZBaseGeom**);
        virtual ZBaseGeom** GetStaticLightsRecur(ZBaseGeom**, ZBaseGeom**);
        virtual bool NotInRoomTree();

        //data (total size is 0x144, base size is 0x70)
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
        int m_lCacheIndex;
        int m_lSoundGraphId;
        uint32_t m_dwRoomRef;
        int m_lAudioFilter;
        int m_iLastVisibleFrameCount;
        int m_iLightState;
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
    RE_VERIFY_SIZE(ZROOM, 0x144);
}
