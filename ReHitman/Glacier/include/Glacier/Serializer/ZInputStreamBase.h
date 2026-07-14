#pragma once

#include <cstdint>


namespace Glacier
{
    struct ZInputStreamBase
    {
        // vtbl
        virtual ~ZInputStreamBase();
        virtual uint32_t ReadRaw(char* address, const uint32_t size) = 0;
        virtual uint32_t ReadChangeEndianness(char * address, const uint32_t size, const uint32_t mask) = 0;

        // methods
        uint32_t Read(char* address, uint32_t count);

        template <typename T>
        uint32_t Read(T* address, uint32_t count)
        {
            return Read(reinterpret_cast<char*>(address), count * sizeof(T));
        }

        template <typename T>
        T Get()
        {
            T value{};
            Read(&value, 1);
            return value;
        }

        template <typename T>
        T GetAndChangeEndiannessIfRequired()
        {
            T value {};
            if (m_ChangeEndianness)
            {
                ReadChangeEndianness((char*)&value, sizeof(T), 3u);
            }
            else
            {
                ReadRaw((char*)&value, sizeof(T));
            }
            return value;
        }

        void SetBigEndian(bool bigEndian);

        // members
        bool m_ChangeEndianness;
    };
}
