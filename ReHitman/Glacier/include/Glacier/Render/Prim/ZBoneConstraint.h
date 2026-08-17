#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    class ZBoneConstraint
    {
    public:
        // methods
        ZBoneConstraint* ZBoneConstraint::Next()
        {
            const auto* pByte = reinterpret_cast<const uint8_t*>(this);
            return const_cast<ZBoneConstraint*>(reinterpret_cast<const ZBoneConstraint*>(pByte + m_lStructSize));
        }

        const ZBoneConstraint* ZBoneConstraint::Next() const
        {
            const auto* pByte = reinterpret_cast<const uint8_t*>(this);
            return reinterpret_cast<const ZBoneConstraint*>(pByte + m_lStructSize);
        }
        
        // members
        uint8_t m_lType;
        uint8_t m_lStructSize;
        uint8_t m_lBoneIndex;
        RE_ADD_PADDING(1);
    };
    RE_VERIFY_SIZE(ZBoneConstraint, 4);
}