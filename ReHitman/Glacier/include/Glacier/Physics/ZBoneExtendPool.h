#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Component/ZComponentSingleton.h>
#include <Glacier/Component/ZRuntimeComponentBase.h>


namespace Glacier
{
    // Fwds
    class ZBoneExtend;

    class ZBoneExtendPool : public ZComponentSingleton<ZBoneExtendPool, ZRuntimeComponentBase>
    {
    public:
        // vtbl
        ~ZBoneExtendPool() override;

        // methods
        ZBoneExtendPool();
        bool Create(uint16_t total);

        ZBoneExtend& operator[](uint16_t index);

        // members
        ZBoneExtend* m_pBXPool{nullptr};
        uint16_t m_wCount{0};
    };
}