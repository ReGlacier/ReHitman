#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZSTL/TIMETYPE.h>
#include <Glacier/GameBase/ZCellCoordinate.h>
#include <Glacier/GameBase/ZCellBitmap.h>
#include <Glacier/GameBase/SSeerObject.h>
#include <cstdint>


namespace Glacier
{
    struct SSeer
    {
        // methods
        SSeer();

        void Dump();
        void LoadSave(ISerializerStream& stream, bool bSaving);
        int32_t CalculateTimeElapsed(TIMETYPE& time);

        // members
        ZGEOM* pSeer;                 // +0x0
        bool bDisabled;               // +0x4
        uint32_t lVisibleTypes;       // +0x8
        float fVisionRange;           // +0xC
        float fVisionCone;            // +0x10
        float fVerticalVisionCone;    // +0x14
        float fLightMultiplier;       // +0x18
        float fSixthSenseRange;       // +0x1C
        ZVector3 vOffset;             // +0x20
        uint32_t lFlag;               // +0x2C
        TIMETYPE ttLastTimeChecked;   // +0x30
        float fLastTimeFraction;      // +0x34
        SSeerObject Objects[200];     // +0x38
        ZCellCoordinate m_cLowCell;   // +0x100
        ZCellCoordinate m_cHighCell;  // +0x103
        ZCellBitmap VisibleCells;     // +0x108
    };
    RE_VERIFY_SIZE(SSeer, 0x128); // PC verified
    RE_VERIFY_OFFSET(SSeer, Objects, 0x38);
    RE_VERIFY_OFFSET(SSeer, m_cLowCell, 0x100);
    RE_VERIFY_OFFSET(SSeer, VisibleCells, 0x108);
}
