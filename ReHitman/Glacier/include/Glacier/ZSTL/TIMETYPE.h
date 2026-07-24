#pragma once

namespace Glacier
{
    /**
     * @brief Fixed-point game time value used by Glacier systems.
     *
     * TIMETYPE stores seconds as 1024 ticks per second. Integer conversion returns
     * whole seconds, while float conversion preserves the fractional fixed-point part.
     */
    struct TIMETYPE
    {
        // constants
        /** @brief Number of fixed-point ticks in one second. */
        static constexpr float kTicksPerSecond = 1024.f;
        /** @brief Multiplier used to convert stored ticks back to seconds. */
        static constexpr float kInvTPS = 1.f / kTicksPerSecond;

        // fields
        /** @brief Stored fixed-point tick value. */
        int secs { 0 };

        // constructors
        /** @brief Creates a zero time value. */
        TIMETYPE() = default;
        /** @brief Creates a time value from whole seconds. */
        TIMETYPE(int sec) : secs(sec << 10) {}
        /** @brief Creates a time value from floating-point seconds. */
        TIMETYPE(float sec) : secs(convert(sec)) {}
        /** @brief Copies the raw fixed-point value from another TIMETYPE. */
        TIMETYPE(const TIMETYPE& tp) = default;

        // operators
        /** @brief Copies the raw fixed-point value from another TIMETYPE. */
        TIMETYPE& operator=(const TIMETYPE& tp) { if (this != &tp) secs = tp.secs; return *this; }

        bool operator==(const TIMETYPE& rhs) const 
        { 
            return secs == rhs.secs; 
        }
        
        bool operator!=(const TIMETYPE& rhs) const
        {
            return secs != rhs.secs; 
        }
        
        bool operator==(int lTime) const 
        {
            TIMETYPE tmp { lTime };
            return tmp == *this;
        }

        bool operator!=(int lTime) const { return !operator==(lTime); }
        bool operator<(const TIMETYPE& rhs) const { return secs < rhs.secs; }
        bool operator>(const TIMETYPE& rhs) const  { return rhs < *this; }
        bool operator<=(const TIMETYPE& rhs) const { return !(*this > rhs); }
        bool operator>=(const TIMETYPE& rhs) const { return !(*this < rhs); }
        /** @brief Converts to whole seconds by truncating the fixed-point fraction. */
        operator int() const
        {
            return secs >> 10;
        }

        // converters & funcs
        /** @brief Converts to seconds as a floating-point value. */
        operator float() const 
        { 
            return (float)secs * kInvTPS; 
        }

        /** @brief Returns true when the stored fixed-point value is non-zero. */
        operator bool() const
        {
            return secs != 0;
        }

        /**
         * @brief Converts floating-point seconds to fixed-point ticks.
         *
         * @param sec Seconds to convert.
         * @return Fixed-point tick value rounded according to the platform's float-to-int conversion.
         */
        [[nodiscard]] int convert(float sec) const
        {
            return (int)(float)(sec * kTicksPerSecond);
        }

        TIMETYPE operator-(const TIMETYPE& rhs) const
        {
            return { secs - rhs.secs };
        }
    };
}
