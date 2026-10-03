#pragma once

#include <Glacier/ZSTL/ZSimpleArray.h>


namespace Glacier
{
    template <typename T>
    struct TDynamicArray : public ZSimpleArray<T>
    {
        // vtbl (no vtbl)
        
        // methods
        TDynamicArray(const T& DefaultValue) 
            : ZSimpleArray<T>()
            , m_Size(0)
            , m_DefaultValue(DefaultValue)
        {
        }

        uint32_t GetSize() const
        {
            return m_Size;
        }

        uint32_t GetCapacity() const
        {
            return ZSimpleArray<T>::GetSize();
        }

        void SetCapacity(uint32_t capacity)
        {
            ZSimpleArray<T>::Resize(capacity, m_DefaultValue);

            if (capacity < m_Size)
                m_Size = capacity;
        }

        void Grow(uint32_t index)
        {
            if (index < m_Size)
                return;

            if (index >= GetCapacity())
                SetCapacity(index + 1);

            m_Size = index + 1;
        }

        T& operator[](uint32_t index)
        {
            Grow(index);
            return ZSimpleArray<T>::operator[](index);
        }

        const T& operator[](uint32_t index) const
        {
            return ZSimpleArray<T>::operator[](index);
        }

        // members
        uint32_t m_Size; // Yep, another size... IOI, why so may sizes? Why so many arrays? What the hell??? Are you enjoying to make bicycles every time?
        const T m_DefaultValue; // And because of that TDynamicArray<T> MUST be default constructible... crap
    };
}
