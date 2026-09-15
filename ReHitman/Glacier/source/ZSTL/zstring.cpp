#include <Glacier/ZSTL/zstring.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>

#include <cstdarg>
#include <cstdio>
#include <cstring>


namespace Glacier
{
    namespace
    {
        constexpr uint32_t kFormatBufferSize = 1024;

        unsigned char ToUpper(unsigned char value)
        {
            if (value >= 'a' && value <= 'z')
                return static_cast<unsigned char>(value - ('a' - 'A'));

            return value;
        }

        char* AllocateString(uint32_t capacity)
        {
            char* data = static_cast<char*>(ZUniMemory::Allocate(static_cast<int>(capacity + 1)));
            data[0] = '\0';
            return data;
        }
    }

    zstring::~zstring()
    {
        ZUniMemory::Free(m_pData);
        m_pData = nullptr;
        m_iLength = 0;
        m_iCapacity = 0;
    }

    zstring::zstring()
    {
        m_pData = AllocateString(0);
        m_iLength = 0;
        m_iCapacity = 0;
    }

    zstring::zstring(const zstring& copy)
    {
        m_iLength = copy.m_iLength;
        m_iCapacity = copy.m_iLength;
        m_pData = AllocateString(m_iCapacity);
        std::memcpy(m_pData, copy.c_str(), m_iLength + 1);
    }

    zstring::zstring(const zstring& copy, uint32_t offset, uint32_t count)
    {
        if (offset > copy.m_iLength)
            offset = copy.m_iLength;

        uint32_t available = copy.m_iLength - offset;
        if (count > available)
            count = available;

        m_iLength = count;
        m_iCapacity = count;
        m_pData = AllocateString(m_iCapacity);
        std::memcpy(m_pData, copy.c_str() + offset, count);
        m_pData[count] = '\0';
    }

    zstring::zstring(const char* pCString)
    {
        if (!pCString)
            pCString = "";

        m_iLength = static_cast<uint32_t>(std::strlen(pCString));
        m_iCapacity = m_iLength;
        m_pData = AllocateString(m_iCapacity);
        std::memcpy(m_pData, pCString, m_iLength + 1);
    }

    zstring::zstring(const char* pCString, uint32_t iLength)
    {
        if (!pCString)
        {
            pCString = "";
            iLength = 0;
        }

        m_iLength = iLength;
        m_iCapacity = iLength;
        m_pData = AllocateString(m_iCapacity);
        std::memcpy(m_pData, pCString, iLength);
        m_pData[iLength] = '\0';
    }

    void zstring::to_upper()
    {
        for (uint32_t i = 0; i < m_iLength; ++i)
            m_pData[i] = static_cast<char>(ToUpper(static_cast<unsigned char>(m_pData[i])));
    }

    bool zstring::operator==(const zstring& rhs) const
    {
        return equal(rhs);
    }

    bool zstring::operator==(const char* pCStr) const
    {
        zstring rhs(pCStr);
        return equal(rhs);
    }

    bool zstring::operator<(const zstring& rhs) const
    {
        return std::strcmp(c_str(), rhs.c_str()) < 0;
    }

    zstring& zstring::operator=(const zstring& str)
    {
        if (this == &str)
            return *this;

        char* newData = AllocateString(str.m_iLength);
        std::memcpy(newData, str.c_str(), str.m_iLength + 1);

        ZUniMemory::Free(m_pData);
        m_pData = newData;
        m_iLength = str.m_iLength;
        m_iCapacity = str.m_iLength;

        return *this;
    }

    zstring operator+(const zstring& lhs, const char* rhs)
    {
        if (!rhs)
            rhs = "";

        const uint32_t lhsLen = lhs.m_iLength;
        const uint32_t rhsLen = static_cast<uint32_t>(std::strlen(rhs));

        zstring result;
        ZUniMemory::Free(result.m_pData);
        result.m_iLength = lhsLen + rhsLen;
        result.m_iCapacity = result.m_iLength;
        result.m_pData = AllocateString(result.m_iCapacity);
        std::memcpy(result.m_pData, lhs.c_str(), lhsLen);
        std::memcpy(result.m_pData + lhsLen, rhs, rhsLen + 1);
        return result;
    }

    zstring operator+(const char* lhs, const zstring& rhs)
    {
        if (!lhs)
            lhs = "";

        const uint32_t lhsLen = static_cast<uint32_t>(std::strlen(lhs));
        const uint32_t rhsLen = rhs.m_iLength;

        zstring result;
        ZUniMemory::Free(result.m_pData);
        result.m_iLength = lhsLen + rhsLen;
        result.m_iCapacity = result.m_iLength;
        result.m_pData = AllocateString(result.m_iCapacity);
        std::memcpy(result.m_pData, lhs, lhsLen);
        std::memcpy(result.m_pData + lhsLen, rhs.c_str(), rhsLen + 1);
        return result;
    }

    void zstring::format(const char* fmt, ...)
    {
        char buffer[kFormatBufferSize + 1]{};

        va_list args;
        va_start(args, fmt);
        int written = std::vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);

        ZASSERT(written >= 0);
        ZASSERT(static_cast<uint32_t>(written) < sizeof(buffer));

        zstring formatted(buffer);
        *this = formatted;
    }

    bool zstring::equal(const zstring& rhs) const
    {
        return std::strcmp(c_str(), rhs.c_str()) == 0;
    }

    bool zstring::equal(const zstring& rhs, uint32_t num) const
    {
        return std::strncmp(c_str(), rhs.c_str(), num) == 0;
    }

    uint32_t zstring::count(char ch) const
    {
        uint32_t result = 0;

        for (uint32_t i = 0; i < m_iLength; ++i)
        {
            if (m_pData[i] == ch)
                ++result;
        }

        return result;
    }

    bool zstring::compare(const zstring& str, uint32_t num) const
    {
        return std::strncmp(c_str(), str.c_str(), num) == 0;
    }

    bool zstring::compare(const zstring& str) const
    {
        return std::strcmp(c_str(), str.c_str()) == 0;
    }

    const char* zstring::c_str() const
    {
        return m_pData ? m_pData : "";
    }

    uint32_t zstring::length() const
    {
        return m_iLength;
    }

    bool zstring::empty() const
    {
        return m_iLength == 0;
    }
}
