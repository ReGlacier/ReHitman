#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Prim/ZPrimAccess.h>


namespace Glacier
{
    class ZPrimAccessMesh : public ZPrimAccess
    {
    public:
        // vtbl
        void ReleasePrim() override;
        ~ZPrimAccessMesh() override;
        
        virtual ZPrimAccessMesh* CreateEditable(uint32_t lNumTriangles, uint32_t lNumVertices) = 0;
        virtual ZPrimAccessMesh* Clone();
        virtual bool IsEditable() const;
        virtual void GetPositions(uint32_t lStartVertex, uint32_t lNumVertices, float* pfPosition) = 0;
        virtual void SetPositions(uint32_t lStartVertex, uint32_t lNumVertices, const float* pfPosition) = 0;
        virtual void GetNormals(uint32_t lStartVertex, uint32_t lNumVertices, float* pfNormal) = 0;
        virtual void SetNormals(uint32_t lStartVertex, uint32_t lNumVertices, const float* pfNormal) = 0;
        virtual void GetColors(uint32_t lStartVertex, uint32_t lNumVertices, uint32_t* plColors) = 0;
        virtual void SetColors(uint32_t lStartVertex, uint32_t lNumVertices, const uint32_t* plColors) = 0;
        virtual void GetTexCoords(uint32_t lStartVertex, uint32_t lNumVertices, float* pfTexCoords) = 0;
        virtual void SetTexCoords(uint32_t lStartVertex, uint32_t lNumVertices, const float* pfTexCoords) = 0;
        virtual void GetVerticesRaw(uint32_t lStartVertex, uint32_t lNumVertices, void* pVertices) = 0;
        virtual void SetVerticesRaw(uint32_t lStartVertex, uint32_t lNumVertices, const void* pVertices) = 0;
        virtual void GetTriangles(uint32_t lStartTriangle, uint32_t lNumTriangles, uint16_t* plVertices) = 0;
        virtual void SetTriangles(uint32_t lStartTriangle, uint32_t lNumTriangles,const uint16_t* plVertices);
        virtual uint32_t GetTrianglesInBox(uint32_t* pTriangles, uint32_t lMaxNumTriangles, const float* vMin, const float* vMax);
        virtual void GetTriangles(uint32_t lStartTriangle, uint32_t lNumTriangles, float* pfVertices) = 0;
        virtual uint32_t GetTrianglesInBox(uint32_t lStartTriangle, uint32_t iNumTestTriangles, float* pfVertices, uint32_t lMaxNumTriangles, const float* vMin, const float* vMax) = 0;

        // methods
        uint32_t GetNumTriangles() const;
        uint32_t GetNumVertices() const;
        uint16_t* GetIndices() const;
        uint16_t* GetIndicesReadWrite();
        const uint32_t* GetVertices() const;
    };
}