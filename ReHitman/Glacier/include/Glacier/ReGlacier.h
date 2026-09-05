#pragma once

#ifndef RE_VERIFY_SIZE
#   define RE_VERIFY_SIZE(cls, expected) static_assert(sizeof(cls) == (expected), "Unexpected size of " #cls " (expected " #expected " byte(s))")
#endif

#ifndef RE_VERIFY_OFFSET
#   include <cstddef>
#   define RE_VERIFY_OFFSET(cls, mbr, expected) static_assert(offsetof(cls, mbr) == expected, "Unexpected offset of " #cls "::" #mbr " (expected +" #expected ")")
#endif

#define PAD_CONCAT_IMPL(x, y) x##y
#define PAD_CONCAT(x, y) PAD_CONCAT_IMPL(x, y)
#define RE_ADD_PADDING(size) uint8_t PAD_CONCAT(_pad_, __COUNTER__)[size]
#define STATIC_ARR_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

#define RE_VERIFY_VTBL_INDEX(cls, method, expected_index) \
    { \
        auto ptr = &cls::method; \
        union { \
            decltype(ptr) m_ptr; \
            unsigned int f_offset; \
        } u; \
        u.m_ptr = ptr; \
        unsigned int actual_index = u.f_offset / sizeof(void*); \
        assert(actual_index == expected_index && "VTBL index mismatch for " #cls "::" #method); \
    }

#if defined(_MSC_VER)
#	define RE_PACKED_STRUCT(al) __pragma(pack(push, 1))
#	define RE_PACKED_STRUCT_END __pragma(pack(pop))
#elif defined(__GNUC__) || defined(__clang__)
#	define RE_PACKED_STRUCT(al)
#	define RE_PACKED_STRUCT_END __attribute__((packed))
#else
#	define RE_PACKED_STRUCT(al)
#	define RE_PACKED_STRUCT_END
#endif

namespace Glacier
{
    template <typename T>
    T* fuck_cast(const void* ptr)
    {
        return const_cast<T*>(reinterpret_cast<const T*>(ptr));
    }
}
