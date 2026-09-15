#pragma once

#include <Glacier/Audio/ZSoundDllBase.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/ZREF.h>
#include <Glacier/ZSTL/REFTAB32.h>

namespace Glacier
{
    class ZROOM;
    struct SLocation;
    struct SLocationEntry;

    struct SListener
    {
        bool m_bActive;
        RE_ADD_PADDING(3);
        EListenerMode m_eMode;
        ZREF m_rPosObj;
        ZREF m_rDirObj;
    };

    class ZLocationController
    {
    public:
        ZLocationController();
        ~ZLocationController();
        void Update(ZROOM* _room, float* _position);

        REFTAB32 m_rtSources;
        ZREF m_rCurrentRoom;
        SLocationEntry* m_pCurrentLocation;
        SLocationEntry* m_pPreviousLocation;
        SLocation* m_pAdjacentLocations[8];
    };

    RE_VERIFY_SIZE(SListener, 0x10);
    RE_VERIFY_OFFSET(ZLocationController, m_rCurrentRoom, 0xAC);
    RE_VERIFY_SIZE(ZLocationController, 0xD8);
}
