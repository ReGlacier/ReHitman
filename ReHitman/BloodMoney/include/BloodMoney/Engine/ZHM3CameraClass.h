#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZMath.h>
#include <BloodMoney/Engine/ZHM3CameraClassBase.h>

namespace Hitman::BloodMoney
{
    class ZHM3CameraClass : public ZHM3CameraClassBase
    {
    public:
        // vftable
        virtual void InitializeView();

        // data (total size is 0x1AC, ZHM3CameraClassBase size is 0x1A8)
        int m_field1A8;
    };
}