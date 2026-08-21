#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Component/ZRuntimeComponentBase.h>
#include <Glacier/Component/ZComponentSingleton.h>
#include <Glacier/ZSTL/REFTAB32.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    class ZWaterBox;

    class ZWaterBoxManager : public ZComponentSingleton<ZWaterBoxManager, ZRuntimeComponentBase>
    {
    public:
        // vtbl
        ~ZWaterBoxManager() override;
        void NukeAndRestart() override;

        // methods
        ZWaterBoxManager();

        uint32_t Count() const;
        bool Exists(const ZWaterBox* pWaterBox) const;
        bool Exists(ZREF rWaterBox) const;
        ZWaterBox* Find(ZGEOM* pGeom) const;
        ZWaterBox* Find(uint32_t lIndex) const;
        bool Add(ZWaterBox* pWaterBox);
        bool Add(ZREF rWaterBox);
        void Remove(const ZWaterBox* pWaterBox);
        void Remove(ZREF rWaterBox);

        ZWaterBox* operator[](uint32_t lIndex);
        ZWaterBox* operator[](ZGEOM* pGeom);
        const ZWaterBox* operator[](uint32_t lIndex) const;
        const ZWaterBox* operator[](ZGEOM* pGeom) const;

        // members
        mutable REFTAB32 m_WaterBoxList;
    };
    RE_VERIFY_SIZE(ZWaterBoxManager, 0xBC); // Verified PC allocation
}
