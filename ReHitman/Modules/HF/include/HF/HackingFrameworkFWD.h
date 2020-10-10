#pragma once

#include <memory>

namespace HF::Win32
{
    class Module; using ModuleRef = std::weak_ptr<Module>; using ModulePtr = std::shared_ptr<Module>;
    class Process; using ProcessRef = std::weak_ptr<Process>; using ProcessPtr = std::shared_ptr<Process>;
}