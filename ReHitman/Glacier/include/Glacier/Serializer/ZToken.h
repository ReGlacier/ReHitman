#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>

#if __cplusplus >= 202002L
#   include <compare>
#endif


namespace Glacier
{
    /**
     * @brief Small serializer token identifier.
     *
     * ZToken is a 32-bit value type used by serializer APIs to identify named fields or
     * special serializer markers. The original engine defines three negative sentinel
     * tokens: Void (-1), Unknown (-2), and Joker (-3).
     */
    struct ZToken
    {
        // constants
        static const ZToken Void;
        static const ZToken Unknown;
        static const ZToken Joker;

        // members
        int32_t m_Token;

        // methods
        constexpr ZToken() : m_Token(-1) {}
        constexpr ZToken(int32_t iValue) : m_Token(iValue) {}
        constexpr ZToken(const ZToken&) = default;

        ZToken& operator=(const ZToken&) = default;

#if __cplusplus >= 202002L
        constexpr auto operator<=>(const ZToken&) const = default;
#else
        constexpr bool operator>(const ZToken& rhs) const { return m_Token > rhs.m_Token; }
        constexpr bool operator==(const ZToken& rhs) const { return m_Token == rhs.m_Token; }
        constexpr bool operator<=(const ZToken& rhs) const { return m_Token <= rhs.m_Token; }
        constexpr bool operator<(const ZToken& rhs) const { return m_Token < rhs.m_Token; }
#endif

        ZToken& operator++()
        {
            ++m_Token;
            return *this;
        }

        constexpr operator int32_t() const { return m_Token; }
    };
    inline constexpr ZToken ZToken::Void{ -1 };
    inline constexpr ZToken ZToken::Unknown{ -2 };
    inline constexpr ZToken ZToken::Joker{ -3 };
    RE_VERIFY_SIZE(ZToken, 0x4); // Verified
}
