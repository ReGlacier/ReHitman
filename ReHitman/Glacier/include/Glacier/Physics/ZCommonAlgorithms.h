#pragma once

#include <Glacier/ZSTL/ZMath.h>


namespace Glacier
{
    // fwds
    struct SRecurseInfoCompiled;
    struct SFastBoxColiTri;
    struct SCapsuleColiInfo;

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

        /**
         * @brief Tests whether a sphere (or ellipsoid) collides with a triangle (PC: 0x004364C0).
         *
         * @details The triangle is transformed into the sphere's local space, where the sphere
         * is the unit sphere (radius 1, centered at the origin), using the sphere position and
         * the inverse sphere basis matrix (basis rows pre-divided by the sphere dimensions, as
         * built by `ZCollisionWintel::CheckSphereCollision`). The test then performs:
         * 1. **Degeneracy rejection:** triangles whose squared face-normal length falls below
         *    \f$2^{-13}\f$ never collide.
         * 2. **Edge tests:** each triangle edge is tested against the unit sphere via LnSphColl().
         * 3. **Interior test:** the sphere center is projected onto the triangle plane; if the
         *    plane distance is within the unit radius and the projected point lies inside the
         *    triangle (CheckCutInside()), a collision is reported.
         *
         * @note Original engine source location: `engine/zstdlib/extpolymath.cpp`.
         *
         * @param[in] SphPos Sphere center position in world space (3 floats).
         * @param[in] SphMat Inverse sphere basis matrix, rows pre-scaled by 1/dimension (9 floats).
         * @param[in] inPos1 First triangle vertex in world space (3 floats).
         * @param[in] inPos2 Second triangle vertex in world space (3 floats).
         * @param[in] inPos3 Third triangle vertex in world space (3 floats).
         *
         * @return `true` if the sphere intersects the triangle; otherwise, `false`.
         */
        static bool PolySphColl(const float *SphPos, const float *SphMat, const float *inPos1, const float *inPos2, const float *inPos3);

        /**
         * @brief Tests whether a line segment intersects the unit sphere (PC: 0x004363D0).
         *
         * @details The segment must already be expressed in unit-sphere local space (sphere of
         * radius 1 centered at the origin). The test returns immediately when the start point
         * lies inside the sphere; otherwise it projects the sphere center onto the segment
         * direction and checks that the closest point lies on the segment and within the sphere.
         *
         * @note Original engine source location: `engine/zstdlib/extpolymath.cpp`.
         *
         * @param[in] Pos1 Segment start point in unit-sphere local space (3 floats).
         * @param[in] Pos2 Segment end point in unit-sphere local space (3 floats).
         *
         * @return `true` if the segment intersects the unit sphere; otherwise, `false`.
         */
        static bool LnSphColl(float *Pos1, float *Pos2);

        /**
         * @brief Tests whether a point on a triangle's plane lies inside the triangle (PC: 0x004362B0).
         *
         * @details The triangle and the test point are projected onto the plane orthogonal to the
         * dominant axis of the given normal (the component with the largest magnitude is dropped),
         * after which 2D half-space (edge cross-product) tests are performed against all three
         * edges. Edge tests share a common winding sign, so both triangle windings are accepted.
         * A tolerance of \f$2^{-13}\f$ keeps points exactly on an edge classified as inside.
         *
         * @note Original engine source location: `engine/zstdlib/extpolymath.cpp`.
         * @note Called by PolySphColl() with the projected sphere center as both @p pn (scaled
         *       normal direction) and @p cp (closest point on the triangle plane).
         *
         * @param[in] vertexptr First triangle vertex; the other two vertices must follow it
         *                      contiguously in memory (9 floats total).
         * @param[in] pn        Triangle plane normal; only its direction is used to select the
         *                      projection axes (3 floats).
         * @param[in] cp        Test point on (or near) the triangle plane (3 floats).
         *
         * @return `true` if the point lies inside the triangle (edges inclusive); otherwise, `false`.
         */
        static bool CheckCutInside(const float *vertexptr, const float *pn, const float *cp);

