#pragma once

#include <Glacier/ZSTL/EAllocType.h>
#include <utility>
#include <memory>


/**
 * This class is not a member of Glacier engine!
 * This is global accessor to unified memory alloc/dealloc
 * In testing env it uses std::malloc/std::free
 * In game env it uses ZSysMem for malloc/free
 */
struct ZUniMemory
{
    static void* Allocate(int bytes);
    static void* Allocate(int bytes, Glacier::EAllocType eAllocType);
    static void Free(void* ptr);

    template <typename T, typename... TArgs>
    static T* New(TArgs&&... args)
    {
        void* ptr = Allocate(sizeof(T));
        new (ptr) T(std::forward<TArgs>(args)...);
        return reinterpret_cast<T*>(ptr);
    }

    template <typename T>
    static T* NewArray(size_t count)
    {
        void** ptr = (void**)Allocate(sizeof(T) * count);
        for (int i = 0; i < count; ++i)
        {
            new (ptr[i]) T();
        }

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

    template <typename T>
    static void DeleteArray(T* ptr, size_t count)
    {
        if (!ptr || !count) return;

        T* p = ptr;
        for (int i = 0; i < count; ++i)
        {
            p->~T();
            p++;
        }

        Free(ptr);
    }
};

#ifdef REHITMAN_TESTS
#   define STATIC_GLOBAL_VAR(type, name, addr, def) static type name = def;
#   define STATIC_CLASS_VAR(cls, type, name) static type name;
#   define STATIC_CLASS_VAR_ARRAY(cls, type, name, size) static type name[size];
#   define STATIC_CLASS_VAR_IMPL(cls, type, name, addr, default_value) type cls::name = default_value;
#   define STATIC_CLASS_VAR_ARRAY_IMPL(cls, type, name, size, addr) type cls::name[size] = {};
#   define STATIC_GLOBAL_CLASS_INSTANCE(type, name) extern type name;
#   define STATIC_GLOBAL_CLASS_INSTANCE_IMPL(type, name, addr, default_value) type name = default_value;
#   define STATIC_GLOBAL_ARRAY(type, size, name, address, ...) static const type name[size] = __VA_ARGS__;
#else
#   define STATIC_GLOBAL_VAR(type, name, addr, def) static type& name = *reinterpret_cast<type*>(addr)
#   define STATIC_CLASS_VAR(cls, type, name) static type& name;
#   define STATIC_CLASS_VAR_ARRAY(cls, type, name, size) static type (&name)[size];
#   define STATIC_CLASS_VAR_IMPL(cls, type, name, addr, default_value) type& cls::name = *reinterpret_cast<type*>(addr);
#   define STATIC_CLASS_VAR_ARRAY_IMPL(cls, type, name, size, addr) type (&cls::name)[size] = *reinterpret_cast<type(*)[size]>(addr);
#   define STATIC_GLOBAL_CLASS_INSTANCE(type, name) extern type& name;
#   define STATIC_GLOBAL_CLASS_INSTANCE_IMPL(type, name, addr, default_value) type& name = *reinterpret_cast<type*>(addr);
#   define STATIC_GLOBAL_ARRAY(type, size, name, address, ...) static const type (&name)[size] = *reinterpret_cast<const type(*)[size]>(address);
#endif


// For any build
#define STATIC_CLASS_VAR_EZ(cls, type, name) static type name;
#define STATIC_CLASS_VAR_IMPL_EZ(cls, type, name, default_value) type cls::name = default_value;


template <typename T, typename... TArgs>
T* znew_placement(T* ptr, TArgs&&... args)
{
    if (!ptr)
        return nullptr;

    std::construct_at(ptr, std::forward<TArgs>(args)...);
    return ptr;
}
