#pragma once


#define RE_VERIFY_SIZE(cls, expected) static_assert(sizeof(cls) == (expected), "Unexpected size of " #cls " (expected " #expected " byte(s))")
#define RE_VERIFY_OFFSET(cls, mbr, expected) static_assert(offsetof(cls, mbr) == expected, "Unexpected offset of " #cls "::" #mbr " (expected +" #expected ")")


// Вспомогательные макросы для склейки имени со счетчиком
#define PAD_CONCAT_IMPL(x, y) x##y
#define PAD_CONCAT(x, y) PAD_CONCAT_IMPL(x, y)

// Финальный макрос: создает массив байт с уникальным именем
#define RE_ADD_PADDING(size) uint8_t PAD_CONCAT(_pad_, __COUNTER__)[size]