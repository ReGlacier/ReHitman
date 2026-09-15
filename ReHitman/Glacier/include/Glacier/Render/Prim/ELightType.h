#pragma once

#include <cstdalign>


namespace Glacier
{
    enum ELightType
    {
        LTSPOT = 0, // Verified by ZPrimControlBase::GetSpotLightData
        LTOMNI = 1,
        LTSPOTSQUARE = 2, // Verified by ZPrimControlBase::GetSpotLightData
        LTENVIRONMENT = 3,
        LTDIRECTIONAL = 4  // Verified in PS2 at ZOldDrawBase::OnGeomNotify [Uses SPrimLightEnvironment?]
    };
}
