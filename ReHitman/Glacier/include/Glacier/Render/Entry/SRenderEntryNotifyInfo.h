#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <cstdint>


namespace Glacier
{
    struct SRenderEntryNotifyInfo
    {
        float fLODScale;
        float vObserver[3];
        bool bMirror;
        bool bFirstPersonCamera;
        RE_ADD_PADDING(2);
        float fDistance;
        uint32_t lDrawDestinationOverride;
        uint32_t lLODMask;
        ZBaseGeom* pCamera;
    };
}