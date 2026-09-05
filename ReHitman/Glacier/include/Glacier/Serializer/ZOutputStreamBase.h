#pragma once

#include <Glacier/Serializer/MaskUtils.h>
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

        template <typename T>
        uint32_t WriteWithEndianness(const T& value)
        {
            if (m_ChangeEndianness)
            {
                return WriteChangeEndianness(reinterpret_cast<char*>(const_cast<T*>(&value)), sizeof(T), GetEndiannessMask<T>());
            }
            else
            {
                return WriteRaw(reinterpret_cast<char*>(const_cast<T*>(&value)), sizeof(T));
            }
        }

        template <typename T>
        uint32_t WriteWithEndianness(T* pBegin, size_t lSize)
        {
            if (m_ChangeEndianness)
            {
                return WriteChangeEndianness(reinterpret_cast<char*>(pBegin), sizeof(T) * lSize, GetEndiannessMask<T>());
            }
            else
            {
                return WriteRaw(reinterpret_cast<char*>(pBegin), sizeof(T) * lSize);
            }
        }

        uint32_t Write(const char* address, const uint32_t size)
        {
            return WriteRaw(const_cast<char*>(address), size);
        }

        void SetBigEndian(bool bigEndian);

        // members
        bool m_ChangeEndianness;
    };
}
