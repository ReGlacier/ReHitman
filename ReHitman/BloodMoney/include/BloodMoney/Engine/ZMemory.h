#pragma once

#include <BloodMoney/Game/Globals.h>

namespace Hitman::BloodMoney
{
    using ZSysMemAllocateFn = void*(__cdecl*)(size_t allocationSize, const char* allocationSource, int line);
    using ZSysMemFreeFn = void(__cdecl*)(void* ptr);

    static ZSysMemAllocateFn ZSysMem_Allocate = (ZSysMemAllocateFn)Hitman::BloodMoney::Globals::kZSysMemAllocAddr;
    static ZSysMemFreeFn ZSysMem_Free = (ZSysMemFreeFn)Hitman::BloodMoney::Globals::kZSysMemFreeAddr;
}

#define G1_ALLOC(t) ZSysMem_Allocate(sizeof(t), __FILE__, __LINE__);