#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZRoomGeomVolumes.h>
#include <Glacier/ZSTL/ZStackArray.h>
#include <Glacier/ZSTL/ZMath.h>


namespace Glacier
{
    // fwds
    class ZCameraSpace;
    class ZRenderEntry;
    class ZVolumeList;

    class ZViewSpace
    {
    public:
        // constants
        static constexpr int MAX_ENTRIES_NR = 1024;

        // types
        struct SClipSpace
        {
            ZVector3 m_vPosition;
            float m_ClipPlanes[48];
            bool m_bClipPlaneEnabled[12];
        };

        struct ZExitPlanes
        {
            ZVector3 pl1;
            ZVector3 pl2;
            ZVector3 pl3;
            ZVector3 pl4;
        };

        struct ZVisibleExit
        {
            ZViewSpace::ZExitPlanes m_Exit;
            ZViewSpace::ZVisibleExit* m_pNext;
        };

        struct ZVisibleRoom
        {
            ZROOM* m_pRoom;
            uint32_t m_lNumExits;
            ZViewSpace::ZVisibleExit* m_pFirstExit;
        };

        // methods
        ZViewSpace();

        void SetClipPlanesFromCameraSpace(ZCameraSpace* pCamSpace);
        void SetClipPlane(uint32_t lIndex, const float* pPlane);
        bool SetClipPlaneEnabled(uint32_t lIndex, bool bEnabled);
        void GetClipPlane(uint32_t lIndex, float* pPlane) const;
        void GetPosition(float* pPosition) const;
        void SetPosition(const float* pPosition);
        void SetPositionInnerRoom(const float* pPosition);
        void GetPositionInnerRoom(float* pPosition) const;
        void Reset();
        uint32_t GetVisibleVolumes(ZVolumeList* pVolumeList, bool bIncludeBackdrop, ZRenderEntry* pRenderEntry);
        bool GetVisibleVolumesIncludingBackdrop(ZVolumeList* pVolumeList, ZRenderEntry* pRenderEntry, bool bIncludeBackdrop);

        // members
        ZVector3 m_vPosition;                                   // 0x0000
        ZVector3 m_vPositionInnerRoom;                          // 0x000C
        float m_ClipPlanes[48];                                 // 0x0018
        bool m_bClipPlanesEnabled[12];                          // 0x00D8
        ZStackArray<MAX_ENTRIES_NR, ZViewSpace::ZVisibleRoom> m_Rooms;   // 0x00E4
        ZStackArray<MAX_ENTRIES_NR, ZViewSpace::ZVisibleExit> m_Exits;   // 0x30E8
        int m_iNumUserPlanes;                                   // 0x100EC
        int m_iNumExists;                                       // 0x100F0
        ZGROUP* m_pTopNode;                                     // 0x100F4
        ZROOM* m_pOwnerRoom;                                    // 0x100F8
        bool m_bExitsEnabled;                                   // 0x100FC
        bool m_bTraverseThroughExitsEnabled;                    // 0x100FD
        ZRoomGeomVolumes m_RoomGeomVolumes;                     // 0x10100
        float m_fFieldOfView;                                   // 0x3111C

    private:
        uint32_t GetClipPlaneDistances(float* pPlanes) const;
        void LocateRoomsAndExits();
        void GetVisibleRoomsRecursive(ZGROUP* pGroup, const float* pPlanes, uint32_t lNrPlanes, const float* pMat, const float* pPos);
        void CheckExitsInRoom(ZVisibleRoom* pRoom, ZVisibleExit* pFirstExit, bool bEnabled, const float* pPlanes, uint32_t lNrPlanes);
    };
    RE_VERIFY_SIZE(ZViewSpace, 0x31120);
    RE_VERIFY_OFFSET(ZViewSpace, m_vPosition, 0x0);
    RE_VERIFY_OFFSET(ZViewSpace, m_vPositionInnerRoom, 0xC);
    RE_VERIFY_OFFSET(ZViewSpace, m_ClipPlanes, 0x18);
    RE_VERIFY_OFFSET(ZViewSpace, m_bClipPlanesEnabled, 0xD8);
    RE_VERIFY_OFFSET(ZViewSpace, m_Rooms, 0xE4);
    RE_VERIFY_OFFSET(ZViewSpace, m_Exits, 0x30E8);
    RE_VERIFY_OFFSET(ZViewSpace, m_iNumUserPlanes, 0x100EC);
    RE_VERIFY_OFFSET(ZViewSpace, m_iNumExists, 0x100F0);
    RE_VERIFY_OFFSET(ZViewSpace, m_pTopNode, 0x100F4);
    RE_VERIFY_OFFSET(ZViewSpace, m_pOwnerRoom, 0x100F8);
    RE_VERIFY_OFFSET(ZViewSpace, m_bExitsEnabled, 0x100FC);
    RE_VERIFY_OFFSET(ZViewSpace, m_bTraverseThroughExitsEnabled, 0x100FD);
    RE_VERIFY_OFFSET(ZViewSpace, m_RoomGeomVolumes, 0x10100);
    RE_VERIFY_OFFSET(ZViewSpace, m_fFieldOfView, 0x3111C);
}
