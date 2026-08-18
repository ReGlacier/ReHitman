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

        static int CapsuleSphereCollision(ZVector3& vDir, float& fLen, const ZVector3& vp0, const ZVector3& cp1, const float& cr, const ZVector3& sc, const float& sr);

        /**
         * @brief Adjusts two particle positions to satisfy a distance constraint using rigid body projection.
         * @details Computes the delta between particles, normalizes it, and distributes the correction
         *          based on inverse masses. If particles are too close (< 1e-6), applies a small random
         *          perturbation to avoid division by zero.
         *
         * @param x1   Position of the first particle (in/out).
         * @param x2   Position of the second particle (in/out).
         * @param m1   Inverse mass of the first particle.
         * @param m2   Inverse mass of the second particle.
         * @param dist Target rest distance between the particles.
         */
        static void AdjustPart2rigid(float* x1, float* x2, float m1, float m2, float dist);

        /**
         * @brief Solves a 3x3 linear system using Cramer's rule.
         * @param base2   3x3 matrix as array of 9 floats (column-major).
         * @param target2_global Right-hand side vector (3 floats).
         * @param lincomb Output solution vector (3 floats).
         * @return true if the system has a unique solution, false if determinant is near zero.
         */
        static bool Solve3x3System(const float* base2, const float* target2_global, float* lincomb);

        /**
         * @brief Computes the distance from a point to a line segment and the closest point direction.
         * @param a1      Point to test.
         * @param b1      Line segment start.
         * @param b2      Line segment end.
         * @param t       Output parameter for closest point on segment [0, 1].
         * @param fMinDist Minimum distance threshold (input/output).
         * @param fDist   Output actual distance.
         * @param vDir    Output normalized direction from closest point to a1.
         * @return true if distance is within fMinDist, false otherwise.
         */
        static bool DistPointLineVar2(const float* a1, const float* b1, const float* b2,
                                      float* t, float* fMinDist, float* fDist, float* vDir);

        /**
         * @brief Generates a random unit vector using the system random seed.
         * @param v Output vector (3 floats).
         */
        static void RandomUnitVector(float* v);

        /**
         * @brief Tests intersection between a line segment and a triangle.
         * @param coli_x   Output intersection point.
         * @param orig     Line segment origin.
         * @param dir      Line segment direction.
         * @param vert0    Triangle vertex 0.
         * @param inv      Inverse matrix for barycentric coordinates.
         * @param t        Output intersection parameter.
         * @param bBothSides If true, tests both sides of the triangle.
         * @return true if intersection occurs within [0, 1], false otherwise.
         */
        static bool IntersectTriangleAndLine3(float* coli_x, const float* orig, const float* dir,
                                              const float* vert0, const float* inv, float* t, bool bBothSides);

        /**
         * @brief Tests intersection between a sphere and a triangle.
         * @param triverts    Triangle vertices (3 x 3 floats).
         * @param spherepos   Sphere center position.
         * @param fRadius     Sphere radius.
         * @param vDir0       Output direction from sphere to triangle.
         * @param fScaledDist Output scaled distance.
         * @param a6          Output penetration depth.
         * @return true if intersection occurs, false otherwise.
         */
        static bool IntersectTriangleAndSphere(const float (*triverts)[3], const float (*spherepos)[3],
                                               float fRadius, float (*vDir0)[3], float* fScaledDist, float* a6);
    };
}