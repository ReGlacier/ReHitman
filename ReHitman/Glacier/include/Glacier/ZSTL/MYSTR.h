#pragma once

#include <Glacier/ReGlacier.h>

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
        MYSTR& operator+=(const MYSTR&);

        operator char*() const { return String; }
        operator const char*() const { return String; }
        operator bool() const { return String != nullptr; }

        friend MYSTR operator+(const MYSTR& lhs, const MYSTR& rhs);
    };
    RE_VERIFY_SIZE(MYSTR, 0x80);
}