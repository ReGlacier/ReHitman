#include <Glacier/Render/Prim/ZPrimAccessStandardWeightedMeshD3D.h>
#include <Glacier/ZUniMemory.h>
#include <cstring>


namespace Glacier
{
    namespace
    {
        // PC 0x0049CF90
        uint32_t PackNormalComponent(float v)
        {
            const int lValue = static_cast<int>((v + 1.0f) * 127.5f);
            if (lValue < 0)
            {
                return 0;
            }
            if (lValue > 255)
            {
                return 255;
            }
            return static_cast<uint32_t>(lValue);
        }

        // PC 0x0049CF90
        void PackNormal(uint32_t* pdwNormal, float x, float y, float z)
        {
            *pdwNormal = (PackNormalComponent(x) << 16)
                       | (PackNormalComponent(y) << 8)
                       | PackNormalComponent(z);
        }

        // PC 0x0049B8E0
        void UnpackNormal(float* px, float* py, float* pz, uint32_t dwNormal)
        {
            *px = static_cast<float>((dwNormal >> 16) & 0xFF) * (1.0f / 127.5f) - 1.0f;
            *py = static_cast<float>((dwNormal >> 8) & 0xFF) * (1.0f / 127.5f) - 1.0f;
            *pz = static_cast<float>(dwNormal & 0xFF) * (1.0f / 127.5f) - 1.0f;
        }
    }

    ZPrimAccessStandardWeightedMeshD3D::ZPrimAccessStandardWeightedMeshD3D(uint32_t hPrim)
        : ZPrimAccessMeshWeighted()
    {
        m_hPrim.m_lHandleValue = hPrim;
        m_lStatusFlags = 0;
        m_pAllocatedVertices = nullptr;
        m_pAllocatedIndices = nullptr;
    }

    ZPrimAccessStandardWeightedMeshD3D::~ZPrimAccessStandardWeightedMeshD3D()
    {
        if (m_pAllocatedVertices)
        {
            ZUniMemory::Free(m_pAllocatedVertices);
            m_pAllocatedVertices = nullptr;
        }

        if (m_pAllocatedIndices)
        {
            ZUniMemory::Free(m_pAllocatedIndices);
            m_pAllocatedIndices = nullptr;
        }
    }

    ZPrimAccessMesh* ZPrimAccessStandardWeightedMeshD3D::CreateEditable(uint32_t lNumTriangles, uint32_t lNumVertices)
    {
        auto* pResult = static_cast<ZPrimAccessStandardWeightedMeshD3D*>(Create(m_hPrim));
        pResult->Allocate(lNumTriangles, lNumVertices);

        return pResult;
    }

    void ZPrimAccessStandardWeightedMeshD3D::GetPositions(uint32_t lStartVertex, uint32_t lNumVertices, float* pfPosition)
    {
        ZASSERT(lStartVertex + lNumVertices <= GetNumVertices());

        const auto* pVertices = reinterpret_cast<const SVertexW4WintelDP3*>(GetVerticesConst());
        for (uint32_t i = 0; i < lNumVertices; ++i)
        {
            const ZVector3& vPosition = pVertices[lStartVertex + i].p;
            pfPosition[0] = vPosition.x;
            pfPosition[1] = vPosition.y;
            pfPosition[2] = vPosition.z;
            pfPosition += 3;
        }
    }

    void ZPrimAccessStandardWeightedMeshD3D::SetPositions(uint32_t lStartVertex, uint32_t lNumVertices, const float* pfPosition)
    {
        ZASSERT(lStartVertex + lNumVertices <= GetNumVertices());

        auto* pVertices = reinterpret_cast<SVertexW4WintelDP3*>(const_cast<uint32_t*>(GetVertices()));
        for (uint32_t i = 0; i < lNumVertices; ++i)
        {
            ZVector3& vPosition = pVertices[lStartVertex + i].p;
            vPosition.x = pfPosition[0];
            vPosition.y = pfPosition[1];
            vPosition.z = pfPosition[2];
            pfPosition += 3;
        }
    }

