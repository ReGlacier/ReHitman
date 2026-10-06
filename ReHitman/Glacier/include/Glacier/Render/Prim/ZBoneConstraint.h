#pragma once

#include <Glacier/ReGlacier.h>
#include <type_traits>
#include <cstdint>


namespace Glacier
{
    class ZBoneConstraint
    {
    public:
        // methods
        ZBoneConstraint* Next()
        {
            auto* pByte = reinterpret_cast<uint8_t*>(this);
            return const_cast<ZBoneConstraint*>(reinterpret_cast<ZBoneConstraint*>(pByte + m_lStructSize));
        }

        const ZBoneConstraint* Next() const
        {
            const auto* pByte = reinterpret_cast<const uint8_t*>(this);
            return reinterpret_cast<const ZBoneConstraint*>(pByte + m_lStructSize);
        }

        template <typename T>
        const T* As() const requires (std::is_base_of_v<ZBoneConstraint, T>)
        {
            return reinterpret_cast<const T*>(this);
        }

        template <typename T>
        T* As() requires (std::is_base_of_v<ZBoneConstraint, T>)
        {
            return reinterpret_cast<T*>(this);
        }

        // members
        uint8_t m_lType;
        uint8_t m_lStructSize;
        uint8_t m_lBoneIndex;
        RE_ADD_PADDING(1);
    };
    RE_VERIFY_SIZE(ZBoneConstraint, 4);
}
