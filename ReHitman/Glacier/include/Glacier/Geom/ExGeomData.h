#pragma once

#include <Glacier/Geom/ZGeomEventList.h>
#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    struct CHUNKFILE;

    static constexpr int16_t ZCEXWANTCAMERAMSG = 1;
    static constexpr int16_t ZCEXMOVINGOBJ = 2;

    struct ExGeomData
    {
        // methods
        ExGeomData() = default;
        ~ExGeomData() = default;

        // members
        // Known bits
        // 1 - want camera msg | ZBaseGeom::WantCameraMsg
        // 2 - is moving
        int16_t _lControl{0};
        ZGeomEventList _Events{};
        RE_ADD_PADDING(2);
        CHUNKFILE* _ExtraInitData{nullptr};
    };
    RE_VERIFY_SIZE(ExGeomData, 0xC); // Verified

}