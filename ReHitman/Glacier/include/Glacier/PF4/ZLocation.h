#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>

namespace Glacier
{
    struct ISerializerStream;
}

namespace Glacier::PF4
{
    struct ZLocation
    {
        // methods
        ZLocation();
        ZLocation(ZLocation& copy);
        ZLocation(const ZVector3& vPos, int16_t Graph, int16_t Component, bool bInside);
        void LoadSave(ISerializerStream& stream, bool bSaving);
        void Reset();
        void Set(const ZVector3& vPos, int16_t Graph, int16_t Component, bool bInside);
        int16_t Component() const;
        int16_t Graph() const;
        uint8_t Inside() const;
        void SetComponent(int16_t Component);
        void SetGraph(int16_t Graph);
        void SetInside(uint8_t Inside);
        ZLocation& operator=(const ZLocation& rhs);

        // members
        int16_t m_Component{0};
        int16_t m_Graph{0};
        uint8_t m_Inside{0};
        uint8_t PADDING{0};
        ZVector3 m_vPos{};
    };
    RE_VERIFY_SIZE(ZLocation, 0x14);
}
