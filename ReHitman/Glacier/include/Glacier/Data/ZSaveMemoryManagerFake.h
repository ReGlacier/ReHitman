#pragma once

#include <Glacier/ZSTL/ISaveMemoryManager.h>

namespace Glacier
{
    // Minimal ISaveMemoryManager swapped into ISaveMemoryManager::m_Instance for
    // the duration of the saved-game restore performed by
    // ZEngineDataBase::AllocSequence. Instead of a dedicated save arena it routes
    // the serializer's temporary allocations straight through ZUniMemory
    // (PC: ZSysMem::allocate for AllocMemory, operator delete for FreeMemory).
    // It is meant to live on the stack; its IDynamicSingleton<ISaveMemoryManager>
    // base installs it as m_Instance on construction and restores the previous one
    // on destruction, matching PC's manual swap-in/restore.
    class ZSaveMemoryManagerFake final : public ISaveMemoryManager
    {
    private:
        // vtbl
        void* AllocMemory(int lSize) override;
        void FreeMemory(void* ptr) override;
    };

    RE_VERIFY_SIZE(ZSaveMemoryManagerFake, 0x8);
}
