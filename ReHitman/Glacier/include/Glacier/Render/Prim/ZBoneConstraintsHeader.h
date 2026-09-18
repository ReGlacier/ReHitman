#pragma once

#include <Glacier/ReGlacier.h>
#include <type_traits>
#include <cstdint>


namespace Glacier
{
    class ZBoneConstraint;

    class ZBoneConstraintsHeader
    {
    public:
        // methods
        const ZBoneConstraint* First() { return reinterpret_cast<const ZBoneConstraint*>(this + 1); }
        uint32_t Size() const { return m_lNrConstraints; }

        template <typename T = ZBoneConstraint>
        const T* Get() const requires (std::is_base_of_v<ZBoneConstraint, T>)
        {
            return reinterpret_cast<const T*>(reinterpret_cast<const uint8_t*>(this) + sizeof(ZBoneConstraintsHeader));
        }

        // members
        uint32_t m_lNrConstraints;
    };
    RE_VERIFY_SIZE(ZBoneConstraintsHeader, 0x4);
}
