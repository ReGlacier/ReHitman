#pragma once

#include <Glacier/Component/Fwds.h>
#include <cstdint>

namespace Glacier
{
    using ZComponentId = const char*;
    using ZComponentGroupId = uint16_t;

    inline constexpr ZComponentGroupId ZCOMPONENT_GROUP_COUNT = 4;

    using ZComponentCreator = ZComponentBase*(*)();
    using ZComponentDestructor = void(*)(ZComponentBase*);
    using ZComponentSetInstance = void(*)(ZComponentBase*);
}
