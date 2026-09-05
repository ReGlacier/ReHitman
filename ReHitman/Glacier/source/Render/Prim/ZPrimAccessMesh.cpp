#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/Prim/ZPrimAccessMesh.h>
#include <Glacier/Render/Prim/SPrimSubMesh.h>
#include <Glacier/Render/Prim/SPrimMesh.h>


namespace Glacier
{
    // ZPrimAccessMesh
    void ZPrimAccessMesh::ReleasePrim()
    {
        const SPrimMesh* pMesh = m_hPrim;
        const uint32_t lPrimId = m_hPrim;
        const uint32_t lSubMeshTable = pMesh->lSubMeshTable;
        const uint32_t* pSubMeshTable = ZPrimHandle{lSubMeshTable};
        const uint32_t lFirstSubMesh = pSubMeshTable[0];
        const SPrimSubMesh* pFirstSubMesh = ZPrimHandle{lFirstSubMesh};

        ZPrimControlBase::Instance()->FreePrimData(pFirstSubMesh->lVertices);
        ZPrimControlBase::Instance()->FreePrimData(pFirstSubMesh->lIndices);
        ZPrimControlBase::Instance()->FreePrimData(lFirstSubMesh);
        ZPrimControlBase::Instance()->FreePrimData(lSubMeshTable);
        ZPrimControlBase::Instance()->FreePrimData(lPrimId);
    }

    ZPrimAccessMesh* ZPrimAccessMesh::Clone()
    {
        const SPrimMesh* pMesh = m_hPrim;
        const uint32_t lPrimId = m_hPrim;
        const uint32_t lSubMeshTable = pMesh->lSubMeshTable;
        const uint32_t* pSubMeshTable = ZPrimHandle{lSubMeshTable};
        const uint32_t lFirstSubMesh = pSubMeshTable[0];
        const SPrimSubMesh* pFirstSubMesh = ZPrimHandle{lFirstSubMesh};
        const uint32_t lNumTriangles = GetNumTriangles();

        auto* pEditable = CreateEditable(lNumTriangles, pFirstSubMesh->lNumVertices);
        pEditable->Lock(2u);
        
        auto* pVertices = (void*)GetVertices(); // Weird, I know
        pEditable->GetVerticesRaw(0, pFirstSubMesh->lNumVertices, pVertices);

        auto* pRWIndices = GetIndicesReadWrite();
        memcpy(GetIndices(), pRWIndices, 6 * lNumTriangles + 4);

        pEditable->Unlock();
        
        return pEditable;
    }

    bool ZPrimAccessMesh::IsEditable() const
    {
        const SPrimMesh* pMesh = m_hPrim;
        return (pMesh->lProperties & SPrimMesh::PROPERTY_FLAGS::PROPERTY_ISEDITABLE) != 0;
    }

    void ZPrimAccessMesh::SetTriangles(uint32_t lStartTriangle, uint32_t lNumTriangles,const uint16_t* plVertices)
    {
        ZASSERT(lStartTriangle + lNumTriangles <= GetNumTriangles());
        if (lNumTriangles == 0 || !plVertices)
        {
            return;
        }

        uint16_t* pIndices = GetIndicesReadWrite();
        const uint32_t lStartIndex = 3 * lStartTriangle;
        const uint32_t lIndexCount = 3 * lNumTriangles;
        memcpy(&pIndices[lStartIndex], plVertices, lIndexCount * sizeof(uint16_t));
    }

    uint32_t ZPrimAccessMesh::GetTrianglesInBox(uint32_t* pTriangles, uint32_t lMaxNumTriangles, const float* vMin, const float* vMax)
    {
        const uint32_t lStartTriangle = reinterpret_cast<uint32_t>(pTriangles);
        ZASSERT(lStartTriangle + lMaxNumTriangles <= GetNumTriangles());

        uint16_t* pDstIndices = &GetIndices()[3 * lStartTriangle];
        const uint16_t* pSrcIndices = reinterpret_cast<const uint16_t*>(vMin);

        const uint32_t lIndexCount = 3 * lMaxNumTriangles;
        for (uint32_t i = 0; i < lIndexCount; ++i)
        {
            pDstIndices[i] = pSrcIndices[i];
        }

        return reinterpret_cast<uint32_t>(&pDstIndices[lIndexCount]);
    }

    uint32_t ZPrimAccessMesh::GetNumTriangles() const
    {
        const uint16_t* pIndices = GetIndices();
        ZASSERT(pIndices);
        ZASSERT(pIndices[0] != 1);
        return pIndices[0] / 3;
    }

    uint32_t ZPrimAccessMesh::GetNumVertices() const
    {
        const SPrimMesh* pMesh = m_hPrim;
        const uint32_t lSubMeshTable = pMesh->lSubMeshTable;
        const uint32_t* pSubMeshTable = ZPrimHandle{lSubMeshTable};
        const uint32_t lFirstSubMesh = pSubMeshTable[0];
        const SPrimSubMesh* pFirstSubMesh = ZPrimHandle{lFirstSubMesh};
        return pFirstSubMesh->lNumVertices;
    }

    uint16_t* ZPrimAccessMesh::GetIndices() const
    {
        ZASSERT((m_lStatusFlags & STATUS_FLAGS::SF_LOCKED) && IsEditable());

        const SPrimMesh* pMesh = m_hPrim;
        const uint32_t lSubMeshTable = pMesh->lSubMeshTable;
        const uint32_t* pSubMeshTable = ZPrimHandle{lSubMeshTable};
        const uint32_t lFirstSubMesh = pSubMeshTable[0];
        const SPrimSubMesh* pFirstSubMesh = ZPrimHandle{lFirstSubMesh};
        const uint32_t lIndices = pFirstSubMesh->lIndices;
        const uint16_t* pIndices = ZPrimHandle{lIndices};
        return const_cast<uint16_t*>(pIndices);
    }

    uint16_t* ZPrimAccessMesh::GetIndicesReadWrite()
    {
        ZASSERT((m_lStatusFlags & STATUS_FLAGS::SF_LOCKED));

        const SPrimMesh* pMesh = m_hPrim;
        const uint32_t lSubMeshTable = pMesh->lSubMeshTable;
        const uint32_t* pSubMeshTable = ZPrimHandle{lSubMeshTable};
        const uint32_t lFirstSubMesh = pSubMeshTable[0];
        const SPrimSubMesh* pFirstSubMesh = ZPrimHandle{lFirstSubMesh};
        const uint32_t lIndices = pFirstSubMesh->lIndices;
        const uint16_t* pIndices = ZPrimHandle{lIndices};
        return const_cast<uint16_t*>(pIndices);
    }

    const uint32_t* ZPrimAccessMesh::GetVertices() const
    {
        ZASSERT((m_lStatusFlags & STATUS_FLAGS::SF_LOCKED) && IsEditable());

        const SPrimMesh* pMesh = m_hPrim;
        const uint32_t lSubMeshTable = pMesh->lSubMeshTable;
        const uint32_t* pSubMeshTable = ZPrimHandle{lSubMeshTable};
        const uint32_t lFirstSubMesh = pSubMeshTable[0];
        const SPrimSubMesh* pFirstSubMesh = ZPrimHandle{lFirstSubMesh};
        const uint32_t lVertices = pFirstSubMesh->lVertices;
        const uint32_t* pVertices = ZPrimHandle{lVertices};
        return pVertices;
    }
}