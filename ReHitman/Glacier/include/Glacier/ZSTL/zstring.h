#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>

namespace Glacier
{
    /**
     * @brief Small owning string class used by Glacier serializer/UI code.
     *
     * zstring stores a null-terminated byte string in a heap buffer owned by the object.
     * It intentionally mirrors the original 12-byte layout and avoids std::string so the
     * type remains binary-compatible with game structures.
     */
    class zstring
    {
    public:
        // methods
        /** @brief Releases the owned character buffer. */
        ~zstring();
        /** @brief Constructs an empty string. */
        zstring();
        /** @brief Constructs a copy of another zstring. */
        zstring(const zstring& copy);
        /** @brief Constructs a substring from @p copy starting at offset with length count. */
        zstring(const zstring& copy, uint32_t, uint32_t);
        /** @brief Constructs from a null-terminated C string. */
        zstring(const char* pCString);
        /** @brief Constructs from the first @p iLength bytes of a C string. */
        zstring(const char* pCString, uint32_t iLength);
        /** @brief Converts ASCII lowercase letters in-place to uppercase. */
        void to_upper();
        /** @brief Compares this string with another zstring. */
        bool operator==(const zstring& rhs) const;
        /** @brief Compares this string with a null-terminated C string. */
        bool operator==(const char* pCStr) const;
        /** @brief Replaces this string with a copy of @p str. */
        zstring& operator=(const zstring& str);
        /** @brief Formats text into this string using printf-style formatting. */
        void format(const char* fmt, ...);
        /** @brief Returns true when this string has the same contents as @p rhs. */
        bool equal(const zstring& rhs) const;
        /** @brief Returns true when the first @p num bytes match @p rhs. */
        bool equal(const zstring& rhs, uint32_t num) const;
        /** @brief Counts occurrences of @p ch in this string. */
        uint32_t count(char ch) const;
        /** @brief Returns true when the first @p num bytes compare equal to @p str. */
        bool compare(const zstring& str, uint32_t num) const;
        /** @brief Returns true when this string compares equal to @p str. */
        bool compare(const zstring& str) const;
        /** @brief Returns a null-terminated character buffer. */
        const char* c_str() const;
        /** @brief Returns the string length excluding the null terminator. */
        uint32_t length() const;
        /** @brief Returns true when the string is empty. */
        bool empty() const;

        // members
        /** @brief Owned null-terminated character buffer. */
        char* m_pData;
        /** @brief Length in bytes excluding the null terminator. */
        uint32_t m_iLength;
        /** @brief Allocated character capacity excluding the null terminator. */
        uint32_t m_iCapacity;
    };
    RE_VERIFY_SIZE(zstring, 0xC);
}
