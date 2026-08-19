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

        // TODO: Finish me after ZWaterBox reversed
// public: bool Exists(const ZWaterBox*);
// public: bool Exists(unsigned int);
// public: bool Add(ZWaterBox*);
// public: bool Add(unsigned int);
// public: void Remove(const ZWaterBox*);
// public: void Remove(unsigned int);
// public: uint32 Count();
// public: ZWaterBox* operator[](unsigned int);
// public: const ZWaterBox* operator[](unsigned int);
// public: ZWaterBox* operator[](ZGEOM*);
// public: const ZWaterBox* operator[](ZGEOM*);
// private: ZWaterBox* Find(ZGEOM*);
// private: ZWaterBox* Find(unsigned int);

        // members
        REFTAB32 m_WaterBoxList;
    };
    RE_VERIFY_SIZE(ZWaterBoxManager, 0xBC); // Verified PC allocation
}