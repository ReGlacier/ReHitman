#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/System/ZDllBase.h>

namespace Glacier
{
    class ZDllMain : public ZDllBase
    {
    public:
        void Init() override;
        void End() override;

        static ZDllMain* BuildInstance();
    };

    RE_VERIFY_SIZE(ZDllMain, 0xC);
}
