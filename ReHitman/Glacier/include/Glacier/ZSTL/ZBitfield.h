#pragma once

#include <type_traits>
#include <cstdint>

namespace Glacier
{
	struct ZBitfieldBase
	{
		uint32_t m_Bitfield{ 0u };
	};

	template <typename T>
	struct ZBitfield : ZBitfieldBase
	{
		static_assert(std::is_enum_v<T>, "Allowed to make bitfield only from enum types");
		static_assert(sizeof(T) == sizeof(uint32_t), "Not allowed to make bitfield from non u32 type");

        ZBitfield()
        {
            this->m_Bitfield = 0u;
        }

        ZBitfield(uint32_t iInitialValue)
        {
            this->m_Bitfield = iInitialValue;
        }

        bool IsSet(T value) const
        {
            return (this->m_Bitfield & static_cast<uint32_t>(value)) != 0u;
        }

        void Set(T value)
        {
            this->m_Bitfield |= static_cast<uint32_t>(value);
        }

        void UnSet(T value)
        {
            this->m_Bitfield &= ~static_cast<uint32_t>(value);
        }

        void Reset()
        {
            this->m_Bitfield = 0u;
        }
	};
}