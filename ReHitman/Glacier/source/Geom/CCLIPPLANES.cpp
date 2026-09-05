#include <Glacier/Geom/CCLIPPLANES.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZUniAssert.h>
#include <cstring>


namespace Glacier
{
    CCLIPPLANES::CCLIPPLANES()
    {
        m_lNrActiveClipPlanes = 0;
        memset(m_ActiveClipPlanes, 0, sizeof(m_ActiveClipPlanes));
    }

    CCLIPPLANES::~CCLIPPLANES()
    {
        // Do nothing
    }

    void CCLIPPLANES::Init()
    {
        m_lNrActiveClipPlanes = 0;
        memset(m_ActiveClipPlanes, 0, sizeof(m_ActiveClipPlanes));
    }

    void CCLIPPLANES::Add(float* pPlanes, uint32_t lPlanesNr)
    {
        ZASSERT(lPlanesNr == m_lNrActiveClipPlanes);

        const float fLength = vnorm(pPlanes);
        pPlanes[3] *= fLength;

        m_ActiveClipPlanes[m_lNrActiveClipPlanes + 1] = m_lNrActiveClipPlanes;
        ++m_ActiveClipPlanes[0];
        memcpy(&m_ClipPlanes[4 * m_lNrActiveClipPlanes], pPlanes, 4 * sizeof(float));
        ++m_lNrActiveClipPlanes;
    }

    uint32_t CCLIPPLANES::GetClipPlanes(float* pPlanes) const
    {
        for (uint32_t i = 0; i != m_ActiveClipPlanes[0]; ++i)
        {
            memcpy(pPlanes, &m_ClipPlanes[4 * m_ActiveClipPlanes[i + 1]], 4 * sizeof(float));
            pPlanes += 4;
        }

        return m_ActiveClipPlanes[0];
    }
}
