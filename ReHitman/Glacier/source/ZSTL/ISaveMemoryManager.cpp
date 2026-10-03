#include <Glacier/ZSTL/ISaveMemoryManager.h>


namespace Glacier
{
    ISaveMemoryManager::ISaveMemoryManager() = default;
    ISaveMemoryManager::~ISaveMemoryManager() = default;

    void* ISaveMemoryManager::Alloc(int lSize)
    {
        return ISaveMemoryManager::Instance().AllocMemory(lSize);
    }

    void ISaveMemoryManager::Free(void* ptr)
    {
        ISaveMemoryManager::Instance().FreeMemory(ptr);
    }
}