#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Prim/ZPrimAccessMesh.h>
#include <Glacier/Render/Prim/SPrimMesh.h>
#include <Glacier/Render/Prim/SPrimSubMesh.h>
#include <Glacier/Render/Prim/SVertexWintel.h>
#include <cstdint>


namespace Glacier
{
    class ZPrimAccessOldMeshD3D : public ZPrimAccessMesh
    {
    public:
        // vtbl
        ~ZPrimAccessOldMeshD3D() override;
        ZPrimAccessMesh* CreateEditable(uint32_t lNumTriangles, uint32_t lNumVertices) override;
        void GetPositions(uint32_t lStartVertex, uint32_t lNumVertices, float* pfPosition) override;
        void SetPositions(uint32_t lStartVertex, uint32_t lNumVertices, const float* pfPosition) override;
        void GetNormals(uint32_t lStartVertex, uint32_t lNumVertices, float* pfNormal) override;
        void SetNormals(uint32_t lStartVertex, uint32_t lNumVertices, const float* pfNormal) override;
        void GetColors(uint32_t lStartVertex, uint32_t lNumVertices, uint32_t* plColors) override;
        void SetColors(uint32_t lStartVertex, uint32_t lNumVertices, const uint32_t* plColors) override;
        void GetTexCoords(uint32_t lStartVertex, uint32_t lNumVertices, float* pfTexCoords) override;
        void SetTexCoords(uint32_t lStartVertex, uint32_t lNumVertices, const float* pfTexCoords) override;
        void GetVerticesRaw(uint32_t lStartVertex, uint32_t lNumVertices, void* pVertices) override;
        void SetVerticesRaw(uint32_t lStartVertex, uint32_t lNumVertices, const void* pVertices) override;
        void GetTriangles(uint32_t lStartTriangle, uint32_t lNumTriangles, float* pfVertices) override; // PC 0x004A4710
        uint32_t GetTrianglesInBox(uint32_t lStartTriangle, uint32_t iNumTestTriangles, float* pfVertices, uint32_t lMaxNumTriangles, const float* vMin, const float* vMax) override; // PC 0x004A4820
        uint32_t GetTrianglesInBox(uint32_t* pTriangles, uint32_t lMaxNumTriangles, const float* vMin, const float* vMax) const override; // PC 0x004A40D0

        // methods
        ZPrimAccessOldMeshD3D(const ZPrimHandle& hPrim);
        void Allocate(uint32_t lNumTriangles, uint32_t lNumVertices);

        // members
        SPrimMesh m_Mesh;
        uint32_t m_alSubMeshTable[1];
        SPrimSubMesh m_SubMeshes[1];
        SVertexWintel* m_pAllocatedVertices;
        uint16_t* m_pAllocatedIndices;
    };
    RE_VERIFY_SIZE(ZPrimAccessOldMeshD3D, 0x60); // PC alloc verified
}
