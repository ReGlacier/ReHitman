#include <Glacier/Fysix/ZCommonAlgorithms.h>


namespace Glacier
{
    namespace 
    {
        void ShortestDistancePointLineSegment(ZVector3& x, const ZVector3& p, const ZVector3& p0, const ZVector3& p1)
        {
            float dx = p1.x - p0.x;
            float dy = p1.y - p0.y;
            float dz = p1.z - p0.z;

            float lenSq = dx * dx + dy * dy + dz * dz;

            float t = 0.0f;
            if (lenSq > 0.0f)
            {
                float dot = dx * (p.x - p0.x) + dy * (p.y - p0.y) + dz * (p.z - p0.z);
                t = dot / lenSq;

                if (t < 0.0f) t = 0.0f;
                if (t > 1.0f) t = 1.0f;
            }

            x.x = p0.x + dx * t;
            x.y = p0.y + dy * t;
            x.z = p0.z + dz * t;
        }
    }

    int ZCommonAlgorithms::CapsuleSphereCollision(ZVector3& vDir, float& fLen, const ZVector3& cp0, const ZVector3& cp1, const float& cr, const ZVector3& sc, const float& sr)
    {
        ZVector3 closestPoint;
        ShortestDistancePointLineSegment(closestPoint, sc, cp0, cp1);

        float dx = sc.x - closestPoint.x;
        float dy = sc.y - closestPoint.y;
        float dz = sc.z - closestPoint.z;
        float distSq = dx * dx + dy * dy + dz * dz;
        float combinedRadius = cr + sr;

        if (distSq >= combinedRadius * combinedRadius)
        {
            return 0;
        }

        float dist = sqrtf(distSq);
        if (dist > 0.0f)
        {
            float invDist = 1.0f / dist;
            vDir.x = dx * invDist;
            vDir.y = dy * invDist;
            vDir.z = dz * invDist;
        }
        else
        {
            vDir.x = 0.0f;
            vDir.y = 0.0f;
            vDir.z = 0.0f;
        }

        fLen = combinedRadius - dist;
        return 1;
    }
}