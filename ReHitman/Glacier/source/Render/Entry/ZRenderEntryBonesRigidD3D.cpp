#include <Glacier/Render/Entry/ZRenderEntryBonesRigidD3D.h>
#include <Glacier/Render/Prim/SPrimHeader.h>
#include <Glacier/Render/Prim/ZPrimHandle.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/ZUniMemory.h>

namespace Glacier
{
    ZRenderEntryBonesRigidD3D::ZRenderEntryBonesRigidD3D(const ZRenderEntryGeomCreateInfo& sInfo)
        : ZRenderEntryBonesRigid(sInfo)
    {
        const uint32_t lPrim = sInfo.m_pBaseGeom ? sInfo.m_pBaseGeom->Prim() : sInfo.m_lPrim;
        const auto* pHeader = ZPrimHandle{ lPrim }.Get<SPrimHeader>();
        if (pHeader && (pHeader->lDrawDestination & 0x30u) != 0)
            m_lGeomListsControl |= 0x8u;
    }

    ZRenderEntryBonesRigidD3D::~ZRenderEntryBonesRigidD3D() = default;

    ZRenderEntryBonesRigidD3D* ZRenderEntryBonesRigidD3D::Create(const ZRenderEntryGeomCreateInfo& sInfo)
    {
        if (!sInfo.m_pBaseGeom)
            return nullptr;

        const auto* pGeom = sInfo.m_pBaseGeom->GetGeom();
        const bool bLinkObject = pGeom
            ? pGeom->IsDerivedFrom<ZLNKOBJ>()
            : sInfo.m_pBaseGeom->IsDerivedFromStdObj(ZLNKOBJ::m_Id);
        if (!bLinkObject)
            return nullptr;

        const auto* pHeader = ZPrimHandle{ sInfo.m_pBaseGeom->Prim() }.Get<SPrimHeader>();
        if (!pHeader || (pHeader->lDrawDestination & 0x4u) == 0)
            return nullptr;

        return ZUniMemory::New<ZRenderEntryBonesRigidD3D>(sInfo);
    }
}