        /**
         * @brief Projects a sphere center out of a half-space defined by a plane.
         *
         * @details Displaces the sphere center along @p vPushDir by @p fPenetrationDepth
         *          to resolve a collision with the plane. Simultaneously produces a
         *          correction direction in @p vDir: if @p vPushDir and @p vDesiredNormal
         *          are in opposing hemispheres (dot product < 0), the correction is
         *          zeroed out; otherwise it equals @p vDesiredNormal.
         *
         *          The resulting center is computed as:
         *          \f[ \mathbf{c}_{\text{out}} = \mathbf{c}_{\text{in}} + \mathbf{n}_{\text{push}} \cdot d \f]
         *
         * @note Used in sphere–plane collision resolution (PolySphColl pipeline) to
         *       push a penetrating sphere back along the contact normal and expose
         *       the effective response direction for subsequent impulse calculation.
         *
         * @param[out] vCenResult          Displaced sphere center after projection (3 floats).
         * @param[in]  vSrcCenter          Original sphere center (3 floats).
         * @param[out] vDir                Effective correction direction: equals @p vDesiredNormal
         *                                 if \f$\mathbf{n}_{\text{push}} \cdot \mathbf{n}_{\text{desired}} \geq 0\f$,
         *                                 otherwise zero (3 floats).
         * @param[in]  vDesiredNormal      Desired response normal; compared against @p vPushDir
         *                                 to gate the correction (3 floats).
         * @param[in]  vPushDir            Plane normal / push direction along which the center is
         *                                 displaced (3 floats).
         * @param[in]  fPenetrationDepth   Scalar displacement magnitude (typically
         *                                 \f$r - \hat{\mathbf{n}}\cdot\mathbf{c}\f$ or equivalent).
         */
        static void ProjectSphereOutFromPlane(float* vCenResult, const float* vSrcCenter, float* vDir, const float* vDesiredNormal, const float* vPushDir, float fPenetrationDepth);

        /**
         * @brief Tests whether a capsule (swept-sphere segment) collides with a precompiled triangle (PC: 0x581E70).
         *
         * @details Clips the capsule segment against the triangle plane and its three edge
         *          perpendicular planes, then resolves the closest feature (face, edge, or vertex)
         *          and reports the contact in @p rResult.
         *
         * @param[in]  vCap0      Capsule segment start point (3 floats).
         * @param[in]  vCap1      Capsule segment end point (3 floats).
         * @param[in]  fRadius    Capsule radius.
         * @param[in]  pTriangle  Precompiled triangle (SFastBoxColiTri).
         * @param[out] rResult    Collision result: contact normal (vDir), segment parameter (t0), and scaled distance (fScaledDist).
         *
         * @return `true` if the capsule intersects the triangle; otherwise, `false`.
         */
        static bool CollideCapsuleAndTriangle(const float (&vCap0)[3], const float (&vCap1)[3], float fRadius, const SFastBoxColiTri* pTriangle, SCapsuleColiInfo& rResult);

        /**
         * @brief Computes the shortest distance between two line segments (PC: 0x5832A0).
         *
         * @details Solves for the closest-point pair on the two infinite lines, clamps the
         *          parameters to the segments, and falls back to point-vs-segment tests when a
         *          parameter lies outside [0, 1].
         *
         * @param[in]  a1       First segment start point (3 floats).
         * @param[in]  a2       First segment end point (3 floats).
         * @param[in]  b1       Second segment start point (3 floats).
         * @param[in]  b2       Second segment end point (3 floats).
         * @param[out] s        Parameter of the closest point on the first segment [0, 1].
         * @param[out] t        Parameter of the closest point on the second segment [0, 1].
         * @param[in]  fMinDist Minimum distance threshold.
         * @param[out] fDist    Output distance between the closest points.
         * @param[out] vDir     Output normalized direction from the closest point on the second segment toward the first.
         *
         * @return `true` if the distance is within fMinDist; otherwise, `false`.
         */
        static bool DistLineLineVar(
            const float* const a1,
            const float* const a2,
            const float* const b1,
            const float* const b2,
            float& s,
            float& t,
            float& fMinDist,
            float& fDist,
            float* vDir);

