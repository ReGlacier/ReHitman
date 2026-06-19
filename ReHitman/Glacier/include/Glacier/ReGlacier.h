#pragma once


#define RE_VERIFY_SIZE(cls, expected) static_assert(sizeof(cls) == (expected), "Unexpected size of " #cls " (expected " #expected " byte(s))")
#define RE_VERIFY_OFFSET(cls, mbr, expected) static_assert(offsetof(cls, mbr) == expected, "Unexpected offset of " #cls "::" #mbr " (expected +" #expected ")")
#define PAD_CONCAT_IMPL(x, y) x##y
#define PAD_CONCAT(x, y) PAD_CONCAT_IMPL(x, y)
#define RE_ADD_PADDING(size) uint8_t PAD_CONCAT(_pad_, __COUNTER__)[size]


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