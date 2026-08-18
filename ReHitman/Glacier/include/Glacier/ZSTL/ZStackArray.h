#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZUniAssert.h>
#include <cstdint>


namespace Glacier
{
	template <size_t CAPACITY, typename T>
	struct ZStackArray
	{
		uint32_t m_lNrEntries;
		T m_Array[CAPACITY];

		ZStackArray()
		{
			Clear();
		}

		uint32_t Count() const 
		{ 
			return m_lNrEntries; 
		}

		bool IsEmpty() const
		{
			return Count() == 0;
		}

		uint32_t TotalNrEntries() const
		{
			return CAPACITY;
		}

		void Clear() 
		{ 
			m_lNrEntries = 0; 
		}

		uint32_t Add(T* pElem)
		{
			ZASSERT(m_lNrEntries < CAPACITY);
			m_Array[m_lNrEntries] = *pElem;
			m_lNrEntries++;
			ZASSERT(m_lNrEntries <= CAPACITY);
			return m_lNrEntries - 1;
		}

		uint32_t Push(T elem)
		{
			return Add(&elem); 
		}

		T* Get(uint32_t index)
		{
			ZASSERT(index < m_lNrEntries);
			return &m_Array[index];
		}

		const T* Get(uint32_t index) const
		{
			ZASSERT(index < m_lNrEntries);
			return &m_Array[index];
		}

		void Remove(uint32_t index)
		{
			ZASSERT(m_lNrEntries > 0 && m_lNrEntries <= CAPACITY);
			ZASSERT(index < m_lNrEntries);

			uint32_t lastIndex = m_lNrEntries - 1;
			m_lNrEntries = lastIndex;

			m_Array[index] = m_Array[lastIndex];
		}

		int32_t Find(const T* pElem) const
		{
			ZASSERT(m_lNrEntries <= CAPACITY);

			for (uint32_t i = 0; i < m_lNrEntries; ++i)
			{
				if (m_Array[i] == *pElem)
				{
					return static_cast<int32_t>(i);
				}
			}

			return -1;
		}

		T* Peek()
		{
			uint32_t count = Count();
			return Get(count - 1); 
		}

		const T* Peek() const
		{
			return Get(Count() - 1);
		}

		T Pop()
		{
			ZASSERT(Count() > 0);

			uint32_t lastIndex = Count() - 1;
			T value = *Get(lastIndex);
			Remove(lastIndex);

			return value;
		}
	};

	template <size_t CAPACITY, typename T, typename TKey>
	struct ZStackArrayInsert : public ZStackArray<CAPACITY, T>
	{
		TKey m_SortValues[CAPACITY];

		TKey GetSortValue(uint32_t index) const
		{
			ZASSERT(index < this->m_lNrEntries);
			return m_SortValues[index];
		}

		uint32_t Add(const T* pElem, TKey key)
		{
			ZASSERT(this->m_lNrEntries < CAPACITY);

			uint32_t targetIndex = this->m_lNrEntries;

			for (uint32_t i = 0; i < this->m_lNrEntries; ++i)
			{
				if (m_SortValues[i] < key)
				{
					targetIndex = i;
					break;
				}
			}

			for (uint32_t j = this->m_lNrEntries; j > targetIndex; --j)
			{
				m_SortValues[j] = m_SortValues[j - 1];
				this->m_Array[j] = this->m_Array[j - 1];
			}

			m_SortValues[targetIndex] = key;
			this->m_Array[targetIndex] = *pElem;
			this->m_lNrEntries++;

			return targetIndex;
		}

		void Remove(uint32_t index)
		{
			ZASSERT(index < this->m_lNrEntries);

			uint32_t elementsToMove = this->m_lNrEntries - index - 1;

			if (elementsToMove > 0)
			{
				memmove(&this->m_Array[index], &this->m_Array[index + 1], elementsToMove * sizeof(T));
				memmove(&m_SortValues[index], &m_SortValues[index + 1], elementsToMove * sizeof(TKey));
			}

			this->m_lNrEntries--;
		}
	};

	template <size_t CAPACITY, typename T> 
    struct ZSafeStackArray : public ZStackArray<CAPACITY, T> 
	{
		T PopSafe()
		{
			if (this->Count() < 2)
			{
				return *(this->Peek());
			}
			else
			{
				return this->Pop();
			}
		}
	};
}