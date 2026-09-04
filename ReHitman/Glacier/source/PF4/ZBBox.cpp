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
        // TODO: Finish me
        return 0.0f;
    }

    ZVector3 ZBBox::Center() const
    {
        return (m_Max - m_Min) * .5f;
    }

    float ZBBox::thickness() const
    {
        // TODO: Finish me
        return 0.0f;
    }

    const ZVector3 ZBBox::FindFastestWayOut(const ZVector3& vStartPoint)
    {
        // TODO: Finish me
        return {};
    }

    void ZBBox::FindShortestAxisAlignedExit(const ZVector3&, bool&, bool&)
    {
        // TODO: Finish me
    }

    void ZBBox::expand(const ZVector3& vSize)
    {
        // TODO: Finish me
    }

    bool ZBBox::Contains2D(const ZBBox& vBox) const
    {
        // TODO: Finish me
        return false;
    }

    bool ZBBox::Contains(const ZBBox& vBox) const
    {
        // TODO: Finish me
        return false;
    }

    bool ZBBox::AxisOverlap(float, float, float, float) const
    {
        // TODO: Finish me
        return false;
    }

    bool ZBBox::Overlap(const ZBBox& vBox) const
    {
        // TODO: Finish me
        return false;
    }

    bool ZBBox::Inside(const float* vPoint) const
    {
        // TODO: Finish me
        return false;
    }

    bool ZBBox::Inside(const float* vPoint, float) const
    {
        // TODO: Finish me
        return false;
    }

    bool ZBBox::InsideXZ(float x, float z) const
    {
        // TODO: Finish me
        return false;
    }

    float ZBBox::GetTop() const
    {
        // TODO: Finish me
        return 0.f;
    }

    float ZBBox::GetBottom() const
    {
        // TODO: Finish me
        return 0.f;
    }
}
