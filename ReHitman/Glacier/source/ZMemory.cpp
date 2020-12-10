#include <G1ConfigurationService.h>
#include <Glacier/ZSTL/ZMemory.h>

#include <stdexcept>

namespace Glacier
{
    void* UseManagedAllocator::operator new(size_t sz)
    {
        if (G1ConfigurationService::G1API_FunctionAddress_ZSysMem_Alloc != G1ConfigurationService::kNotConfiguredOption)
        {
            return ((void*(__cdecl*)(size_t, const char*, int))(G1ConfigurationService::G1API_FunctionAddress_ZSysMem_Alloc))(sz, __FILE__, __LINE__);
        }
        else
        {
            throw std::runtime_error { "UseManagedAllocator Fatal Error: The variable G1ConfigurationService::G1API_FunctionAddress_ZSysMem_Alloc not configured!" };
        }
    }

    void UseManagedAllocator::operator delete(void* ptr) noexcept
    {
        if (G1ConfigurationService::G1API_FunctionAddress_ZSysMem_Free != G1ConfigurationService::kNotConfiguredOption)
        {
            ((void(*)(void*))(G1ConfigurationService::G1API_FunctionAddress_ZSysMem_Free))(ptr);
        }
    }
}