#pragma once


namespace Glacier::SysInput
{
    using DeviceHandler = int(*)(unsigned int, int, void*);
    using TimeoutCallback = void(*)();
}