        /**
         * @brief Computes the distance from a point to a line segment (PC: 0x582FF0).
         *
         * @details Rejects the query when the point's projection falls outside the segment
         *          (before the start or beyond the end), unlike DistPointLineVar2 which clamps.
         *
         * @param[in]  b1       Point to test (3 floats).
         * @param[in]  a1       Line segment start (3 floats).
         * @param[in]  a2       Line segment end (3 floats).
         * @param[out] s        Output parameter of the closest point on the segment [0, 1].
         * @param[in]  fMinDist Minimum distance threshold.
         * @param[out] fDist    Output distance to the segment.
         * @param[out] vDir     Output normalized direction from the point toward the closest point on the segment.
         *
         * @return `true` if the projection is within the segment and the distance is within fMinDist; otherwise, `false`.
         */
        static bool DistPointLineVar(
            const float* const b1,
            const float* const a1,
            const float* a2,
            float& s,
            float& fMinDist,
            float& fDist,
            float* vDir);

        /**
         * @brief Computes the distance from a point to a line segment, clamping the projection (PC: 0x583140).
         *
         * @details Clamps the closest point to the segment endpoints when the point's projection
         *          lies outside [0, 1].
         *
         * @param[in]  a1       Point to test (3 floats).
         * @param[in]  b1       Line segment start (3 floats).
         * @param[in]  b2       Line segment end (3 floats).
         * @param[out] t        Output parameter of the closest point on the segment [0, 1].
         * @param[in]  fMinDist Minimum distance threshold.
         * @param[out] fDist    Output distance to the segment.
         * @param[out] vDir     Output normalized direction from the closest point on the segment toward the point.
         *
         * @return `true` if the distance is within fMinDist; otherwise, `false`.
         */
        static bool DistPointLineVar2(
            const float* const a1,
            const float* const b1,
            const float* const b2,
            float& t,
            float& fMinDist,
            float& fDist,
            float* vDir
        );

        /**
         * @brief Resolves a capsule-triangle collision by pulling the capsule endpoints out of the triangle (PC: 0x583540).
         *
         * @details Displaces the two capsule endpoints along the contact normal (weighted by the
         *          segment parameter t0) so the weighted contact point stays fixed, then damps the
         *          outgoing normal and tangential velocity of the endpoints. The contact normal in
         *          @p sColiInfo is normalized in-place.
         *
         * @param[in,out] vCap0     Capsule segment start point (3 floats).
         * @param[in,out] vCap1     Capsule segment end point (3 floats).
         * @param[in,out] vVel0     Velocity of the segment start point (3 floats).
         * @param[in,out] vVel1     Velocity of the segment end point (3 floats).
         * @param[in,out] sColiInfo Collision result: contact normal (vDir), segment parameter (t0), and scaled distance (fScaledDist).
         */
        static void PullTriangleCyl2(float* vCap0, float* vCap1, float* vVel0, float* vVel1, SCapsuleColiInfo& sColiInfo);

        /**
         * @brief Computes the distance from a point to the surface of an axis-aligned box centered at the origin (PC: 0x00428A80).
         *
         * @details The box is defined by its half-extents @p s0 and is centered at the origin. The
         *          function first derives the signed per-axis separation between the point and the box
         *          boundary:
         *          \f[
         *              d_x = |p_x| - s_x, \qquad d_y = |p_y| - s_y, \qquad d_z = |p_z| - s_z,
         *          \f]
         *          where a negative component indicates that the point lies inside the box along that
         *          axis. The returned value is the Euclidean length of the positive separations only,
         *          i.e. \f$\sqrt{\max(d_x,0)^2 + \max(d_y,0)^2 + \max(d_z,0)^2}\f$, which is the exact
         *          distance to the closest point on the box surface for any point outside the box. As an
         *          intentional quirk inherited from the original engine, a point that is inside the box on
         *          every axis is special-cased to return the (negative) Y separation @f$d_y\f$ rather than
         *          zero.
         *
         * @note Original engine source location: `engine/zstdlib/extpolymath.cpp`.
         *
         * @param[in] p0 Point position (3 floats).
         * @param[in] s0 Box half-extents (3 floats).
         *
         * @return The distance from the point to the box surface; if the point is fully inside the box,
         *         the negative Y-axis penetration @f$d_y\f$.
         */
        static float DistanceBoxAndPoint(const float* p0, const float* s0);
    };
}
