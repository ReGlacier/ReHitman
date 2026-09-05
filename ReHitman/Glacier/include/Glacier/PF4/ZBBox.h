#pragma once

#include <Glacier/ZSTL/ZMath.h>


namespace Glacier::PF4
{
    class ZBBox
    {
    public:
        // methods
        ZBBox();

        void reset();
        const ZVector3& min() const;
        const ZVector3& max() const;
        float GetLengthOfLongestAxis() const;
        ZVector3 Center() const;
        float thickness() const;
        ZVector3 FindFastestWayOut(const ZVector3& vStartPoint);
        void FindShortestAxisAlignedExit(const ZVector3&, bool&, bool&);
        void expand(const ZVector3& vSize);
        bool Contains2D(const ZBBox& vBox) const;
        bool Contains(const ZBBox& vBox) const;
        bool Contains(const ZVector3& vPoint, float fSize) const;
        bool AxisOverlap(float, float, float, float) const;
        bool Overlap(const ZBBox& vBox) const;
        bool Inside(const float* vPoint) const;
        bool Inside(const float* vPoint, float) const;
        bool InsideXZ(float x, float z) const;
        float GetTop() const;
        float GetBottom() const;

        // members
        ZVector3 m_Min;
        ZVector3 m_Max;
    };

}