    void ZPrimAccessStandardWeightedMeshD3D::GetNormals(uint32_t lStartVertex, uint32_t lNumVertices, float* pfNormal)
    {
        ZASSERT(lStartVertex + lNumVertices <= GetNumVertices());

        const auto* pVertices = reinterpret_cast<const SVertexW4WintelDP3*>(GetVerticesConst());
        for (uint32_t i = 0; i < lNumVertices; ++i)
        {
            UnpackNormal(&pfNormal[0], &pfNormal[1], &pfNormal[2], pVertices[lStartVertex + i].n);
            pfNormal += 3;
        }
    }

    void ZPrimAccessStandardWeightedMeshD3D::SetNormals(uint32_t lStartVertex, uint32_t lNumVertices, const float* pfNormal)
    {
        ZASSERT(lStartVertex + lNumVertices <= GetNumVertices());

        auto* pVertices = reinterpret_cast<SVertexW4WintelDP3*>(const_cast<uint32_t*>(GetVertices()));
        for (uint32_t i = 0; i < lNumVertices; ++i)
        {
            PackNormal(&pVertices[lStartVertex + i].n, pfNormal[0], pfNormal[1], pfNormal[2]);
            pfNormal += 3;
        }
    }

    void ZPrimAccessStandardWeightedMeshD3D::GetColors(uint32_t lStartVertex, uint32_t lNumVertices, uint32_t* plColors)
    {
        ZASSERT(lStartVertex + lNumVertices <= GetNumVertices());

        const auto* pVertices = reinterpret_cast<const SVertexW4WintelDP3*>(GetVerticesConst());
        for (uint32_t i = 0; i < lNumVertices; ++i)
        {
            plColors[i] = pVertices[lStartVertex + i].c;
        }
    }

    void ZPrimAccessStandardWeightedMeshD3D::SetColors(uint32_t lStartVertex, uint32_t lNumVertices, const uint32_t* plColors)
    {
        ZASSERT(lStartVertex + lNumVertices <= GetNumVertices());

        auto* pVertices = reinterpret_cast<SVertexW4WintelDP3*>(const_cast<uint32_t*>(GetVertices()));
        for (uint32_t i = 0; i < lNumVertices; ++i)
        {
            pVertices[lStartVertex + i].c = plColors[i];
        }
    }

    void ZPrimAccessStandardWeightedMeshD3D::GetTexCoords(uint32_t lStartVertex, uint32_t lNumVertices, float* pfTexCoords)
    {
        ZASSERT(lStartVertex + lNumVertices <= GetNumVertices());

        const auto* pVertices = reinterpret_cast<const SVertexW4WintelDP3*>(GetVerticesConst());
        for (uint32_t i = 0; i < lNumVertices; ++i)
        {
            const ZVector2& vTexCoords = pVertices[lStartVertex + i].t;
            pfTexCoords[0] = vTexCoords.x;
            pfTexCoords[1] = vTexCoords.y;
            pfTexCoords += 2;
        }
    }

    void ZPrimAccessStandardWeightedMeshD3D::SetTexCoords(uint32_t lStartVertex, uint32_t lNumVertices, const float* pfTexCoords)
    {
        ZASSERT(lStartVertex + lNumVertices <= GetNumVertices());

        auto* pVertices = reinterpret_cast<SVertexW4WintelDP3*>(const_cast<uint32_t*>(GetVertices()));
        for (uint32_t i = 0; i < lNumVertices; ++i)
        {
            ZVector2& vTexCoords = pVertices[lStartVertex + i].t;
            vTexCoords.x = pfTexCoords[0];
            vTexCoords.y = pfTexCoords[1];
            pfTexCoords += 2;
        }
    }

    void ZPrimAccessStandardWeightedMeshD3D::GetVerticesRaw(uint32_t lStartVertex, uint32_t lNumVertices, void* pVertices)
    {
        ZASSERT(lStartVertex + lNumVertices <= GetNumVertices());

        const auto* pSrc = reinterpret_cast<const SVertexW4WintelDP3*>(GetVerticesConst()) + lStartVertex;
        auto* pDst = reinterpret_cast<SVertexW4WintelDP3*>(pVertices);
        for (uint32_t i = 0; i < lNumVertices; ++i)
        {
            memcpy(&pDst[i], &pSrc[i], sizeof(SVertexW4WintelDP3));
        }
    }

