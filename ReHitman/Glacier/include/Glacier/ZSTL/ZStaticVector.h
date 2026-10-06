#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZUniAssert.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <utility>

namespace Glacier
{
    /**
     * @brief Fixed-capacity vector compatible with the original std::vectorS<T, N>.
     *
     * @tparam T Element type stored inline.
     * @tparam N Maximum number of elements.
     *
     * @details ZStaticVector owns no heap memory. It stores a current size followed by
     * inline storage for exactly N elements, matching the original binary layout used by
     * Glacier. The container is intended for trivially relocatable game structs and small
     * pointer/value lists; operations shift elements by assignment.
     */
    template <typename T, size_t N>
    struct ZStaticVector
    {
        // members
        uint32_t m_iSize;
        T m_Data[N];

        // methods
        ZStaticVector() : m_iSize(0) {}
        ZStaticVector(const ZStaticVector& other) : m_iSize(other.m_iSize)
        {
            std::copy(other.begin(), other.end(), begin());
        }

        ZStaticVector& operator=(const ZStaticVector& other)
        {
            if (this != &other)
            {
                m_iSize = other.m_iSize;
                std::copy(other.begin(), other.end(), begin());
            }

            return *this;
        }

        uint32_t size() const { return m_iSize; }
        static constexpr size_t capacity() { return N; }
        bool empty() const { return size() == 0; }
        [[nodiscard]] bool IsEmpty() const { return empty(); }

        T& operator[](size_t i) { return m_Data[i]; }
        const T& operator[](size_t i) const { return m_Data[i]; }

        T* begin() { return m_Data; }
        const T* begin() const { return m_Data; }
        const T* cbegin() const { return m_Data; }
        T* end() { return m_Data + m_iSize; }
        const T* end() const { return m_Data + m_iSize; }
        const T* cend() const { return m_Data + m_iSize; }

        void clear() { m_iSize = 0; }

        void push_back(const T& value)
        {
            ZASSERT(m_iSize < N);
            m_Data[m_iSize++] = value;
        }

        uint32_t pop_back()
        {
            uint32_t oldSize = size();

            if (oldSize)
                --m_iSize;

            return oldSize;
        }

        T* erase(uint32_t iIndex)
        {
            if (iIndex == m_iSize)
                return &m_Data[m_iSize];

            for (uint32_t i = iIndex; i < m_iSize - 1; ++i)
                m_Data[i] = m_Data[i + 1];

            --m_iSize;
            return &m_Data[iIndex];
        }

        T* erase(T* first, T* last)
        {
            return erase(static_cast<uint32_t>(first - begin()), static_cast<uint32_t>(last - begin()));
        }

        T* erase(uint32_t first, uint32_t last)
        {
            if (first == last)
                return &m_Data[first];

            uint32_t count = last - first;
            for (uint32_t i = first; i + count < m_iSize; ++i)
                m_Data[i] = m_Data[i + count];

            m_iSize -= count;
            return &m_Data[first];
        }
    };
}

namespace std
{
    template <typename T, size_t N>
    using vectorS = Glacier::ZStaticVector<T, N>;
}
