#pragma once

#include <utility>


/**
 * This class is not a member of Glacier engine!
 * This is global accessor to unified memory alloc/dealloc
 * In testing env it uses std::malloc/std::free
 * In game env it uses ZSysMem for malloc/free
 */
struct ZUniMemory
{
    static void* Allocate(int bytes);
    static void Free(void* ptr);

    template <typename T, typename... TArgs>
    static T* New(TArgs&&... args)
    {
        void* ptr = Allocate(sizeof(T));
        new (ptr) T(std::forward<TArgs>(args)...);
        return reinterpret_cast<T*>(ptr);
    }

    template <typename T>
    static void Delete(T* ptr)
    {
        if (!ptr) return;
        
        if constexpr (std::is_destructible_v<T>) 
        {
            ptr->~T(); 
        }

        Free(ptr);
    }
};