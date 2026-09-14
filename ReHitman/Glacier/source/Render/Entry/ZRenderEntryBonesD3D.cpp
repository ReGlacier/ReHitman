#include <Glacier/Render/Entry/ZRenderEntryBonesD3D.h>
#include <Glacier/Render/Prim/SPrimObjectHeader.h>
#include <Glacier/Render/Prim/ZPrimHandle.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZRenderEntryBonesD3D::~ZRenderEntryBonesD3D() = default;

    ZRenderEntryBonesD3D::ZRenderEntryBonesD3D(const ZRenderEntryGeomCreateInfo& sInfo)
        : ZRenderEntryBones(sInfo)
    {
        if (sInfo.m_pBaseGeom)
        {
            m_lPrimId = sInfo.m_pBaseGeom->Prim();
        }
        else
        {
            m_lPrimId = sInfo.m_lPrim;
        }

        const SPrimObjectHeader* pHeader = ZPrimHandle { m_lPrimId };
        const auto lMask = (SPrimObjectHeader::PROPERTY_FLAGS::HAS_REFLECTION2D | SPrimObjectHeader::PROPERTY_FLAGS::HAS_REFRACTION2D);
        static_assert(lMask == 0x30, "Bad value of lMask (expected 0x30)");

        if (pHeader && (pHeader->lPropertyFlags & lMask) != 0)
        {
            m_lGeomListsControl |= 0x8u;
        }
    }

    ZRenderEntryBonesD3D* ZRenderEntryBonesD3D::Create(const ZRenderEntryGeomCreateInfo& sInfo)
    {
        return ZUniMemory::New<ZRenderEntryBonesD3D>(sInfo);
    }
}
