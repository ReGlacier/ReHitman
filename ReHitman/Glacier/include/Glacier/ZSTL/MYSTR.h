#pragma once

#include <Glacier/ReGlacier.h>
#include <type_traits>

namespace Glacier
{
    struct MYSTR
    {
        // members
        char* String{nullptr};
        char m_Buffer[124];

        // methods
        MYSTR();
        MYSTR(const char*);
        MYSTR(const MYSTR&);
        ~MYSTR();

        void SetString(const char*);
        int Length() const;
        void ToLower();

        MYSTR& operator=(const MYSTR&);
        MYSTR& operator=(int);
        MYSTR& operator=(bool);
        MYSTR& operator=(float);

        template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
        MYSTR& operator=(T value)
        {
            if constexpr (std::is_floating_point_v<T>)
                return AssignFloatingPoint(static_cast<double>(value));
            else if constexpr (std::is_signed_v<T>)
                return AssignSignedInteger(static_cast<long long>(value));
            else
                return AssignUnsignedInteger(static_cast<unsigned long long>(value));
        }

        MYSTR& operator+=(const MYSTR&);

        operator char*() const { return String; }
        operator const char*() const { return String; }
        operator bool() const { return String != nullptr; }

        friend MYSTR operator+(const MYSTR& lhs, const MYSTR& rhs);

    private:
        MYSTR& AssignSignedInteger(long long value);
        MYSTR& AssignUnsignedInteger(unsigned long long value);
        MYSTR& AssignFloatingPoint(double value);
    };
    RE_VERIFY_SIZE(MYSTR, 0x80);
}
