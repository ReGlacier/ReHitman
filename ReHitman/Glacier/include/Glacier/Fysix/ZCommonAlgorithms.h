#pragma once

#include <Glacier/ZSTL/ZMath.h>


namespace Glacier
{
    struct SRecurseInfoCompiled;
    
    struct ZCommonAlgorithms
    {
        /**
         * @brief Performs a highly optimized 3D ray / line segment intersection test against an Axis-Aligned Bounding Box (AABB).
         * 
         * @details This function implements a variant of the classic slab-based ray-box intersection algorithm originally 
         * proposed by Kay and Kajiya [1] and further optimized for computing the entry parameter \f$t\f$ by Brian Smits [2].
         * It determines whether a directed finite line segment (defined by a start point and a direction/end vector) 
         * intersects the given AABB within the normalized time/distance interval \f$[0.0, 1.0]\f$.
         * 
         * **Algorithmic Steps:**
         * 1. **Early-Out In-Box Test:** Instantly returns `true` with `pOutT = 0.0f` if the segment's starting point lies entirely inside the AABB.
         * 2. **Slab Intersection:** Computes the near intersection parameter \f$t\f$ for the $X$, $Y$, and $Z$ parallel slab planes.
         * 3. **Interval Consolidation:** Finds the maximum \f$t\f$ value (\f$t_{max}\f$), which represents the actual point of entry into the 3D volume.
         * 4. **Boundary Validation:** Validates the computed hit point against the other two orthogonal dimensions to ensure it lands on the actual box face.
         * 5. **Segment Range Check:** Returns `true` only if \f$t_{max}\f$ falls within the valid range of the finite segment \f$[0.0, 1.0]\f$.
         * 
         * @note Original engine source location: `engine/zstdlib/extpolymath.cpp`.
         * @note Contains a diagnostic `ZASSERT` to enforce structurally valid bounding boxes (\f$min \le max\f$).
         * 
         * **References:**
         * - [1] Kay, T. L., & Kajiya, J. T. (1986). *Ray tracing complex scenes*. In ACM SIGGRAPH Computer Graphics (Vol. 20, No. 4, pp. 269-278).
         * - [2] Smits, B. (1998). *Efficiency issues for ray tracing*. Journal of Graphics Tools, 3(2), 1-14.
         * 
         * @param[in]  fMinX    Minimum X-coordinate boundary of the AABB.
         * @param[in]  fMinY    Minimum Y-coordinate boundary of the AABB.
         * @param[in]  fMinZ    Minimum Z-coordinate boundary of the AABB.
         * @param[in]  fMaxX    Maximum X-coordinate boundary of the AABB.
         * @param[in]  fMaxY    Maximum Y-coordinate boundary of the AABB.
         * @param[in]  fMaxZ    Maximum Z-coordinate boundary of the AABB.
         * @param[in]  fLinePosX Starting X-coordinate of the line segment.
         * @param[in]  fLinePosY Starting Y-coordinate of the line segment.
         * @param[in]  fLinePosZ Starting Z-coordinate of the line segment.
         * @param[in]  fLineVecX X-component of the segment's direction/length vector (End.x - Start.x).
         * @param[in]  fLineVecY Y-component of the segment's direction/length vector (End.y - Start.y).
         * @param[in]  fLineVecZ Z-component of the segment's direction/length vector (End.z - Start.z).
         * @param[out] pOutT     Pointer to a float where the intersection parameter \f$t \in [0.0, 1.0]\f$ will be stored if a hit occurs.
         * 
         * @return `true` if the line segment intersects the AABB; otherwise, `false`.
         */
        static bool LineVS_AABB(float fMinX, float fMinY, float fMinZ,
                                float fMaxX, float fMaxY, float fMaxZ,
                                float fLinePosX, float fLinePosY, float fLinePosZ,
                                float fLineVecX, float fLineVecY, float fLineVecZ,
                                float* pOutT);

        /**
         * @brief Fast 2D line segment vs AABB intersection test on the XZ plane.
         * @details Used for rapid quadtree node pruning during ray casting.
         * 
         * @param fMinX   Minimum X boundary of the 2D node.
         * @param fMinZ   Minimum Z boundary of the 2D node.
         * @param fMaxX   Maximum X boundary of the 2D node.
         * @param fMaxZ   Maximum Z boundary of the 2D node.
         * @param pInfo   Pointer to the compiled ray/intersection context.
         * @param pOutT   Output parameter for the intersection time t.
         * @return true if the 2D segment intersects the node bounds within t in [0.0, 1.0].
         */
        static bool Line2D_VS_AABB(
            float fMinX, float fMinZ, 
            float fMaxX, float fMaxZ, 
            SRecurseInfoCompiled* pInfo, 
            float* pOutT);


        static bool LineVS_AAbox(const Vector3& vStart, const Vector3& vDir, const Vector3& vMin, 
                                 const Vector3& vMax, 
                                 float& fOutT, 
                                 float fMaxT);
    };
}