    void ZPrimAccessStandardWeightedMeshD3D::SetVerticesRaw(uint32_t lStartVertex, uint32_t lNumVertices, const void* pVertices)
    {
        ZASSERT(lStartVertex + lNumVertices <= GetNumVertices());

        const auto* pSrc = reinterpret_cast<const SVertexW4WintelDP3*>(pVertices) + lStartVertex;
        auto* pDst = reinterpret_cast<SVertexW4WintelDP3*>(const_cast<uint32_t*>(GetVertices()));
        for (uint32_t i = 0; i < lNumVertices; ++i)
        {
            memcpy(&pDst[i], &pSrc[i], sizeof(SVertexW4WintelDP3));
        }
    }

    void ZPrimAccessStandardWeightedMeshD3D::GetTriangles(uint32_t lStartTriangle, uint32_t lNumTriangles, float* pfVertices)
    {
        ZASSERT(lStartTriangle + lNumTriangles <= GetNumTriangles());

        const auto* pVertices = reinterpret_cast<const SVertexW4WintelDP3*>(GetVerticesConst());
        const uint16_t* pTriangles = &GetIndicesConst()[3 * lStartTriangle + 2];
        const uint32_t lNumIndices = 3 * lNumTriangles;

        for (uint32_t i = 0; i < lNumIndices; ++i)
        {
            const ZVector3& vPosition = pVertices[pTriangles[i]].p;
            pfVertices[0] = vPosition.x;
            pfVertices[1] = vPosition.y;
            pfVertices[2] = vPosition.z;
            pfVertices += 3;
        }
    }

    uint32_t ZPrimAccessStandardWeightedMeshD3D::GetTrianglesInBox(uint32_t lStartTriangle, uint32_t iNumTestTriangles, float* pfVertices, uint32_t lMaxNumTriangles, const float* vMin, const float* vMax)
    {
        GetNumVertices();
        GetNumTriangles();

        const auto* pVertices = reinterpret_cast<const SVertexW4WintelDP3*>(GetVerticesConst());
        const uint16_t* pTriangles = &GetIndicesConst()[3 * lStartTriangle + 2];

        uint32_t lNumFound = 0;
        for (uint32_t i = 0; i < iNumTestTriangles; ++i)
        {
            const SVertexW4WintelDP3* pV0 = &pVertices[pTriangles[0]];
            const SVertexW4WintelDP3* pV1 = &pVertices[pTriangles[1]];
            const SVertexW4WintelDP3* pV2 = &pVertices[pTriangles[2]];

            const float* pCoord0 = &pV0->p.x;
            const float* pCoord1 = &pV1->p.x;
            const float* pCoord2 = &pV2->p.x;

            bool bInside = true;
            for (uint32_t axis = 0; axis < 3; ++axis)
            {
                const float fMin = vMin[axis];
                const float fMax = vMax[axis];

                if (!((pCoord0[axis] >= fMin || pCoord1[axis] >= fMin || pCoord2[axis] >= fMin) &&
                      (pCoord0[axis] <= fMax || pCoord1[axis] <= fMax || pCoord2[axis] <= fMax)))
                {
                    bInside = false;
                    break;
                }
            }

            if (bInside)
            {
                pfVertices[0] = pV0->p.x;
                pfVertices[1] = pV0->p.y;
                pfVertices[2] = pV0->p.z;
                pfVertices[3] = pV1->p.x;
                pfVertices[4] = pV1->p.y;
                pfVertices[5] = pV1->p.z;
                pfVertices[6] = pV2->p.x;
                pfVertices[7] = pV2->p.y;
                pfVertices[8] = pV2->p.z;
                pfVertices += 9;

                ++lNumFound;
                if (lNumFound == lMaxNumTriangles)
                {
                    return lNumFound;
                }
            }

            pTriangles += 3;
        }

        return lNumFound;
    }

