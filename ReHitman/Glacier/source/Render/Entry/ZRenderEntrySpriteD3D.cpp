#include <Glacier/Render/Entry/ZRenderEntrySpriteD3D.h>
#include <Glacier/Render/Prim/SPrimHeader.h>
#include <Glacier/Render/Prim/EPrimType.h>
#include <Glacier/Render/Prim/ZPrimHandle.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZRenderEntrySpriteD3D::~ZRenderEntrySpriteD3D()
    {
        // TODO: Finish me
    }

    ZRenderEntrySpriteD3D::ZRenderEntrySpriteD3D(ZRenderEntryGeomCreateInfo* pInfo)
        : ZRenderEntrySprite(pInfo)
    {
    }

    ZRenderEntrySpriteD3D* ZRenderEntrySpriteD3D::Create(ZRenderEntryGeomCreateInfo* pInfo)
    {
        const ZPrimHandle hPrim{pInfo->m_lPrim};
        if (!hPrim)
        {
            return nullptr;
        }

        const SPrimHeader* pHeader = ZPrimHandle{pInfo->m_lPrim};
        if (pHeader->lType != PTSPRITES)
        {
            return nullptr;
        }

        return ZUniMemory::New<ZRenderEntrySpriteD3D>(pInfo);
    }
}