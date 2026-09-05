#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>

namespace Glacier
{
    /**
     * @brief Fixed-capacity path string that normalizes backslashes to forward slashes.
     *
     * ZFilePath mirrors the original 264-byte engine layout: a fixed 260 byte
     * character buffer followed by a 4 byte length. It is used to normalize and
     * compare file paths without heap allocation, keeping the type binary-compatible
     * with game structures.
     */
    class ZFilePath
    {
    public:
        // methods
        /** @brief Constructs an empty path. */
        ZFilePath();
        /** @brief Constructs a path from a null-terminated C string (asserts non-null). */
        ZFilePath(const char* pPath);
        /** @brief Constructs a copy of another path. */
        ZFilePath(const ZFilePath& copy);
        /** @brief Copies another path into this one. */
        ZFilePath& operator=(const ZFilePath& copy);

        /** @brief Resets the path to an empty string. */
        void Clear();
        /** @brief Appends another path, inserting a '/' separator as needed. */
        void Append(const ZFilePath& path);

        /** @brief Returns the path as a null-terminated character buffer. */
        const char* AsChar() const;
        /** @brief Returns the number of characters excluding the null terminator. */
        uint32_t CharCount() const;
        /** @brief Returns true when the path is empty. */
        bool IsEmpty() const;

    private:
        // methods
        /** @brief Copies @p pPath, converting backslashes to forward slashes. */
        void SetValue(const char* pPath);

        // members
        char m_Buffer[260];   // +0x00
        uint32_t m_Length;    // +0x104
    };
    RE_VERIFY_SIZE(ZFilePath, 0x108);
}
