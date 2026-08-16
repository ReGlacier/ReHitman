#include <Glacier/Render/Prim/ZPrimAccessMesh.h>
#include <Glacier/Render/Prim/SPrimSubMesh.h>
#include <Glacier/Render/Prim/SPrimMesh.h>


namespace Glacier
{
    // ZPrimAccessMesh
    void ZPrimAccessMesh::ReleasePrim()
    {}

    ZPrimAccessMesh* ZPrimAccessMesh::Clone()
    {
        // TODO: Finish me
        return nullptr;
    }

    bool ZPrimAccessMesh::IsEditable()
    {
        const SPrimMesh* pMesh = m_hPrim;
        return (pMesh->lProperties & SPrimMesh::PROPERTY_FLAGS::PROPERTY_ISEDITABLE) != 0;
    }

    void ZPrimAccessMesh::SetTriangles(uint32_t lStartTriangle, uint32_t lNumTriangles,const uint16_t* plVertices)
    {
        // TODO: Finish me
    }

    uint32_t ZPrimAccessMesh::GetTrianglesInBox(uint32_t* pTriangles, uint32_t lMaxNumTriangles, const float* vMin, const float* vMax)
    {
        // TODO: Finish me
        return 0;
    }

    uint32_t ZPrimAccessMesh::GetNumTriangles() const
    {
        const SPrimMesh* pMesh = m_hPrim;
        const uint32_t lSubMeshTable = pMesh->lSubMeshTable;
        const uint32_t* pSubMeshTable = ZPrimHandle{lSubMeshTable};
        const uint32_t lFirstSubMesh = pSubMeshTable[0];
        const SPrimSubMesh* pFirstSubMesh = ZPrimHandle{lFirstSubMesh};
        const uint32_t lIndices = pFirstSubMesh->lIndices;
        const uint16_t* pIndices = ZPrimHandle{lIndices};
        ZASSERT(pIndices[0] != 1);
        return pIndices[0] / 3;
    }
}