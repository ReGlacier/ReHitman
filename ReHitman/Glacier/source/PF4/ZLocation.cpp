#include <Glacier/PF4/ZLocation.h>
#include <Glacier/Serializer/ISerializerStream.h>


namespace Glacier::PF4
{
    ZLocation::ZLocation()
    {
        Reset();
    }

    ZLocation::ZLocation(ZLocation& copy)
    {
        m_Component = copy.m_Component;
        m_Graph = copy.m_Graph;
        m_Inside = copy.m_Inside;
        m_vPos = copy.m_vPos;
    }

    ZLocation::ZLocation(const ZVector3& vPos, int16_t Graph, int16_t Component, bool bInside)
    {
        PADDING = 0;
        Reset();
        Set(vPos, Graph, Component, bInside);
    }

    void ZLocation::LoadSave(ISerializerStream& stream, bool bSaving)
    {
        (void)bSaving;

        stream.Exchange("m_Component", m_Component);
        stream.Exchange("m_Graph", m_Graph);

        int16_t inside = m_Inside;
        stream.Exchange("m_Inside", inside);
        m_Inside = static_cast<uint8_t>(inside);

        stream.ExchangeArray("m_vPos", &m_vPos.x, 3);
    }

    void ZLocation::Reset()
    {
        m_Component = -1;
        m_Inside = 0;
        m_Graph = -1;
    }

    void ZLocation::Set(const ZVector3& vPos, int16_t Graph, int16_t Component, bool bInside)
    {
        m_vPos = vPos;

        m_Graph = Graph;
        if (bInside)
        {
            m_Inside = 1;
        }
        else
        {
            m_Inside = 0;
        }
        m_Component = Component;
    }

    int16_t ZLocation::Component() const
    {
        return m_Component;
    }

    int16_t ZLocation::Graph() const
    {
        return m_Graph;
    }

    uint8_t ZLocation::Inside() const
    {
        return m_Inside;
    }

    ZLocation& ZLocation::operator=(const ZLocation& rhs)
    {
        m_Component = rhs.m_Component;
        m_Graph = rhs.m_Graph;
        m_Inside = rhs.m_Inside;
        PADDING = rhs.PADDING;
        m_vPos = rhs.m_vPos;
        return *this;
    }
}
