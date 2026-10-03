#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/IDynamicSingleton.h>


namespace Glacier
{
    class ISaveMemoryManager : public IDynamicSingleton<ISaveMemoryManager>
    {
    private:
        // vtbl
        virtual void* AllocMemory(int lSize) = 0;
        virtual void FreeMemory(void* ptr) = 0;

    public:
        // methods
        ISaveMemoryManager();
        ~ISaveMemoryManager();

        static void* Alloc(int lSize);
        static void Free(void* ptr);
        
        // members
    };

}