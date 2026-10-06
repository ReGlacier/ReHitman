#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{

    class CCLIPPLANES
    {
    public:
        // methods
        CCLIPPLANES();
        ~CCLIPPLANES();

        void Init();
        void Add(float* pPlanes, uint32_t lPlanesNr);
        uint32_t GetClipPlanes(float* pPlanes) const;

        // members
        uint32_t m_lNrActiveClipPlanes;
        uint32_t m_ActiveClipPlanes[13];
        float m_ClipPlanes[48];
    };
    RE_VERIFY_SIZE(CCLIPPLANES, 0xF8);
}