    uint32_t ZPrimAccessStandardWeightedMeshD3D::GetTrianglesInBox(uint32_t* pTriangles, uint32_t lMaxNumTriangles, const float* vMin, const float* vMax) const
    {
        GetNumVertices();
        const uint32_t lNumTriangles = GetNumTriangles();

        const auto* pVertices = reinterpret_cast<const SVertexW4WintelDP3*>(GetVerticesConst());
        const uint16_t* pIndices = &GetIndicesConst()[2];

        uint32_t lNumFound = 0;
        for (uint32_t i = 0; i < lNumTriangles; ++i)
        {
            const SVertexW4WintelDP3* pV0 = &pVertices[pIndices[0]];
            const SVertexW4WintelDP3* pV1 = &pVertices[pIndices[1]];
            const SVertexW4WintelDP3* pV2 = &pVertices[pIndices[2]];

            const float* pCoord0 = &pV0->p.x;
            const float* pCoord1 = &pV1->p.x;
            const float* pCoord2 = &pV2->p.x;

            bool bInside = true;
            for (uint32_t axis = 0; axis < 3; ++axis)
            {
                const float fMin = vMin[axis];
                const float fMax = vMax[axis];

                if (!((pCoord0[axis] >= fMin || pCoord1[axis] >= fMin || pCoord2[axis] >= fMin) &&
                      (pCoord0[axis] <= fMax || pCoord1[axis] <= fMax || pCoord2[axis] <= fMax)))
                {
                    bInside = false;
                    break;
                }
            }

            if (bInside)
            {
                pTriangles[0] = pIndices[0];
                pTriangles[1] = pIndices[1];
                pTriangles[2] = pIndices[2];
                pTriangles += 3;

                ++lNumFound;
                if (lNumFound == lMaxNumTriangles)
                {
                    return lNumFound;
                }
            }

            pIndices += 3;
        }

        return lNumFound;
    }

    void ZPrimAccessStandardWeightedMeshD3D::CloneVertex(uint32_t lVertex, const ZPrimAccessMeshWeighted* pSource, uint32_t lSourceVertex)
    {
        ZASSERT((m_lStatusFlags & STATUS_FLAGS::SF_LOCKED) && IsEditable());
        ZASSERT(lVertex <= GetNumVertices());
        ZASSERT(pSource && lSourceVertex <= pSource->GetNumVertices());

        auto* pVertices = reinterpret_cast<SVertexW4WintelDP3*>(const_cast<uint32_t*>(GetVertices()));
        const auto* pSourceVertices = reinterpret_cast<const SVertexW4WintelDP3*>(pSource->GetVerticesConst());
        memcpy(&pVertices[lVertex], &pSourceVertices[lSourceVertex], sizeof(SVertexW4WintelDP3));
    }

    void ZPrimAccessStandardWeightedMeshD3D::Allocate(uint32_t lNumTriangles, uint32_t lNumVertices)
    {
        const SPrimMeshWeighted* pSrcMesh = m_hPrim;

        m_pAllocatedVertices = ZUniMemory::NewArray<SVertexW4WintelDP3>(lNumVertices);
        memset(m_pAllocatedVertices, 0, sizeof(SVertexW4WintelDP3));

        m_pAllocatedIndices = ZUniMemory::NewArray<uint16_t>(3 * lNumTriangles + 2);
        memset(m_pAllocatedIndices, 0, sizeof(uint16_t) * (3 * lNumTriangles + 2));
        m_pAllocatedIndices[0] = 1;
        m_pAllocatedIndices[1] = static_cast<uint16_t>(3 * lNumTriangles);

        m_SubMeshes[0].lNumVertices = lNumVertices;
        m_SubMeshes[0].lVertices = reinterpret_cast<uint32_t>(m_pAllocatedVertices) | 0x80000000u;
        m_SubMeshes[0].lNumIndices = 0;
        m_SubMeshes[0].lIndices = reinterpret_cast<uint32_t>(m_pAllocatedIndices) | 0x80000000u;
        m_alSubMeshTable[0] = reinterpret_cast<uint32_t>(m_SubMeshes) | 0x80000000u;

        memcpy(&m_Mesh, pSrcMesh, sizeof(m_Mesh));
        m_Mesh.lProperties |= SPrimObject::PROPERTY_FLAGS::PROPERTY_ISEDITABLE;
        m_Mesh.lSubMeshTable = reinterpret_cast<uint32_t>(m_alSubMeshTable) | 0x80000000u;

        m_hPrim.m_lHandleValue = reinterpret_cast<uint32_t>(&m_Mesh) | 0x80000000u;
    }
}
