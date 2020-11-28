#pragma once

#include <imgui.h>

namespace ImGui
{
    // Default impl
    template <typename T> struct Inspector { static void Draw(const char* id, T*) {} };
}