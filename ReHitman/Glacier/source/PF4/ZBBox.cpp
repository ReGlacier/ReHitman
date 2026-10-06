#include <Glacier/PF4/ZBBox.h>


namespace Glacier::PF4
{
    ZBBox::ZBBox()
        : m_Min()
        , m_Max()
    {
    }

    void ZBBox::reset()
    {
        m_Min.Reset();
        m_Max.Reset();
    }

    const ZVector3& ZBBox::min() const
    {
        return m_Min;
    }

    const ZVector3& ZBBox::max() const
    {
        return m_Max;
    }

    float ZBBox::GetLengthOfLongestAxis() const
    {
        return std::max({fabsf(m_Max.x - m_Min.x), fabsf(m_Max.y - m_Min.y), fabsf(m_Max.z - m_Min.z)});
    }

    ZVector3 ZBBox::Center() const
    {
        return (m_Max - m_Min) * .5f;
    }

    float ZBBox::thickness() const
    {
        return std::min({fabsf(m_Max.x - m_Min.x), fabsf(m_Max.y - m_Min.y), fabsf(m_Max.z - m_Min.z)});
    }

    ZVector3 ZBBox::FindFastestWayOut(const ZVector3& vStartPoint)
    {
        ZVector3 vOut = vStartPoint;
        if (vOut.x < m_Min.x)
            vOut.x = m_Min.x;
        else if (vOut.x > m_Max.x)
            vOut.x = m_Max.x;
        if (vOut.y < m_Min.y)
            vOut.y = m_Min.y;
        else if (vOut.y > m_Max.y)
            vOut.y = m_Max.y;
        if (vOut.z < m_Min.z)
            vOut.z = m_Min.z;
        else if (vOut.z > m_Max.z)
            vOut.z = m_Max.z;
        return vOut;
    }

    void ZBBox::FindShortestAxisAlignedExit(const ZVector3& vPoint, bool& bX, bool& bZ)
    {
        float fDX, fDZ;
        if (vPoint.x < m_Min.x)
            fDX = fabsf(vPoint.x - m_Min.x);
        else
            fDX = fabsf(vPoint.x - m_Max.x);
        if (vPoint.z < m_Min.z)
            fDZ = fabsf(vPoint.z - m_Min.z);
        else
            fDZ = fabsf(vPoint.z - m_Max.z);
        bX = fDX <= fDZ;
        bZ = !bX;
    }

    void ZBBox::expand(const ZVector3& vSize)
    {
        float fOx = 1.f, fOy = 100.f, fOz = 1.f;
        m_Min.x = std::min(m_Min.x, vSize.x - fOx);
        m_Min.y = std::min(m_Min.y, vSize.y - fOy);
        m_Min.z = std::min(m_Min.z, vSize.z - fOz);
        m_Max.x = std::max(m_Max.x, vSize.x + fOx);
        m_Max.y = std::max(m_Max.y, vSize.y + fOy);
        m_Max.z = std::max(m_Max.z, vSize.z + fOz);
    }

    bool ZBBox::Contains2D(const ZBBox& vBox) const
    {
        return m_Min.x <= vBox.m_Min.x && vBox.m_Max.x <= m_Max.x
            && m_Min.z <= vBox.m_Min.z && vBox.m_Max.z <= m_Max.z;
    }

    bool ZBBox::Contains(const ZBBox& vBox) const
    {
        return m_Min.x <= vBox.m_Min.x && vBox.m_Max.x <= m_Max.x
            && m_Min.y <= vBox.m_Min.y && vBox.m_Max.y <= m_Max.y
            && m_Min.z <= vBox.m_Min.z && vBox.m_Max.z <= m_Max.z;
    }

    bool ZBBox::Contains(const ZVector3& vPoint, float fSize) const
    {
        return fSize + vPoint.x >= m_Min.x
            && vPoint.x - fSize <= m_Max.x
            && fSize + vPoint.y >= m_Min.y
            && vPoint.y - fSize <= m_Max.y
            && fSize + vPoint.z >= m_Min.z
            && vPoint.z - fSize <= m_Max.z;
    }

    bool ZBBox::AxisOverlap(float fMin1, float fMax1, float fMin2, float fMax2) const
    {
        return fMin1 <= fMax2 && fMin2 <= fMax1;
    }

    bool ZBBox::Overlap(const ZBBox& vBox) const
    {
        return AxisOverlap(m_Min.x, m_Max.x, vBox.m_Min.x, vBox.m_Max.x)
            && AxisOverlap(m_Min.y, m_Max.y, vBox.m_Min.y, vBox.m_Max.y)
            && AxisOverlap(m_Min.z, m_Max.z, vBox.m_Min.z, vBox.m_Max.z);
    }

    bool ZBBox::Inside(const float* vPoint) const
    {
        return m_Min.x <= vPoint[0] && vPoint[0] <= m_Max.x
            && m_Min.y <= vPoint[1] && vPoint[1] <= m_Max.y
            && m_Min.z <= vPoint[2] && vPoint[2] <= m_Max.z;
    }

    bool ZBBox::Inside(const float* vPoint, float fSize) const
    {
        return m_Min.x <= vPoint[0] + fSize && vPoint[0] - fSize <= m_Max.x
            && m_Min.y <= vPoint[1] + fSize && vPoint[1] - fSize <= m_Max.y
            && m_Min.z <= vPoint[2] + fSize && vPoint[2] - fSize <= m_Max.z;
    }

    bool ZBBox::InsideXZ(float x, float z) const
    {
        return m_Min.x < x && x < m_Max.x && m_Min.z < z && z < m_Max.z;
    }

    float ZBBox::GetTop() const
    {
        return m_Max.z;
    }

    float ZBBox::GetBottom() const
    {
        return m_Min.z;
    }
}
