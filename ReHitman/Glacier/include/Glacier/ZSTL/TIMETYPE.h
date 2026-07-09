#pragma once

namespace Glacier
{
    struct TIMETYPE
    {
        // constants
        static constexpr float kTicksPerSecond = 1024.f; // In Glacier we have 1024 ticks per second for gameplay logic
        static constexpr float kInvTPS = 1.f / kTicksPerSecond;

        // fields
        int secs { 0 };

        // operators
        TIMETYPE& operator=(TIMETYPE& tp) { secs = tp.secs; return *this;}

        // converters & funcs
        operator float() const 
        { 
            return (float)secs * kInvTPS; 
        }

        operator bool() const
        {
            return secs != 0;
        }

        [[nodiscard]] int convert(float sec) const
        {
            return (int)(float)(sec * kTicksPerSecond);
        }
    };
}