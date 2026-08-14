#include <Glacier/Render/Object/ZRenderObjectOldMeshD3D.h>
#include <Glacier/Render/Prim/SPrimSubMesh.h>
#include <Glacier/Render/Prim/SPrimMesh.h>
#include <Glacier/Render/Prim/EPrimType.h>
#include <Glacier/Render/Globals.h>


namespace Glacier
{
    ZRenderObjectOldMeshD3D::ZRenderObjectOldMeshD3D(const ZPrimHandle& hPrim, ZRenderMaterialInstance* pMaterialInstance)
        : ZRenderObjectD3D(hPrim, pMaterialInstance)
    {
        const SPrimMesh* pMesh = hPrim;
        ZASSERT(pMesh->lType == EPrimType::PTMESH);
        ZASSERT(pMesh->lSubMeshTable);

        // TODO: Finish me
    }

    ZRenderObjectOldMeshD3D::~ZRenderObjectOldMeshD3D()
    {
        m_VertexContainer.Release();
        m_IndexContainer.Release();
    }

    ZRenderObjectInstance* ZRenderObjectOldMeshD3D::CreateInstance(ZBaseGeom* pBaseGeom)
    {
        // TODO: Finish me
        return nullptr;
    }
}