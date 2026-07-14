#pragma once

#include <cstdint>


namespace Glacier
{
    struct ZOutputStreamBase
    {
        // vtbl
        virtual ~ZOutputStreamBase();
        virtual uint32_t WriteRaw(char* address, const uint32_t size) = 0;
        virtual uint32_t WriteChangeEndianness(char * address, const uint32_t size, const uint32_t mask) = 0;

        // methods
        template <typename T>
        uint32_t Write(const T& value)
        {
            return WriteRaw(reinterpret_cast<char*>(const_cast<T*>(&value)), sizeof(T));
        }

        uint32_t Write(const char* address, const uint32_t size)
        {
            return WriteRaw(const_cast<char*>(address), size);
        }

        // members
        bool m_ChangeEndianness;
    };
}
