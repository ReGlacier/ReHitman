#pragma once

#include <Glacier/ReGlacier.h>
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

        // members
        uint32_t m_lNrConstraints;
    };
    RE_VERIFY_SIZE(ZBoneConstraintsHeader, 0x4);
}