#include <Glacier/ZSTL/MYSTR.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZUniAssert.h>
#include <charconv>
#include <cstring>
#include <cstdlib>
#include <system_error>


namespace Glacier
{
    inline const char* SafeStr(const char* str) 
    {
        return str ? str : "";
    }

    MYSTR::MYSTR()
    {
        String = m_Buffer;
        m_Buffer[0] = '\0';
    }

    MYSTR::MYSTR(const char* InStr)
    {
        SetString(InStr);
    }

    MYSTR::MYSTR(const MYSTR& InStr)
    {
        SetString(SafeStr(InStr.String));
    }

    MYSTR::~MYSTR()
    {
        if (String != &m_Buffer[0])
        {
            ZUniMemory::Free((void*)String);
        }
    }

    void MYSTR::SetString(const char* InStr)
    {
        InStr = SafeStr(InStr);

        auto iLength = std::strlen(InStr);

        if (String && String != &m_Buffer[0])
        {
            ZUniMemory::Free((void*)String);
        }

        if (iLength < 124)
        {
            String = m_Buffer;
        }
        else
        {
            String = (char*)ZUniMemory::Allocate(iLength+1);
        }

        std::memcpy(String, InStr, iLength + 1);
    }

    int MYSTR::Length() const
    {
        if (!String) return 0;
        return static_cast<int>(std::strlen(String));
    }

    void MYSTR::ToLower()
    {
        if (!String) return;

        const auto iLength = Length();
        for (size_t i = 0; i < iLength; ++i)
        {
            if (String[i] >= 'A' && String[i] <= 'Z')
            {
                String[i] = String[i] + 32;
            }
        }
    }

    MYSTR& MYSTR::operator=(const MYSTR& InStr)
    {
        if (this != &InStr)
        {
            if (String != m_Buffer)
            {
                ZUniMemory::Free((void*)String);
                String = m_Buffer;
            }

            SetString(SafeStr(InStr.String));
        }
        return *this;
    }

    MYSTR& MYSTR::operator=(int nr)
    {
        return AssignSignedInteger(nr);
    }

    MYSTR& MYSTR::operator=(bool value)
    {
        return AssignSignedInteger(value ? 1 : 0);
    }

    MYSTR& MYSTR::operator=(float value)
    {
        return AssignFloatingPoint(value);
    }

    MYSTR& MYSTR::AssignSignedInteger(long long value)
    {
        char buffer[32];
        auto result = std::to_chars(buffer, buffer + sizeof(buffer) - 1, value);

        ZASSERT(result.ec == std::errc());
        if (result.ec != std::errc())
            return SetString(""), *this;

        *result.ptr = '\0';
        SetString(buffer);
        return *this;
    }

    MYSTR& MYSTR::AssignUnsignedInteger(unsigned long long value)
    {
        char buffer[32];
        auto result = std::to_chars(buffer, buffer + sizeof(buffer) - 1, value);

        ZASSERT(result.ec == std::errc());
        if (result.ec != std::errc())
            return SetString(""), *this;

        *result.ptr = '\0';
        SetString(buffer);
        return *this;
    }

    MYSTR& MYSTR::AssignFloatingPoint(double value)
    {
        char buffer[124];
        auto result = std::to_chars(buffer, buffer + sizeof(buffer) - 1, value);

        ZASSERT(result.ec == std::errc());
        if (result.ec != std::errc())
            return SetString(""), *this;

        *result.ptr = '\0';
        SetString(buffer);
        return *this;
    }

    MYSTR& MYSTR::operator+=(const MYSTR& InStr)
    {
        const char* lhsStr = SafeStr(String);
        const char* rhsStr = SafeStr(InStr.String);

        size_t lhsLen = std::strlen(lhsStr);
        size_t rhsLen = std::strlen(rhsStr);
        size_t totalLen = lhsLen + rhsLen;

        if (totalLen < 124)
        {
            std::memcpy(m_Buffer + lhsLen, rhsStr, rhsLen + 1);
            String = m_Buffer;
        }
        else
        {
            char* newStr = static_cast<char*>(ZUniMemory::Allocate(totalLen + 1));
            
            std::memcpy(newStr, lhsStr, lhsLen);
            std::memcpy(newStr + lhsLen, rhsStr, rhsLen + 1);

            if (String != m_Buffer)
            {
                ZUniMemory::Free((void*)String);
            }

            String = newStr;
        }

        return *this;
    }

    MYSTR operator+(const MYSTR& lhs, const MYSTR& rhs)
    {
        MYSTR result(lhs);
        result += rhs;
        
        return result;
    }
}
